#!/usr/bin/env bash
# tools/msvc-test-compile-inner.sh - runs INSIDE the MSVC container; invoked by msvc-test-compile.sh.
#
# Contract (exported by the caller):
#   SRCWIN  space-separated Windows paths of the TUs to compile (Z:\work\...)
#   OUTWIN  Windows path of the exe to produce
#   OBJWIN  Windows directory for per-TU object files
#
# Split from the outer script on purpose: the first attempt embedded all of this in a `docker run -lc "..."`
# double-quoted string with inner `wine cmd /c \"...\"`, and the escaping layers fought each other the same
# way they did in tools/msvc-suite-probe.sh - where the visible symptom was an outer script reporting the
# PREVIOUS run's errors. One quoting level per file is the fix.

set -uo pipefail

: "${SRCWIN:?SRCWIN not set}"
: "${OUTWIN:?OUTWIN not set}"
: "${OBJWIN:?OBJWIN not set}"
[ -n "$SRCWIN" ] || { echo "REFUSE: no sources - a compile of nothing can only report green"; exit 3; }

# The container creates and destroys its own scratch: it runs as uid 100999, so host-side `rm -rf` on its
# leftovers fails with Permission denied and litters the tree.
mkdir -p "/work/${STAGE:-build/msvc-tests}/objs" || { echo "CLRC=1 EXE=no STAGE=args"; exit 1; }
trap 'rm -rf "/work/${STAGE:-build/msvc-tests}/objs"' EXIT

VC='call Z:\home\msvc2002\bin\vcvars32.bat >NUL'
INC='/I Z:\work\src /I Z:\work\modules\openje\src /I Z:\work\modules\tracing\src'
# /W0 ~ gcc's -w ; /MT ~ -static ; /EHsc matches cmake.toml's own msvc options.
# No /std:c++98 - VC7.0 has no such switch, C++98 is simply what it speaks.
FLAGS='/nologo /c /EHsc /W0 /MT'
# Tracks cmake.toml `windows.private-link-libraries`. d3d8 joins this list the day a suite links render
# code, and both lists change together - the last thing wanted here is a second compiler-shaped divergence.
LINKLIBS='user32.lib gdi32.lib winmm.lib shell32.lib'

# ONE cl INVOCATION PER TU, unique object names. Non-negotiable: several suites pair a test TU with an impl
# TU of the SAME basename (Scripting.cpp, ActionMoveTo.cpp, PathManager.cpp, ResourceSystem.cpp,
# cls_0x4d8d70.cpp). One cl call sharing a single /Fo directory fuses them into one object; cl then says
# LNK4042 "object specified more than once; extras ignored" and the link reports LNK2019 for every symbol
# of the vanished TU. gcc sidesteps this by naming objects after the full path, so the mingw harness never
# surfaced it - and untreated it looks precisely like "cl cannot compile this codebase".
objs=''
i=0
rc=0
for src in $SRCWIN; do
  i=$((i+1))
  o="$OBJWIN\\o$i.obj"
  if ! wine cmd /c "$VC && cl $FLAGS /Fo$o $INC $src"; then
    rc=1
    break        # a missing object must not reach the link line as a phantom operand
  fi
  objs="$objs $o"
done

if [ "$rc" -ne 0 ]; then
  echo "MSVC-COMPILE-FAILED rc=$rc"
  exit "$rc"
fi

wine cmd /c "$VC && link /nologo /OUT:$OUTWIN $objs $LINKLIBS" || rc=1
if [ "$rc" -ne 0 ]; then echo "MSVC-LINK-FAILED rc=$rc"; exit "$rc"; fi
echo "MSVC-BUILD-OK objects=$i"
