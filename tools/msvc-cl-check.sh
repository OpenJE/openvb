#!/usr/bin/env bash
# tools/msvc-cl-check.sh - syntax-check sources with the product's compiler, no code generated.
#
#   usage: tools/msvc-cl-check.sh <file> [<file> ...]
#
# WHY. tools/verify-symbol.sh used to pick a compiler from a fallback chain ending in host g++, because
# mingw was the only Windows-targeting front end in the shell. The reimplementation is built by MSVC 2002 cl
# inside the docker image, so a symbol check performed by any other compiler is evidence about a toolchain
# the shipped binary does not use - the same proxy the suite harness just shed. cl is a Windows compiler, so
# there was nothing to work around: this replaces the cross compiler rather than wrapping it.
#
# /Zs is cl's syntax-check-only mode, the equivalent of gcc's -fsyntax-only: the TU is parsed and resolved,
# no object is emitted. That matters for more than speed - the check must not litter the tree with .obj files
# the way a /c invocation would, and it must not depend on link libraries being right.
#
# Exit status is cl's. Diagnostics go to stdout so the caller can grep them the way it grepped gcc's, and the
# phrasing differs (see the pattern table in verify-symbol.sh): cl says "error C2065: 'x' : undeclared
# identifier", gcc says "'x' was not declared in this scope".

set -uo pipefail

IMAGE="${MSVC_IMAGE:?MSVC_IMAGE not set - run inside the devenv shell}"
DOCKER="${OPENVB_DOCKER:-docker}"

[ $# -ge 1 ] || { echo "usage: $0 <file> [...]" >&2; exit 2; }

files=()
for f in "$@"; do
  # Path normalisation is a correctness issue, not tidiness. An ABSOLUTE path was previously turned into
  # Z:\work\/mnt/shanes-.../probe.cpp by prefixing blindly, and a command line that confused produced no
  # error at all - cmd fell back to its INTERACTIVE PROMPT and blocked on stdin forever. Observed as a
  # 90s timeout with no diagnostic, and `timeout` could not even interrupt the docker client cleanly.
  # So: make every source relative to the repo root first, refuse anything outside it, and never let a
  # container inherit stdin (see the </dev/null below).
  case "$f" in
    "$PWD"/*) f="${f#"$PWD"/}" ;;
    ./*)      f="${f#./}" ;;
    /*)       echo "REFUSE: '$f' is outside the repo root ($PWD) - the container mounts only the tree" >&2
              exit 3 ;;
  esac
  [ -f "$f" ] || { echo "REFUSE: '$f' is not in the tree (checking nothing can only report clean)" >&2; exit 3; }
  files+=("$f")
done

WIN="$(printf '%s\n' "${files[@]}" | sed 's#^#Z:\\work\\#; s#/#\\#g' | tr '\n' ' ')"

# /Zs means no /Fo is needed, so there is no scratch directory to win or clean up, and the same-basename
# object-collapse hazard that forces one cl call per TU in the build path does not apply here.
# No -i and no -t: attaching stdin is what turned a malformed path into an endless hang, because
# cmd drops to an interactive prompt and blocks reading input that never arrives.
"$DOCKER" run --rm --cap-add=NET_RAW --security-opt seccomp=unconfined \
    -v "$PWD:/work" -w /work \
    -e SRCWIN="$WIN" \
    "$IMAGE" -lc '
      VC="call Z:\\home\\msvc2002\\bin\\vcvars32.bat >NUL"
      # /I Z:\work is not decorative. The probes in verify-symbol include headers by REPO-RELATIVE path
      # (#include "modules/openje/src/JE/Foo.hpp"), so without the tree root on the include path every probe
      # fails C1083 and the tool reports a stale symbol when what is actually missing is its own -I.
      # (An earlier draft of this very comment broke the script: it contained an apostrophe, and this block
      # lives inside a single-quoted host string, so the quote ended early and bash reported "unexpected EOF
      # while looking for matching quote". Comments inside quoted heredoc-style strings must not use
      # apostrophes - or, better, the logic belongs in its own file, which is the pattern this repo keeps
      # converging on.)
      INC="/I Z:\\work /I Z:\\work\\src /I Z:\\work\\modules\\openje\\src /I Z:\\work\\modules\\tracing\\src"
      rc=0
      for s in $SRCWIN; do
        wine cmd /c "$VC && cl /nologo /Zs /EHsc /W3 /WX- $INC $s" 2>&1 | sed "s/^/  /"
        p=${PIPESTATUS[0]}
        [ "$p" -eq 0 ] || rc=1
      done
      exit $rc
    ' < /dev/null
exit $?
