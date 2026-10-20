#!/usr/bin/env bash
#
# verify-symbol.sh - cross-TU symbol verification WITHOUT a link step (T-023).
#
# Born while the shared tree could not link: `d3d8.lib` was missing, so the honest answer to
# "did your rename break a caller?" would have been "cannot test today". It can be tested: a
# generated probe TU that takes the member's address through its real header, plus a compile of
# every site that references the name, catches the class of bug LNK2019 would have caught.
#
# WHAT THIS PROVES                        WHAT IT DOES NOT PROVE
# - the name resolves through the real    - runtime behaviour, in any form (there is no exe here)
#   header, from another TU               - that the link would succeed (symbols, not code)
# - every referencing site still          - that the program does the right thing when it runs
#   compiles under the host compiler      - anything about the shipped binary's agreement
# - struct/class tag mismatches IF the
#   chosen compiler reports them
#
# A green run here is NEVER an acceptance result. Say so out loud; do not read it as one.

set -u

TMP="$(mktemp -d 2>/dev/null || echo /tmp/verify-symbol.$$)"
mkdir -p "$TMP" 2>/dev/null
trap 'rm -rf "$TMP"' EXIT
# (TMP is created here, BEFORE pick_compiler: the Win32 capability probe writes into it.
#  The first draft built the probe file first and died on `set -u` with TMP unbound.)


ROOT_DEFAULT="$(cd "$(dirname "$0")/.." && pwd)"
HIST_VOCAB='formerly|disproved|renamed|old name|was called|previously|no longer|superseded|T-0'

usage() {
  cat <<'EOF'
usage: verify-symbol.sh [mode] [options]

modes (exactly one required)
  --member  Class::member   take the member's address from a separate TU (works for arrays)
  --fn      [Class::]name   take a function's address
  --type    Class           require a COMPLETE type (sizeof)
  --absent  name            the name must survive ONLY in explicitly historical comments

options
  --header  path      header the probe includes (default: auto-detect the declaring .hpp)
  --root    dir       repo root (default: this script's parent)
  --compiler c++      force the compiler; default prefers clang++ (it reports tag mismatches)
  --no-tag-check      skip the BUG-007 struct/class diagnostic scan
  --self-test         prove this checker can fail (scratch copy; repo untouched)
  -h, --help          this text

exit 0 verified | 1 FAILED, with the site list | 2 setup error (no compiler, no header)
EOF
}

MODE=""; SYMBOL=""; HEADER=""; ROOT="$ROOT_DEFAULT"; CC_FORCED=""; TAGCHECK=1; SELFTEST=0
while [ $# -gt 0 ]; do
  case "$1" in
    --member|--fn|--type|--absent) MODE="${1#--}"; SYMBOL="${2:-}"; shift 2 ;;
    --header)   HEADER="${2:-}"; shift 2 ;;
    --root)     ROOT="${2:-}"; shift 2 ;;
    --compiler) CC_FORCED="${2:-}"; shift 2 ;;
    --no-tag-check) TAGCHECK=0; shift ;;
    --self-test) SELFTEST=1; shift ;;
    -h|--help)  usage; exit 0 ;;
    *) echo "verify-symbol: unknown argument: $1" >&2; usage >&2; exit 2 ;;
  esac
done

# ---------------------------------------------------------------- compiler
# Our sources are Win32: almost every site includes windows.h through JE.hpp. A compiler that
# cannot resolve windows.h does not report a STALE SYMBOL, it reports itself - so candidates are
# tried against a real Win32 TU, not merely `command -v`. Note ${CXX} here is `g++`, which cannot:
# the same trap run-all-tests.nix:129-131 documents. Preference is a capability test, not a guess.
win32_ok() {
  printf '#include <windows.h>\nint main(void){ return 0; }\n' > "$TMP/cap.c"
  "$1" -std=c++98 -fsyntax-only "$TMP/cap.c" -o /dev/null >/dev/null 2>&1
}

pick_compiler() {
  if [ -n "$CC_FORCED" ]; then
    # A forced compiler is a promise about WHICH toolchain produced the verdict. Silently
    # substituting a working one when the requested one is broken prints a green that the user
    # did not ask for - and hides that their toolchain cannot do the job at all.
    command -v "$CC_FORCED" >/dev/null 2>&1 || { echo "SETUP ERROR: --compiler $CC_FORCED not found." >&2; exit 2; }
    win32_ok "$CC_FORCED" || { echo "SETUP ERROR: --compiler $CC_FORCED cannot resolve the Win32 headers." >&2
      echo "  Our sources include windows.h via JE.hpp, so this compiler cannot verify them." >&2
      echo "  Not substituting another: the verdict must name the toolchain that ran." >&2
      exit 2; }
    printf '%s' "$CC_FORCED"; return 0
  fi
  # ${CXX} is FIRST only because the user set it; on this host it is g++, which cannot resolve
  # windows.h, and the capability test below rejects it (run-all-tests.nix:129-131 notes the same).
  for c in "${CXX:-}" i686-w64-mingw32-g++ clang++ g++ c++ cc; do
    [ -n "$c" ] || continue
    command -v "$c" >/dev/null 2>&1 || continue
    if win32_ok "$c"; then printf '%s' "$c"; return 0; fi
  done
  return 1
}

CC="$(pick_compiler)" || { echo "SETUP ERROR: no compiler here can resolve the Win32 headers." >&2
  echo "  That makes the check UNVERIFIABLE, not verified - refusing to report a pass." >&2; exit 2; }
CC_VER="$("$CC" --version 2>/dev/null | head -1)"

INC="-I$ROOT/src"
for m in "$ROOT"/modules/*/src; do [ -d "$m" ] && INC="$INC -I$m"; done
CFLAGS="-std=c++98 -Wall -Wextra -fsyntax-only"

SRC_GLOBS="--include=*.cpp --include=*.hpp --include=*.h"
search_roots() {
  [ -d "$ROOT/src" ] && printf '%s\n' "$ROOT/src"
  for m in "$ROOT"/modules/*/src "$ROOT"/modules/*/tests; do [ -d "$m" ] && printf '%s\n' "$m"; done
}

gather() {
  local name="$1" files
  files="$(search_roots | while read -r r; do grep -rn $SRC_GLOBS -F -w -- "$name" "$r" 2>/dev/null; done \
            | cut -d: -f1 | sort -u)"
  [ -n "$files" ] || return 0
  # shellcheck disable=SC2086
  echo "$files" | xargs -r awk -v sym="$name" '
    BEGIN { inblk = 0 }
    FNR==1 { inblk = 0 }
    {
      line = $0; out = ""; i = 1; n = length(line)
      while (i <= n) {
        two = substr(line, i, 2)
        if (inblk) { if (two == "*/") { inblk = 0; i += 2 } else { i += 1 }; continue }
        if (two == "//") { break }
        if (two == "/*") { inblk = 1; i += 2; continue }
        ch = substr(line, i, 1)
        if (ch == "\"" || ch == "'"'"'") {
          j = i + 1
          while (j <= n) { c = substr(line, j, 1)
            if (c == "\\") { j += 2; continue }
            if (c == "\"" || c == "'"'"'") { break }
            j += 1 }
          out = out substr(line, i, j - i + 1); i = j + 1; continue
        }
        out = out ch; i += 1
      }
      kind = ""
      if (index(out, sym) > 0) kind = "CODE"
      else if (index(line, sym) > 0) kind = "COMMENT"
      if (kind != "") print FILENAME ":" FNR ":" kind ":" line
    }'
}

# ---------------------------------------------------------------- probe
auto_header() {
  local name="$1" hit
  hit="$(gather "$name" | awk -F: '$3=="CODE" && $1 ~ /\.hpp$/ { print $1; exit }')"
  printf '%s' "$hit"
}

probe_compile() {
  local hdr_rel="$1" body="$2" log="$TMP/probe.log"
  { echo "// generated by tools/verify-symbol.sh - cross-TU probe, never compiled as part of a build"
    echo "#include \"$hdr_rel\""
    echo 'template <typename T> static void vs_take(T) {}'
    echo 'static void vs_probe()'
    echo '{'
    printf '%s\n' "$body"
    echo '}'
  } > "$TMP/probe.cpp"
  # NOT piped: the exit status of a pipe is the last command's, which is how a green is faked.
  "$CC" $CFLAGS $INC -I"$ROOT" "$TMP/probe.cpp" -o /dev/null > "$log" 2>&1
  printf '%s' "$?"
}

compile_site() {
  local f="$1" log="$2"
  case "$f" in
    *.hpp|*.h) printf '#include "%s"\n' "${f#$ROOT/}" > "$TMP/site.cpp"
               "$CC" $CFLAGS $INC -I"$ROOT" "$TMP/site.cpp" -o /dev/null > "$log" 2>&1 ;;
    *)         "$CC" $CFLAGS $INC -I"$ROOT" "$f" -o /dev/null > "$log" 2>&1 ;;
  esac
  if [ "$?" -eq 0 ]; then printf 'ok'; return 0; fi
  # A missing platform header is the TOOLCHAIN failing, not the CODE. Calling that a stale
  # symbol would blame the rename for $CC's missing windows.h - so classify it apart.
  if grep -qi "No such file or directory" "$log"; then printf 'unverifiable'; else printf 'failed'; fi
}

boundary() {
  echo "  boundary: compiler=$CC_VER"
  echo "            this proves DECLARATIONS and cross-TU name resolution at COMPILE time."
  echo "            It does NOT prove runtime behaviour, that the link would succeed, or that the"
  echo "            exe works. A green here is not an acceptance result - if the task needs the"
  echo "            binary, this run has not provided it."
}

# --------------------------------------------------------- BUG-007 tag check
tag_capability() {
  [ "$TAGCHECK" = 1 ] || { echo "  tag-check  : skipped (--no-tag-check)"; return 0; }
  printf 'struct vs_tag;\nclass vs_tag { public: int x; };\nvs_tag g;\n' > "$TMP/tag.cpp"
  # The SITE compiler is chosen for Win32 ability (it is usually mingw); the TAG check wants the
  # opposite capability, so they are resolved separately. Reporting "tag-blind" when a capable
  # compiler is sitting on the PATH would be a false limit.
  local tc="" c
  for c in clang++ "$CC"; do
    command -v "$c" >/dev/null 2>&1 || continue
    "$c" -std=c++98 -Wall -fsyntax-only "$TMP/tag.cpp" -o /dev/null > "$TMP/tag.log" 2>&1
    if grep -qi "mismatched-tags\|defined as a class here\|previously declared as a struct" "$TMP/tag.log"; then
      tc="$c"; break
    fi
  done
  if [ -n "$tc" ]; then
    TAG_CC="$tc"
    if [ "$tc" != "$CC" ]; then
      echo "  tag-check  : DETECTS BUG-007 via $tc (the site compiler $CC cannot emit this warning)"
    else
      echo "  tag-check  : DETECTS BUG-007 (struct/class tag mismatch)"
    fi
  else
    TAG_CC=""
    echo "  tag-check  : BLIND to BUG-007 on this host - no compiler here reports a struct/class"
    echo "               mismatch. Stated limit, NOT a pass: MSVC 2002 decorates the two"
    echo "               differently (docs/DEFERRED-NOTES.md:389) and nothing here can see it."
  fi
}

tag_findings() {
  [ "$TAGCHECK" = 1 ] || return 0
  local f log rc
  for f in "$@"; do
    log="$TMP/tag_$(basename "$f").log"
    rc="$(compile_site "$f" "$log")"
    if grep -qi "mismatched-tags\|previously declared as a struct" "$log" 2>/dev/null; then
      echo "  TAG-MISMATCH in $f:"
      grep -i "mismatched-tags\|previously declared" "$log" | head -2 | sed 's/^/      /'
    fi
  done
}

# ---------------------------------------------------------------- self-test
RC=0; LAST_OUT=""
run_check() {
  # Sets $RC and $LAST_OUT and returns the status - it does NOT print the status. Two drafts of
  # this helper were wrong before this one worked: (a) `( exec "$0" "$@" )` leaked the child's
  # whole report into $rc, so every scenario compared a multi-line string against "0"; and (b)
  # `run_check …; rc=$RC` ran the helper in a SUBSHELL, so LAST_OUT never reached the caller and
  # scenario 3 could not see the stale caller it had correctly found. A self-test that cannot
  # read its own evidence reports a PASS as confidently as a FAIL.
  LAST_OUT="$("$0" "$@" 2>&1)"
  RC=$?
  return "$RC"
}

expect() {
  # One scenario = one number AND one piece of evidence. Both are required, because an empty $RC
  # satisfies `[ "$rc" != 0 ]` exactly as a real failure does - and three of this self-test's
  # scenarios "passed" that way while capturing nothing. Requiring a matched pattern is what makes
  # a PASS mean the tool named the site, not merely refused.
  local name="$1" want="$2" pattern="$3"
  case "$RC" in 0|1|2) : ;; *) echo "  BUG   $name  (non-numeric rc '$RC')"; BAD=$((BAD + 1)); return ;; esac
  if [ "$want" = "ok" ] && [ "$RC" != 0 ]; then
    echo "  FAIL  $name  (expected exit 0, got $RC)"; BAD=$((BAD + 1)); return
  fi
  if [ "$want" = "bad" ] && [ "$RC" = 0 ]; then
    echo "  FAIL  $name  (expected a non-zero verdict, got a green)"; BAD=$((BAD + 1)); return
  fi
  if [ -n "$pattern" ] && ! printf '%s\n' "$LAST_OUT" | grep -q "$pattern"; then
    echo "  FAIL  $name  (verdict right, evidence missing: no line matched /$pattern/)"; BAD=$((BAD + 1)); return
  fi
  echo "  PASS  $name  (exit $RC)"
}

self_test() {
  BAD=0
  local scratch name
  scratch="$(mktemp -d)"
  echo "== self-test (falsification; scratch copy, repo untouched) =="
  echo "  root under test: $ROOT"

  # 1+2. controls - without these, every red below could be the harness being broken
  run_check --member F3::CombatStats::dtRowB --root "$ROOT"
  expect "control: dtRowB verifies through CombatMath.hpp" ok "VERIFIED"
  run_check --absent dtParts56 --root "$ROOT"
  expect "control: retired name survives only in historical prose" ok "VERIFIED"

  # 3. THE required falsification: rename the member in ONE header, leave the stale callers.
  mkdir -p "$scratch/modules" "$scratch/tools"
  # NOT `cp -a $ROOT/src $scratch/src` after that mkdir: with the destination dir already present
  # cp puts the tree at $scratch/src/src, so the sed below edited a file that was never there and
  # the scenario tested a tree nothing could see.
  cp -a "$ROOT/src" "$scratch" 2>/dev/null
  for m in "$ROOT"/modules/*/; do
    local mn; mn="$(basename "$m")"
    mkdir -p "$scratch/modules/$mn"
    [ -d "$m/src" ] && cp -a "$m/src" "$scratch/modules/$mn/src"
    [ -d "$m/tests" ] && cp -a "$m/tests" "$scratch/modules/$mn/tests"
  done
  name="$(basename "$0")"
  cp "$0" "$scratch/tools/$name"
  sed -i 's/\bdtRowB\b/dtRowZ/g' "$scratch/src/F3/CombatMath.hpp"

  run_check --absent dtRowB --root "$scratch"
  expect "stale callers NAMED when a header renames under them" bad "STALE CALLER"
  printf '%s\n' "$LAST_OUT" | grep "STALE CALLER" | head -3 | sed 's/.*STALE CALLER/          ->/'
  run_check --member F3::CombatStats::dtRowB --root "$scratch"
  expect "the probe itself refuses a member the header no longer declares" bad "FAILED\|UNVERIFIABLE"

  # 4. an invented member must not verify - guards the probe against being vacuous
  run_check --member F3::CombatStats::dtNoSuchThing --root "$ROOT"
  expect "invented member refused (probe asserts something)" bad "FAILED\|no code sites"

  # 5. a NON-historical comment mention must fail --absent: the vocab gate is a real gate
  printf '// the dtParts56 name\nstatic int vs_plain_prose_marker = 0;\n' > "$scratch/src/F3/vs_prose.cpp"
  run_check --absent dtParts56 --root "$scratch"
  expect "plain prose mention refused as 'historical'" bad "UNEXPLAINED"

  # 6. a forced-but-broken compiler is a setup error, never a substituted green
  run_check --member F3::CombatStats::dtRowB --root "$ROOT" --compiler /nonexistent/c++
  expect "unusable --compiler errors out instead of falling back" bad "SETUP ERROR"

  rm -rf "$scratch"
  echo "  boundary: the self-test proves the CHECKER discriminates. It says nothing about the repo,"
  echo "            and nothing at all about runtime behaviour - there is no exe in any of this."
  [ "$BAD" = 0 ] && return 0 || return 1
}

# ---------------------------------------------------------------- main
if [ "$SELFTEST" = 1 ]; then self_test; exit $?; fi

if [ -z "$MODE" ] || [ -z "$SYMBOL" ]; then usage >&2; exit 2; fi
NAME="${SYMBOL##*::}"
HITS="$(gather "$NAME")"
CODE_SITES="$(printf '%s\n' "$HITS" | awk -F: '$3=="CODE"{print $1":"$2}')"
CMT_SITES="$(printf '%s\n' "$HITS" | awk -F: '$3=="COMMENT"{print $1":"$2}')"

echo "verify-symbol: mode=$MODE symbol=$SYMBOL"
echo "  sites       : code=$(printf '%s\n' "$CODE_SITES" | grep -c . || true) comment=$(printf '%s\n' "$CMT_SITES" | grep -c . || true)"

fail=0
if [ "$MODE" = "absent" ]; then
  echo "== code sites (any hit is a failure) =="
  if [ -n "$CODE_SITES" ]; then
    printf '%s\n' "$CODE_SITES" | while read -r s; do [ -n "$s" ] && echo "  STALE CALLER  $s"; done
    fail=1
  else
    echo "  none"
  fi
  echo "== comment mentions (must be explicitly historical) =="
  if [ -z "$CMT_SITES" ]; then
    echo "  none"
  else
    : > "$TMP/cmtverdict"
    while read -r k rest; do
      echo "  $k  $rest"
      [ "$k" = "UNEXPLAINED" ] && echo x >> "$TMP/cmtverdict"
    done <<EOFC
$(printf '%s\n' "$HITS" | awk -F: -v vocab="$HIST_VOCAB" '$3=="COMMENT"{
      # Match the vocabulary against the comment TEXT (strip file:line:KIND: first). Matching the
      # whole record would let a FILE PATH containing a vocab token bless every mention in it.
      txt = tolower($0); sub(/^[^:]*:[0-9]*:[A-Z]*:/, "", txt)
      if (txt ~ vocab) print "HISTORICAL " $1 ":" $2
      else             print "UNEXPLAINED " $1 ":" $2
    }')
EOFC
    [ -s "$TMP/cmtverdict" ] && fail=1
  fi
  if [ "$fail" = 0 ]; then
    echo "== verdict =="
    echo "  VERIFIED: the name appears nowhere in code and only in historical prose."
  else
    echo "== verdict =="
    echo "  FAILED: remove the code sites above, or mark the prose as historical (state it was"
    echo "  renamed and by which row) if it is genuinely a retired-name note."
    boundary; exit 1
  fi
  boundary; exit 0
fi

echo "== header =="
if [ -z "$HEADER" ]; then
  rel_hdr="$(auto_header "$NAME")"
  if [ -z "$rel_hdr" ]; then
    if [ -z "$CODE_SITES" ]; then
      echo "  FAILED: $NAME has no code sites and no header declares it - the symbol does not"
      echo "  exist here. That is a verdict about the claim, not a setup problem."
      boundary; exit 1
    fi
    # Sites exist but NOTHING declares the name. That is not a setup problem - it is precisely
    # the LNK2019-class finding this tool exists to surface, so it is reported as a verdict.
    echo "  FAILED: no header declares $NAME, yet these sites reference it:"
    printf '%s\n' "$CODE_SITES" | while read -r c; do [ -n "$c" ] && echo "           $c"; done
    echo "  A link step would have failed here too; this host has no link step, so this is the"
    echo "  only place the bug can be caught today."
    boundary; exit 1
  fi
else
  rel_hdr="$HEADER"
fi
echo "  $rel_hdr"

echo "== probe =="
case "$MODE" in
  member|fn) body="  vs_take(&$SYMBOL);" ;;
  type)      body="  { int vs_sz = (int)sizeof($SYMBOL); (void)vs_sz; }" ;;
esac
prd="$(probe_compile "${rel_hdr#$ROOT/}" "$body")"
if [ "$prd" = "0" ]; then echo "  OK     resolves through the real header from a separate TU"
elif grep -qi "No such file or directory" "$TMP/probe.log"; then
  echo "  UNVERIFIABLE the probe TU could not be compiled here at all (platform header missing)"
  sed 's/^/      /' "$TMP/probe.log" | head -3
  fail=2
else echo "  FAILED probe did not compile:"; sed 's/^/      /' "$TMP/probe.log" | head -12; fail=1; fi

echo "== referencing sites (compiled, not just grepped) =="
if [ -z "$CODE_SITES" ]; then
  echo "  no code sites found - nothing references this name (see boundary: absence is not a"
  echo "  verification of the declaration, the probe above is what does that)"
else
  UNVER=0
  while read -r f; do
    [ -n "$f" ] || continue
    lrc="$(compile_site "$f" "$TMP/site.log")"
    case "$lrc" in
      ok)           echo "  OK     $f" ;;
      unverifiable) echo "  SKIP   $f  (host cannot compile it: platform header missing)"
                    UNVER=$((UNVER + 1)) ;;
      *)            echo "  FAILED $f"
                    grep -E "error:" "$TMP/site.log" | head -3 | sed 's/^/           /' ;;
    esac
  done <<EOF
$(printf '%s\n' "$CODE_SITES" | cut -d: -f1 | sort -u)
EOF
  if [ "$UNVER" -gt 0 ]; then
    echo "  NOTE: $UNVER site(s) could not be verified here (platform headers). That is an"
    echo "        unproven case, not a clean one - the name is confirmed for the rest."
  fi
  # real failures only: re-derive the verdict without a pipe (an exit status through a pipeline
  # is the LAST command's, which is exactly how a green gets faked)
  : > "$TMP/verdict"
  while read -r f; do
    [ -n "$f" ] || continue
    [ "$(compile_site "$f" "$TMP/check.log")" = "failed" ] && echo x >> "$TMP/verdict"
  done <<EOF
$(printf '%s\n' "$CODE_SITES" | cut -d: -f1 | sort -u)
EOF
  [ -s "$TMP/verdict" ] && fail=1
fi

tag_findings $(printf '%s\n' "$CODE_SITES" | cut -d: -f1 | sort -u)
tag_capability

echo "== verdict =="
if [ "$fail" = 2 ]; then
  echo "  UNVERIFIABLE: this host cannot compile the probe TU at all, so nothing about $SYMBOL"
  echo "  was established. Unverifiable is not verified - do not quote this as a pass."
  boundary
  exit 2
fi
if [ "$fail" = 0 ]; then
  echo "  VERIFIED: $SYMBOL resolves and every referencing site compiles."
  boundary
  exit 0
fi
echo "  FAILED: see the sites above. This is the LNK2019-class bug you would otherwise have"
echo "  discovered only at link time - which today you cannot reach at all."
boundary
exit 1
