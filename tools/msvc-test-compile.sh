#!/usr/bin/env bash
# tools/msvc-test-compile.sh - build a host test suite with the SAME compiler the product is shipped as.
#
#   usage: tools/msvc-test-compile.sh <out.exe> <source.cpp> [<source.cpp> ...]
#
# WHY
# Suites were compiled by i686-w64-mingw32-g++ while the product was built by MSVC 2002 cl in docker, so
# every green suite was evidence about a compiler the shipped binary does not use. This is that other
# compiler: same image, same -v $PWD:/work, same Z:\home\msvc2002\bin\vcvars32.bat bootstrap as
# devenv/scripts/msvc-build.nix, which is what makes "it compiles here" mean "it compiles in the product".
#
# QUOTING DISCIPLINE
# The build logic lives in tools/msvc-test-compile-inner.sh and is reached through exported variables.
# Embedding it in `docker run -lc "..."` with inner `wine cmd /c \"...\"` is what hung this script on an
# unterminated quote until bash waited for input that never came - the same nested-escaping trap that
# defeated two consecutive edits in tools/msvc-suite-probe.sh. One quoting level per file, on purpose.
# The image's ENTRYPOINT is also `bash -c`, so -e VAR= flags are parsed as the COMMAND; the environment is
# exported inside the -lc script instead.
#
# Exit status is the container's, and success additionally requires the output file to exist: a build that
# produced nothing must never be reported as green.

set -uo pipefail

IMAGE="${MSVC_IMAGE:?MSVC_IMAGE not set - run inside the devenv shell}"
DOCKER="${OPENVB_DOCKER:-docker}"

[ $# -ge 2 ] || { echo "usage: $0 <out.exe> <source.cpp> [...]" >&2; exit 2; }
out="${1#./}"; shift

srcs=()
for s in "$@"; do
  f="$s"
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
  s="$f"
  if [ -f "$s" ]; then srcs+=("$s"); else echo "REFUSE: '$s' is not in the tree (compiling nothing can only report green)" >&2; exit 3; fi
done

# Scratch goes under build/, and the exe is copied to the caller's path afterwards - not a stylistic
# choice. The container's wine user cannot write into .devenv/state/tests/: the first attempt failed with
#   fatal error C1083: Cannot open compiler generated file: ...\.objs\o1.obj: Permission denied
# because host-side state dirs are owned by a different uid than the container's build user. msvc-build.nix
# already works around exactly this by creating /work/build and /work/obj INSIDE the container; build/ is
# the writable-by-convention tree, so intermediate output stays there and only the finished exe is moved.
STAGE="build/msvc-tests"
STAGE_OBJ="Z:\\work\\build\\msvc-tests\\objs"
SRCWIN="$(printf '%s\n' "${srcs[@]}" | sed 's#^#Z:\\work\\#; s#/#\\#g' | tr '\n' ' ')"
OUTWIN="Z:\\work\\build\\msvc-tests\\$(basename "$out")"

# Env goes through `-e` BEFORE the image name. Two failed attempts taught this: `-e VAR=` after the image is
# parsed as the COMMAND (observed: "/bin/bash: SUITE=cls_0x4d8d70: No such file or directory"), and embedding
# the whole build in `-lc "...wine cmd /c \"...\"..."` produced a command string bash tried to execute as a
# single filename. Values with backslashes and spaces have no business being quoted twice, so the container
# gets a variable-free one-word command and every string it needs arrives out of band.
# No -i and no -t: attaching stdin is what turned a malformed path into an endless hang, because
# cmd drops to an interactive prompt and blocks reading input that never arrives.
"$DOCKER" run --rm --cap-add=NET_RAW --security-opt seccomp=unconfined \
    -v "$PWD:/work" -w /work \
    -e SRCWIN="$SRCWIN" -e OUTWIN="$OUTWIN" -e OBJWIN="$STAGE_OBJ" -e STAGE="$STAGE" \
    "$IMAGE" -lc 'bash /work/tools/msvc-test-compile-inner.sh'
st=$?

if [ "$st" -eq 0 ]; then
  mkdir -p "$(dirname "$out")" && cp -f "$STAGE/$(basename "$out")" "$out" 2>/dev/null || st=$?
fi

# Scratch is removed by the container, not the host: the staged exe is owned by uid 100999 and a host
# `rm -rf` fails with Permission denied while still leaving exit status intact. Best-effort either way -
# cleanup has no business changing a build verdict.
"$DOCKER" run --rm -v "$PWD:/work" -w /work "$IMAGE" -lc "rm -rf /work/$STAGE" >/dev/null 2>&1 || true
rm -rf "$STAGE" 2>/dev/null || true

if [ "$st" -eq 0 ] && [ ! -f "$out" ]; then
  echo "msvc-test-compile: exit 0 but '$out' does not exist - reporting failure, not green" >&2
  exit 1
fi
exit "$st"
