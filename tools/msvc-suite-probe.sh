#!/usr/bin/env bash
# tools/msvc-suite-probe.sh - does MSVC 2002's cl compile the TEST SUITES, not just the product?
#
# WHY THIS EXISTS
# The wave is moving from "gcc builds the suites, MSVC builds the product" to MSVC-only. The product path
# is proven (44 sources compile under cl today), but the 14 suite TUs have only ever been compiled by
# i686-w64-mingw32-g++ -std=c++98. cl is stricter in ways this repo has already documented (C2440 x2:
# "MSVC-catches-mingw-slides"), so we need what cl actually says before the mingw row is removed - and a
# static grep for nullptr/long long/cstdint returning zero hits is a prediction, not an answer.
#
# It mirrors devenv/scripts/msvc-build.nix exactly: same image, same -v $PWD:/work, same vcvars32.bat
# bootstrap. Green here means the product's own compiler accepts the sources the suites run on today.
#
# Usage: tools/msvc-suite-probe.sh [SuiteName]        (default: Scripting)
#        tools/msvc-suite-probe.sh --all
set -uo pipefail

IMAGE="${MSVC_IMAGE:?MSVC_IMAGE not set - enter the devenv shell first}"
DOCKER="${OPENVB_DOCKER:-docker}"
JE=modules/openje/src/JE
TESTDIR=modules/openje/tests/je
SRC=modules/openje/src/JE.cpp

# Same test|impl pairing as devenv/scripts/run-all-tests.nix. Update both or neither.
suite_files() {
  case "$1" in
    Scripting)     printf '%s\n' "$TESTDIR/Scripting/Scripting.cpp" \
                       "$JE/Scripting.cpp" "$JE/ActionMoveTo.cpp" "$JE/Action.cpp" "$JE/PathManager.cpp" \
                       "$JE/Map.cpp" "$JE/SpatialGrid.cpp" "$JE/ResourceSystem.cpp" \
                       "$JE/Common/Chunk.cpp" "$JE/VFX_Parameter.cpp" "$JE/Resources.cpp" \
                       "$JE/Subsystem.cpp" "$JE/World.cpp" "$JE/Gregorian.cpp" "$SRC" ;;
    CombatMath)    printf '%s\n' "$TESTDIR/CombatMath/CombatMath.cpp" "src/F3/CombatMath.cpp" "$SRC" ;;
    cls_0x4d8d70)  printf '%s\n' "$TESTDIR/cls_0x4d8d70/cls_0x4d8d70.cpp" "$JE/cls_0x4d8d70.cpp" ;;
    *) return 1 ;;
  esac
}

probe_one() {
  local s="$1" files win
  files="$(suite_files "$s")" || { echo "  unknown suite '$s' (known: Scripting CombatMath cls_0x4d8d70)"; return 2; }

  # A probe that compiles nothing can only report green, so missing sources are fatal here, loudly.
  while IFS= read -r f; do
    [ -f "$f" ] || { echo "  REFUSE: '$f' is not in the tree"; return 3; }
  done <<< "$files"

  win="$(printf '%s\n' "$files" | sed 's#^#Z:\\work\\#; s#/#\\#g' | tr '\n' ' ')"

  local out line rc exe
  # The image's ENTRYPOINT is `bash -c`, so `-e VAR=` flags get consumed as the COMMAND (observed:
  # "/bin/bash: SUITE=cls_0x4d8d70: No such file or directory"). Environment has to be set INSIDE the
  # -lc script, which is also exactly how devenv/scripts/msvc-build.nix invokes this image.
  out="$($DOCKER run --rm -i --cap-add=NET_RAW --security-opt seccomp=unconfined \
      -v "$PWD:/work" -w /work "$IMAGE" -lc \
      "export SUITE=$s; export SRCWIN='$win'; bash /work/tools/msvc-suite-probe-inner.sh" 2>&1)"

  # VERDICT reads cl's propagated status from an explicit marker line, NOT docker's exit code: the first
  # version of this script took PIPESTATUS[0] of a pipeline ending in `tail` and printed "exit 0" for a
  # compile that had already failed. Instrument reporting the green it was about to conclude from.
  line="$(printf '%s\n' "$out" | tr '\r' '\n' | grep -E '^CLRC=' | tail -1)"
  rc="$(printf '%s' "$line" | sed -n 's/^CLRC=\([0-9]*\).*/\1/p')"
  exe="$(printf '%s' "$line" | sed -n 's/.*EXE=\([a-z]*\).*/\1/p')"

  printf '%s\n' "$out" | grep -E 'error|LNK4042|C[0-9]{4}:|warning C' | head -12 | sed 's/^/    /'
  if [ "${rc:-x}" = "0" ] && [ "${exe:-x}" = "yes" ]; then
    echo "  VERDICT $s: cl ACCEPTS it (exe built) - can move off mingw as-is"
    return 0
  fi
  echo "  VERDICT $s: cl REJECTS it (rc=${rc:-?} exe=${exe:-?} $(printf '%s' "$line" | sed -n 's/.*STAGE=\([a-z]*\).*/[failed at \1]/p')) - do not remove the mingw row for this suite yet"
  return 1
}

if [ "${1:-}" = "--all" ]; then
  fails=0
  for s in Scripting CombatMath cls_0x4d8d70; do probe_one "$s" || fails=$((fails+1)); done
  echo; echo "probe summary: $fails of 3 suites not yet cl-clean"
  exit $(( fails ? 1 : 0 ))
fi
probe_one "${1:-Scripting}"
