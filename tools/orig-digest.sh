#!/bin/sh
# T-031 - tools/orig-digest.sh: emit the SHIPPED side of the simulation digest, in the SAME line
# grammar as src/F3/SimDigest.hpp, so tools/diff-digest.sh stays the only comparator in this repo.
#
# This is a thin entry point around tools/orig-digest.py, and the split is deliberate: the emit
# needs the F3 object database (4 MB of JSON) to turn `[action+0x00]` vftptrs into CLASS NAMES, and
# a shell JSON parser would be a second source of truth about that DB. The row promised `.sh` (or a
# .py, "matching the other offline tools"); this keeps the promised command name and puts the logic
# where it can be read.
#
# WHAT IT IS: a RENDERER FOR A CAPTURE FILE. It does not touch a running process. See
# docs/f3/OrigCapture.md for the capture grammar and docs/f3/ParityOracleDesign.md for why.
#
# WHAT IT REFUSES, and each of these is demonstrated by tools/orig-digest-verify.sh:
#   PROVENANCE REQUIRED      no --provenance and no record=provenance line  -> exit 3, nothing written
#   PROVENANCE-UNVERIFIED    claims `lockstep` without naming its reads / exe=<md5> / matching DB -> exit 3
#   INVARIANT-VIOLATION      frame indices out of order or duplicated; entity slots not dense;
#                            vt not exactly 4 slots; q above what the capture's own pointers imply;
#                            unknown record type or field; --require-resolved-ak with an UNKNOWN_VT
#                                                       -> exit 4, nothing written
# The provenance label decides what the FILE says: a capture that is not a verified lockstep read
# emits `# SYNTHETIC - exercises the tool, proves nothing about F3.exe`. Deliberately synthetic
# input is fine and expected; a synthetic side that does not SAY so is not.
#
# Exit codes: 0 wrote a digest | 2 usage/IO | 3 provenance | 4 invariant.

set -u

here=$(dirname "$0")
PY=${PYTHON:-python3}
command -v "$PY" >/dev/null 2>&1 || { echo "orig-digest: need python3 on PATH (or set PYTHON=)" >&2; exit 2; }
exec "$PY" "$here/orig-digest.py" "$@"
