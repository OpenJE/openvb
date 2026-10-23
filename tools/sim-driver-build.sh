#!/usr/bin/env bash
# T-028 driver build — source list GENERATED from the run-all-tests table, never hand-copied
# (a copied list rots the moment a lane adds a cross-module dep; the table is the maintained
# source of truth for "what it takes to link the sim headless").
# This lane touches no cmake.toml / CMakeLists.txt / devenv scripts (wave rule 1): the only
# build-file relationship here is READING one.
set -euo pipefail
cd "$(git rev-parse --show-toplevel)"

TABLE="devenv/scripts/run-all-tests.nix"
ROW_GREP='^ *"PathManager\|'

# expand the two variable tokens the table uses
JE=$(sed -n 's/^\s*JE="\([^"]*\)".*/\1/p' "$TABLE" | head -1)
SRC=$(sed -n 's/^\s*SRC="\([^"]*\)".*/\1/p' "$TABLE" | head -1)
[ -n "$JE" ] && [ -n "$SRC" ] || { echo "sim-driver-build FAIL: JE/SRC vars not found in $TABLE (table moved? update this script, do not hard-code)"; exit 1; }

SRCS=$( { grep -E "$ROW_GREP" "$TABLE"; grep -E '^ *"SpawnMove\|' "$TABLE"; } | cut -d'|' -f3 | tr ' ' '\n' | sed '/^$/d' | sort -u | tr '\n' ' ' )
[ -n "$SRCS" ] || { echo "sim-driver-build FAIL: PathManager row not found in $TABLE — refusing to link a guess"; exit 1; }

SRCS=${SRCS//\$SRC/$SRC}
SRCS=${SRCS//\$JE/$JE}

# Compiler: MSVC 2002 cl in the docker image - the same toolchain the product and the test suites now use
# (tools/msvc-test-compile.sh). This was mingw g++ only because the suite harness was, and the requirement
# here was never "a cross compiler": the artifact has to be a Windows PE that runs under wine, and cl is
# literally a Windows compiler, so that requirement is satisfied natively rather than by cross-compiling.
CC_SH="tools/msvc-test-compile.sh"   # the script cd's to the repo toplevel, so this is relative to the tree root
[ -x "$CC_SH" ] || [ -f "$CC_SH" ] || { echo "sim-driver-build FAIL: $CC_SH missing"; exit 1; }
[ -n "${MSVC_IMAGE:-}" ] || { echo "sim-driver-build FAIL: MSVC_IMAGE not set - run inside the devenv shell"; exit 1; }

# the merged rows carry everything the T-030 seam's acceptance test links; no extra srcs needed.
EXTRA=""

echo "[sim-driver-build] sources generated from PathManager row ($(echo "$SRCS" | wc -w) files) + $EXTRA"
# `|| rc=$?` is load-bearing under `set -euo pipefail` at the top of this file: a bare call would abort the
# script on cl's nonzero status before the artifact check could say WHY it failed.
rc=0
bash "$CC_SH" tools/sim-driver.exe tools/sim-driver.cpp $SRCS $EXTRA || rc=$?
# Verdict comes from the artifact, not from a line count in the log: the gcc version of this script decided
# success by grepping for 6 occurrences of "Error", which passes on a build that produced no exe at all.
if [ "$rc" -ne 0 ] || [ ! -f tools/sim-driver.exe ]; then
  echo "sim-driver-build FAIL: cl rc=$rc, exe $([ -f tools/sim-driver.exe ] && echo present || echo ABSENT)"; exit 1
fi
echo "[sim-driver-build] OK -> tools/sim-driver.exe (MSVC cl)"
