#!/usr/bin/env python3
"""tools/struct-layout-check.py - does a port header's NAMED members actually sit at the offsets in their names?

USAGE
    tools/struct-layout-check.py <header.hpp> <Class> [--srcs "a.cpp b.cpp ..."] [--size N]

WHY THIS EXISTS (earned, not decorative)
    GameWeapon's Read-era suite passed 45 asserts and then died on `movl (%edx)` with EDX=0. The cause was
    layout, not logic: the product wrote cells by RAW offset ((char*)&w + 0x3B0) while the test read them by
    NAME (w.mbr_0x3b0), and the header had no explicit padding, so names and true addresses diverged. Answering
    that question by hand took a generated probe TU, a cl build, a wine run, and a table - about an hour. It is
    one command, and it is the mechanical form of a rule this project states in prose ("DB member listings are
    excerpts", "base:true and base:false are different coordinate systems"), which means it currently depends
    on someone remembering to be suspicious.

WHAT IT REPORTS
    sizeof(port type) vs the DB size, then every member whose name encodes an offset (mbr_0xNNN, gap_0xNNN):
    implied offset, real address, drift. Any CHANGE in drift is printed as a break point, because the useful
    signal is not "there is drift" but "drift starts here" - GameWeapon was uniform +440 and stepped to +428
    at mbr_0x3e0, which localises the missing pad to the byte/dword pair immediately before it. That two-break
    shape is the reason this tool's output format is a table and not a boolean.

WHAT IT CANNOT DO, stated so a green here is not over-read
    - It measures the PORT's layout only. The shipped layout comes from artifacts/F3.json member offsets and,
      when the DB is silent (it lists ~30 named members for a 2056-byte class), from store instructions. Names
      matching the DB's offsets proves anchoring, not that the DB's list was complete.
    - It cannot tell you whether a member SHOULD exist at an offset. A cell the DB does not list may still be
      real: GameWeapon's mbr_0x3cc was dismissed as "DB has no member there" and then cited at
      `lea edx,[esi+3CCh]; push edx; call sub_49C8D0` @0x412d5c inside the PARENT reader - a different function
      from the 0x414xxx method the member-usages scan covered. Absence in a scoped scan is not absence at the
      metal, so a "no such member" verdict needs the scan's scope named.
    - If A-F style tests assert by name against an unanchored type they agree with the product in the SAME wrong
      axis, and stay green. A green suite is not evidence of byte parity; this tool is the evidence for that.
"""

import json
import os
import re
import subprocess
import sys
import tempfile

DB = os.environ.get("OPENVB_TYPE_DB", "artifacts/F3.json")
OFFSET_NAME = re.compile(r"^(?:mbr|gap)_0x([0-9a-fA-F]{2,4})(?:_\d+)?$")
# `dword mbr_0x3b0;` / `byte gap_0x3d1[11];` / `void* mbr_0x40c;` - declarations only, comments stripped.
DECL = re.compile(
    r"^\s*(?:dword|word|byte|int|char|float|unsigned\s+char|unsigned\s+int|void\s*\*?|"
    r"signed\s+char|bool|__int64|long|short)\s+((?:mbr|gap)_0x[0-9a-fA-F]{2,4}(?:_\d+)?)"
    r"\s*(\[[^\]]*\])?\s*;"
)


def strip_comments(text):
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    return re.sub(r"//[^\n]*", "", text)


def class_body(src, cls):
    """Return the brace-balanced body of `class/struct cls`, or None."""
    m = re.search(r"\b(?:class|struct)\s+" + re.escape(cls) + r"\b[^{;]*\{", src)
    if not m:
        return None
    i = m.end() - 1
    depth = 0
    for j in range(i, len(src)):
        if src[j] == "{":
            depth += 1
        elif src[j] == "}":
            depth -= 1
            if depth == 0:
                return src[i : j + 1]
    return None


def members(body):
    out = []
    for line in body.splitlines():
        d = DECL.match(line)
        if not d:
            continue
        name, arr = d.group(1), d.group(2)
        om = OFFSET_NAME.match(name)
        if not om:
            continue
        out.append((name, int(om.group(1), 16), bool(arr)))
    return out


def db_size(cls):
    try:
        j = json.load(open(DB))
    except Exception as e:
        return None, "no DB (%s)" % e
    for key, c in j.get("structures", {}).items():
        if (c.get("demangled_name") or "").lower() == cls.lower() or key.lower() == cls.lower():
            offs = {}
            for k, v in (c.get("members") or {}).items():
                try:
                    offs[int(v.get("offset", "0x0"), 16)] = v.get("name", k)
                except Exception:
                    pass
            return c.get("size"), offs
    return None, {}


def simulate(header, cls):
    """Arithmetic layout: MSVC x86 rules, no compiler. Instant, and answers the 90% case.

    Alignment per MSVC x86 default packing: a scalar aligns to its own size capped at 4 (dword/int/float/ptr
    = 4, word/short = 2, byte/char = 1); an array aligns like its element. This is exactly the rule whose
    ABSENCE-of-pad bit WildUnion: byte mbr_0x3d0 then dword mbr_0x3e0 put 0x3e0 twelve bytes early."""
    src = strip_comments(open(header).read())
    body = class_body(src, cls)
    if body is None:
        sys.exit("class/struct %s not found in %s" % (cls, header))
    mem = members(body)
    if not mem:
        sys.exit("no offset-encoded members in %s::%s" % (header, cls))
    SIZE = {"dword": 4, "unsigned int": 4, "int": 4, "float": 4, "void*": 4, "word": 2, "short": 2,
            "byte": 1, "char": 1, "unsigned char": 1, "signed char": 1, "bool": 1}
    rows, cursor = [], 0
    # If the type is virtual and the header does not declare the vptr explicitly, the DB says it is there
    # (vftable_count >= 1) and it occupies offset 0. Account for it so the first named member's drift is not
    # polluted by a missing 4 bytes nobody listed.
    j = json.load(open(DB)) if os.path.exists(DB) else {}
    vf = 0
    for key, c in j.get("structures", {}).items():
        if (c.get("demangled_name") or "").lower() == cls.lower() or key.lower() == cls.lower():
            vf = int(c.get("vftable_count") or 0)
            break
    if vf and not re.search(r"vft|vptr|vfptr", body.split(";")[0]):
        cursor = 4   # vptr occupies 0..3
    for name, implied, is_arr in mem:
        dm = DECL.match(re.search(r"^\s*.*" + re.escape(name) + r"[^;\n]*;", body, re.M).group(0))
        ty = dm.group(0).strip().split()[0]
        n = 1
        if is_arr:
            n = int(re.search(r"\[\s*(0x[0-9a-fA-F]+|\d+)\s*\]", dm.group(0)).group(1), 0)
        esz = SIZE.get(ty, 4)
        al = min(esz, 4)
        cursor = (cursor + al - 1) // al * al
        rows.append((name, implied, cursor, cursor - implied))
        cursor += esz * n
    align = 4
    total = (cursor + align - 1) // align * align
    return rows, total, vf
def main():
    if len(sys.argv) < 3:
        print(__doc__)
        sys.exit(2)
    header, cls = sys.argv[1], sys.argv[2]
    srcs, want_size = [], None
    if "--srcs" in sys.argv:
        i = sys.argv.index("--srcs")
        srcs = sys.argv[i + 1].split()
    if "--size" in sys.argv:
        want_size = int(sys.argv[sys.argv.index("--size") + 1])

    if "--probe" in sys.argv:
        lines = build_and_run(header, cls, srcs, [])
        if not lines:
            sys.exit(1)
        size, rows = None, []
        for l in lines:
            if l.startswith("SIZE="):
                size = int(l.split("=")[1])
            elif l.startswith("MEM "):
                _, name, kind, actual, drift = l.split()
                rows.append((name, kind.replace("arr", "sca"), int(actual), int(drift)))
    else:
        # Default: arithmetic only - no compiler, no docker, no wine. Runs in milliseconds. The probe exists
        # for when the arithmetic result must be confirmed by the actual compiler, and is opt-in for speed.
        rows0, total, vf = simulate(header, cls)
        size = total
        rows = [(n, "sim", a, d) for n, _i, a, d in rows0]

    dsize, offs = db_size(cls)
    print("=" * 78)
    print("struct-layout-check: %s :: %s" % (header, cls))
    print("-" * 78)
    if rows and rows[0][1] == "sim":
        print("declared span (sim) = %s   DB = %s" % (size, dsize))
        print("  NOTE: the sim walks only the members this tool parses, so its span is NOT the compiler's")
        print("  sizeof - any member typed outside the recognised set (class objects, nested structs) is")
        print("  invisible to it, and v1 of this tool printed that span as 'sizeof' and thereby lied by")
        print("  omission. For the real sizeof, run --probe (cl, pointer-cast, one TU, ~40s).")
    else:
        print("sizeof(port, cl) = %s   DB = %s%s" % (size, dsize,
              "" if not dsize else ("   MISMATCH of %d bytes" % (dsize - size) if size != dsize else "   OK")))
    if want_size is not None:
        print("requested guard size = 0x%X (%d)" % (want_size, want_size))
    print("-" * 78)
    print("%-16s %-4s %10s %10s %8s" % ("member", "kind", "actual", "drift", "note"))
    prev = None
    breaks = []
    for name, kind, actual, drift in rows:
        note = ""
        if prev is not None and drift != prev:
            note = "<-- BREAK (%+d)" % (drift - prev)
            breaks.append((name, prev, drift))
        elif drift != 0 and prev is None:
            note = "<-- first member already off"
        print("%-16s %-4s %10d %10d  %s%s" % (name, kind, actual, drift, note,
              "" if drift == 0 else ""))
        prev = drift
    print("-" * 78)
    zero = [r for r in rows if r[3] != 0]
    if not zero and (dsize is None or size == dsize):
        print("VERDICT: anchored - every name matches its address, and sizeof matches the DB.")
    else:
        if zero:
            print("VERDICT: NOT ANCHORED - %d/%d named members sit at the wrong address." % (len(zero), len(rows)))
            print("  Named access and raw-offset access are therefore DIFFERENT CELLS. A suite that asserts by")
            print("  name against this type agrees with the product in the same wrong axis and proves nothing")
            print("  about shipped's bytes.")
        for name, a, b in breaks:
            print("  drift changes at %s: %+d -> %+d (a pad is missing just above it)" % (name, a, b))
        print("  Cure: explicit gap arrays sized by subtraction (see modules/openje/src/JE/ActionMoveTo.hpp:30,")
        print("  `byte gap_0xNN[0xnext-0xNN];` plus ctor fill loops so they cannot rot).")
        print("  Then make it unremovable - a C++98 tripwire that fails the BUILD, not the test:")
        if dsize:
            print('      typedef char %sSizeCheck[( sizeof(%s) == %d ) ? 1 : -1];' % (cls, cls, dsize))
        print("  Enable it ONLY once the anchoring lands; enabled before, it just reports the same drift.")
    sys.exit(0)


main()
