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

# compiler: same binary the suite table uses (devenv/scripts/run-all-tests.nix: mingw32 cross).
if [ -n "${MINGW_CXX:-}" ]; then CXX="$MINGW_CXX"; elif command -v i686-w64-mingw32-g++ >/dev/null; then
  CXX=i686-w64-mingw32-g++
else
  CXX=$(ls /nix/store/*/bin/i686-w64-mingw32-g++ 2>/dev/null | head -1)
fi
if ! { [ -n "$CXX" ] && { [ -x "$CXX" ] || command -v "$CXX" >/dev/null 2>&1; }; }; then
  echo "sim-driver-build FAIL: mingw g++ not resolvable (set MINGW_CXX)"; exit 1
fi

# the merged rows carry everything the T-030 seam's acceptance test links; no extra srcs needed.
EXTRA=""

echo "[sim-driver-build] sources generated from PathManager row ($(echo "$SRCS" | wc -w) files) + $EXTRA"
"$CXX" -std=c++98 -static -w -Isrc -Imodules/openje/src -Imodules/tracing/src \
    tools/sim-driver.cpp $SRCS $EXTRA -o tools/sim-driver.exe
echo "[sim-driver-build] OK -> tools/sim-driver.exe"
