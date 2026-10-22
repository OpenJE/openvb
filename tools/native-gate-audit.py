#!/usr/bin/env python3
"""Audit the PORT's native gate calls against SHIPPED's own prologue immediates.

Why this exists separately from native-table-dump.py
---------------------------------------------------
`native-table-dump.py --audit` compares NAMES between the port's table and shipped's table, and compares
shipped's registered name against shipped's own gate string. This tool answers the OTHER question, the one
raised by the two-name law (docs/f3/NativeInstallerEvidence.md 5b): does each PORTED BODY pass to
ScriptNativeCheckArity the same (name, cells) pair that the shipped native pushes into 0x59AF40? Those are
different fields from the table's registered name, and they can diverge on either side.

COVERAGE-BEFORE-VERDICT (the rule this tool exists to enforce, named by NiceTiger)
--------------------------------------------------------------------------------
An audit that never shows its denominator can report "0 divergences" while parsing NOTHING. Two earlier
one-off attempts at this exact check did precisely that: a row regex that silently matched 32 rows, then 0,
both of which printed a clean-looking zero. So the expected population is derived from representations that
share NO failure path with the row parse:

  1. the array declarator in src/F3/ScriptNatives.cpp:  s_gameCreatureNatives[N]
  2. the suite's own recompute in .../ScriptNatives.cpp: check( nCount == M, ... )
  3. the number of rows parsed from the table block

All three must be equal, or the tool exits 1 WITHOUT printing a verdict. A shared bug cannot satisfy (1) and
(2) by accident, because they are different files, different syntax, and written by different slices.

Read-only: this tool never edits the TU it audits (that file has an owner in this project).

Usage:
    tools/native-gate-audit.py                        # audit, human-readable
    tools/native-gate-audit.py --json                 # machine-readable
    tools/native-gate-audit.py --selftest             # gates (exit 1 on failure)
Exit status: 0 = coverage holds and no unexpected divergence; 1 = anything else.
"""

import argparse
import json
import re
import struct
import sys
from pathlib import Path

# --- identity of the thing being read (a verdict without an exe identity is not reproducible) ---------
PINNED_SHA256 = "4652a3d03e7e09facab0ea123568a36c9be3b44ada80c1f60dac0449b3579b87"
IMAGE_BASE = 0x400000
GATE_EA = 0x59AF40            # the shared arity gate; the port's ScriptNativeCheckArity
RDATA_LO, RDATA_HI = 0x647000, 0x6F1000

# Shipped's OWN name defect, per row: the arity gate pushes a name different from the registered one.
# The port's job is to MIRROR it, so port gate == shipped gate is CORRECT here. This table exists to catch
# the opposite error: a kept defect silently "fixed" (a match against shipped would then vanish, and the
# evidence goes with it). Keyed row name -> (shipped gate name, cite).
EXPECTED_SHIPPED_NAME_DEFECT = {
    "UnequipItem": ("EquipItem", "0x45a72c: shipped's own gate pushes EquipItem's name; port mirrors it"),
}

ROOT = Path(__file__).resolve().parent.parent
TU = ROOT / "src/F3/ScriptNatives.cpp"
SUITE = ROOT / "modules/openje/tests/je/ScriptNatives/ScriptNatives.cpp"
EXE_DEFAULT = None


def sha256_of(path):
    import hashlib
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


class Pe:
    def __init__(self, path):
        self.d = open(path, "rb").read()
        pe = struct.unpack_from("<I", self.d, 0x3c)[0]
        nsec = struct.unpack_from("<H", self.d, pe + 6)[0]
        opt = struct.unpack_from("<H", self.d, pe + 20)[0]
        self.secs = []
        for i in range(nsec):
            o = pe + 24 + opt + i * 40
            # FIELD ORDER MATTERS: VirtualSize, VirtualAddress, SizeOfRawData, PointerToRawData.
            # Swapping these decodes garbage that still LOOKS structured - see harness notes item 69's
            # sibling; the gate() selftest below is what catches it, not a glance.
            vsize, vaddr, rsz, rptr = struct.unpack_from("<IIII", self.d, o + 8)
            self.secs.append((IMAGE_BASE + vaddr, rptr, rsz, vsize))

    def read(self, va, n):
        for sva, rptr, rsz, vsize in self.secs:
            if sva <= va < sva + vsize:
                off = rptr + (va - sva)
                if 0 <= off and off + n <= len(self.d):
                    return self.d[off:off + n]
        return b"\x00" * n

    def u32(self, va):
        b = self.read(va, 4)
        return struct.unpack("<I", b)[0] if len(b) == 4 else None

    def cstring(self, va, cap=90):
        b = self.read(va, cap)
        return b.split(b"\x00")[0].decode("latin1")


def shipped_gate(pe, fn_va, window=96):
    """Read the (name, cells) a shipped native pushes into the arity gate.

    Prologue shape: push <name-ptr>; push <imm8 cells>; push reg; push reg; call 0x59AF40.
    Returns None when the native has NO gate at all (the quest/journal alias case)."""
    code = pe.read(fn_va, window)
    for i in range(len(code) - 1):
        if code[i] == 0x68:                                    # push imm32
            nv = struct.unpack_from("<I", code, i + 1)[0]
            if RDATA_LO <= nv < RDATA_HI:
                for j in range(i + 5, min(i + 20, len(code) - 1)):
                    if code[j] == 0x6A:                         # push imm8  = cells
                        return {"name": pe.cstring(nv), "cells": code[j + 1],
                                "name_va": nv, "at": fn_va + i}
    return None


# --- port side: what each ported body actually passes ------------------------------------------------
ROW = re.compile(r'\{\s*"([^"]+)"\s*,\s*&(\w+)\s*,\s*(-?\d+)\s*\}')
DECL = re.compile(r"s_gameCreatureNatives\[(\d+)\]")
SUITE_COUNT = re.compile(r"check\(\s*nCount\s*==\s*(\d+)")
CHECK_ARITY = re.compile(r'ScriptNativeCheckArity\(\s*amx,\s*\w+,\s*(\d+)\s*,\s*"([^"]+)"')
RESOLVE_HELPER = re.compile(r'ResolveNativeEntity\(\s*amx,\s*\w+,\s*"([^"]+)"')
HELPER_CELLS = re.compile(r'ScriptNativeCheckArity\(\s*amx,\s*params,\s*(\d+)\s*,\s*pcFnName')


def parse_port_rows(text):
    """Rows from the table block, each with its body's gate call resolved."""
    m = re.search(r"static const ScriptNativeEntry s_gameCreatureNatives\[\d+\]\s*=\s*\{(.*?)\n\t\};",
                  text, re.S)
    if not m:
        return []
    block = m.group(1)
    rows = []
    for rm in ROW.finditer(block):
        name, sym, cells = rm.group(1), rm.group(2), int(rm.group(3))
        body = re.search(r"\n\tint %s\( int amx, int params \) \{(.*?)\n\t\} // %s"
                         % (sym, sym), text, re.S)
        gate = None
        if body:
            ca = CHECK_ARITY.search(body.group(1))
            rh = RESOLVE_HELPER.search(body.group(1))
            if ca:
                gate = {"name": ca.group(2), "cells": int(ca.group(1)), "via": "direct"}
            elif rh:
                hc = HELPER_CELLS.search(text)
                gate = {"name": rh.group(1), "cells": int(hc.group(1)) if hc else None,
                        "via": "ResolveNativeEntity"}
        rows.append({"name": name, "sym": sym, "cells": cells, "gate": gate,
                     "body_found": bool(body)})
    return rows


def audit(pe, tu_text, suite_text):
    decl = DECL.search(tu_text)
    suite_m = SUITE_COUNT.search(suite_text)
    rows = parse_port_rows(tu_text)
    coverage = {
        "declarator_count": int(decl.group(1)) if decl else None,
        "suite_asserted_count": int(suite_m.group(1)) if suite_m else None,
        "parsed_rows": len(rows),
    }
    # COVERAGE-BEFORE-VERDICT: refuse to render a verdict on an unproven population.
    c = set(v for v in coverage.values() if v is not None)
    if len(c) != 1:
        coverage["ok"] = False
        return coverage, None
    coverage["ok"] = True

    findings, kept, checked = [], [], 0
    for r in rows:
        if r["cells"] < 0:                      # port claims: no gate exists
            if not r["body_found"]:
                findings.append((r["name"], "port body not located"))
                continue
            if r["gate"]:
                findings.append((r["name"], "port declares -1 (no gate) but its body calls a gate"))
            checked += 1
            continue
        ea = None
        m = re.search(r'\{\s*"%s"[^/]*//\s*0x([0-9a-f]{6})' % re.escape(r["name"]), tu_text)
        if m:
            ea = int(m.group(1), 16)
        if ea is None:
            findings.append((r["name"], "no shipped EA in the row comment"))
            continue
        sg, pg = shipped_gate(pe, ea), r["gate"]
        if pg is None:
            findings.append((r["name"], "port body has no resolvable gate call (helper mismatch?)"))
            continue
        if sg is None:
            findings.append((r["name"], "shipped native at %#x has NO gate but port declares cells=%d"
                             % (ea, r["cells"])))
            continue
        checked += 1
        if (pg["name"], pg["cells"]) != (sg["name"], sg["cells"]):
            findings.append((r["name"], "DRIFT: port passes (%r,%d) but shipped pushes (%r,%d) at %#x"
                             % (pg["name"], pg["cells"], sg["name"], sg["cells"], ea)))
        # Now the separate, INVERSE check: a registered-name-vs-gate-name defect shipped has, that the port
        # is supposed to still mirror. If shipped's gate name stops differing, the defect was "fixed" and
        # the evidence disappeared - which is a finding, not a success.
        if r["name"] in EXPECTED_SHIPPED_NAME_DEFECT:
            want, cite = EXPECTED_SHIPPED_NAME_DEFECT[r["name"]]
            if sg["name"] != want:
                findings.append((r["name"], "shipped's gate no longer pushes %r (it pushes %r) - the kept "
                                            "defect moved; re-read 0x45a6f0 before trusting this row"
                                            % (want, sg["name"])))
            elif (pg["name"], pg["cells"]) != (sg["name"], sg["cells"]):
                findings.append((r["name"], "port failed to mirror shipped's name defect: port=(%r,%d) "
                                            "shipped=(%r,%d)" % (pg["name"], pg["cells"],
                                                                  sg["name"], sg["cells"])))
            else:
                kept.append((r["name"], (sg["name"], sg["cells"]), cite))
    return coverage, {"rows_checked": checked, "divergences": findings,
                     "kept_shipped_defects": kept, "total_rows": len(rows)}


def selftest():
    exe = default_exe()
    digest = sha256_of(exe)
    assert digest == PINNED_SHA256, "exe sha256 %s != pinned %s" % (digest, PINNED_SHA256)
    pe = Pe(exe)
    # 1. the gate reader must find a known (name, cells) and a known divergence.
    g = shipped_gate(pe, 0x45b900)
    assert g == {"name": "OpenTask", "cells": 2, "name_va": g["name_va"], "at": g["at"]}, g
    assert shipped_gate(pe, 0x45a6f0)["name"] == "EquipItem", "UnequipItem's shipped defect moved"
    # 1b. and the port must be MIRRORING it, not fixing it.
    tu_probe = TU.read_text()
    cov_probe, v_probe = audit(pe, tu_probe, SUITE.read_text())
    assert cov_probe.get("ok"), "coverage gate fired on the real tree: %s" % cov_probe
    assert [k[0] for k in v_probe["kept_shipped_defects"]] == ["UnequipItem"], v_probe["kept_shipped_defects"]
    assert v_probe["divergences"] == [], "real tree must be drift-free: %s" % v_probe["divergences"]
    # 2. the ungated alias body must yield NO gate (else -1 rows would be wrong).
    assert shipped_gate(pe, 0x420990, window=16) is None, "0x420990 must have no arity gate"
    # 3. a known cell count from a helper-mediated native.
    assert shipped_gate(pe, 0x45abe0)["cells"] == 1, "GetLevel cells != 1"
    # 4. COVERAGE-BEFORE-VERDICT must actually fire: feed it a suite text with a wrong number.
    tu = TU.read_text()
    cov, verdict = audit(pe, tu, "check( nCount == 999, \"x\" );")
    assert cov["ok"] is False and verdict is None, "coverage gate did not fire on a mismatched denominator"
    print("selftest PASS: sha pinned, gate reader correct on 4 natives, ungated alias recognised, "
          "coverage gate fires")


def default_exe():
    import os
    p = os.environ.get("OPENVB_ORIG_EXE")
    return Path(p) if p else None


def main():
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    ap.add_argument("exe", nargs="?", help="original F3.exe (default: $OPENVB_ORIG_EXE)")
    ap.add_argument("--json", action="store_true")
    ap.add_argument("--selftest", action="store_true")
    a = ap.parse_args()
    if a.selftest:
        selftest()
        return 0
    exe = Path(a.exe) if a.exe else default_exe()
    if not exe:
        print("no exe: pass a path or set $OPENVB_ORIG_EXE", file=sys.stderr)
        return 1
    digest = sha256_of(exe)
    if digest != PINNED_SHA256:
        print("REFUSING: %s sha256 %s != pinned %s" % (exe, digest, PINNED_SHA256), file=sys.stderr)
        return 1
    pe = Pe(exe)
    coverage, verdict = audit(pe, TU.read_text(), SUITE.read_text())
    if a.json:
        print(json.dumps({"coverage": coverage, "verdict": verdict}, indent=1))
    else:
        print("coverage: %s" % coverage)
        if not coverage.get("ok"):
            print("REFUSING TO RENDER A VERDICT: the three independent population counts disagree, so the\n"
                  "row parse is not proven to cover the table. An audit whose denominator was never shown is\n"
                  "unexecuted, not green. (See COVERAGE-BEFORE-VERDICT in this file's header.)")
            return 1
        print("rows checked: %d of %d" % (verdict["rows_checked"], verdict["total_rows"]))
        for k in verdict["kept_shipped_defects"]:
            print("  MIRRORED SHIPPED DEFECT (correct, do not 'fix'): %s gate=(%r,%d)  [%s]"
                  % (k[0], k[1][0], k[1][1], k[2]))
        if verdict["divergences"]:
            for f in verdict["divergences"]:
                print("  DIVERGENCE: %s: %s" % f)
            return 1
        print("=> every ported body's gate (name, cells) equals shipped's prologue immediates, and the kept\n"
              "   defect is still kept. Denominators agree, so this verdict covers all %d rows."
              % verdict["total_rows"])
    return 0


if __name__ == "__main__":
    sys.exit(main())
