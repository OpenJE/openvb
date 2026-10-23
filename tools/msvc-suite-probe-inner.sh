#!/usr/bin/env bash
# tools/msvc-suite-probe-inner.sh - runs INSIDE the msvc container (invoked by msvc-suite-probe.sh).
#
# Deliberately a separate file: this logic first lived inside a `docker run -lc "..."` double-quoted
# string, and the nested backslash/dollar escaping defeated two consecutive edits - invisibly, because the
# outer script kept running and reporting the PREVIOUS attempt's errors. Two files, one quoting level each.
#
# Contract (set by the caller): SUITE=name, SRCWIN="Z:\path\a.cpp Z:\path\b.cpp ..."
set -uo pipefail

SUITE="${SUITE:?SUITE not set}"
[ -n "${SRCWIN:-}" ] || { echo "CLRC=1 EXE=no STAGE=args"; echo "SRCWIN empty - refusing to report green on zero sources"; exit 1; }

OUTDIR='Z:\work\build\msvc-probe'
EXE_WIN="$OUTDIR\\$SUITE.exe"
EXE_LIN="/work/build/msvc-probe/$SUITE.exe"
mkdir -p /work/build/msvc-probe/obj

VC='call Z:\home\msvc2002\bin\vcvars32.bat >NUL'
INC='/I Z:\work\src /I Z:\work\modules\openje\src /I Z:\work\modules\tracing\src'
FLAGS='/nologo /c /EHsc /W0 /MT'    # /W0 ~ gcc -w ; /MT ~ -static ; /EHsc = the product target's own option

# One cl invocation per TU, each with a unique object path. This is the migration's real landmine, not
# cosmetics: several suites pair a test TU and an impl TU with the SAME basename (Scripting.cpp,
# ActionMoveTo.cpp, PathManager.cpp, ResourceSystem.cpp, cls_0x4d8d70.cpp). A single multi-file cl call
# sharing one /Fo directory collapses them onto one object: cl warns
#   "LNK4042: object specified more than once; extras ignored"
# and the surviving object is missing the other TU's symbols, so the link then reports LNK2019 for every
# function of the dropped file - which reads exactly like "cl cannot compile this codebase" and is nothing
# of the sort. gcc derives object names from the full source path, so the mingw harness never shows it.
# Observed on the very first probe run, on a suite with no C++ issues at all.
objs=""
i=0
rc=0
for src in $SRCWIN; do
  i=$((i+1))
  o="Z:\work\build\msvc-probe\obj\o$i.obj"
  echo "--- cl /c [$i] $src"
  wine cmd /c "$VC && cl $FLAGS /Fo$o $INC $src" || rc=1
  objs="$objs $o"
done

if [ "$rc" -ne 0 ]; then
  echo "CLRC=$rc EXE=no STAGE=compile"
  exit "$rc"
fi

# Explicit Win32 libs. gcc's mingw target auto-links the common Windows import libraries, MSVC does not:
# the first probe runs compiled every C++ TU cleanly and failed only at link, on
#   LNK2019: unresolved external symbol __imp__MessageBoxA@16  (in JE::FatalError)
# i.e. "cl cannot build the suites" would have been the wrong conclusion from a missing user32.lib. This is
# the product target's own list (cmake.toml windows.private-link-libraries = user32 gdi32 winmm shell32
# d3d8), minus d3d8 which no suite references - keep the two lists aligned if a suite ever draws render code.
echo "--- link ($i objects)"
wine cmd /c "$VC && link /nologo /OUT:$EXE_WIN $objs user32.lib gdi32.lib winmm.lib shell32.lib" || rc=1
if [ "$rc" -eq 0 ] && [ -f "$EXE_LIN" ]; then
  echo "CLRC=0 EXE=yes STAGE=done"
else
  echo "CLRC=$rc EXE=no STAGE=link"
  exit 1
fi
