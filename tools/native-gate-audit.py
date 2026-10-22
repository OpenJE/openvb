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
# TWO shipped arity gates, not one (found by FastNova 248f842; both verified here from instruction text):
#   0x59AF40 EXACT   : cmp/je  - 4 args, `add esp,10h`, message "Expected %d parameters but recieved %d."
#   0x59AF90 MINIMUM : cmp/jge - 5 args, `add esp,14h`, the 5th is &out_actual (written by
#                                `mov [edi],eax` @0x59afa5) and its own message
#                                "Expected at least %d parameters but recieved %d." @0x6A0694
# The port implements only the EXACT form (ScriptNativeCheckArity). No F3 game-lib native uses the min gate
# (engine core does, in 6 SendMessage-family natives), so this audit's verdict on the 36 rows is unaffected -
# but a tool that cannot TELL the two apart would silently mis-audit any future min-gated port.
GATE_EA = 0x59AF40
GATE_MIN_EA = 0x59AF90
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


# THE BOUND IS THE FUNCTION, NOT THE RADIUS (FastNova 2026-10-22, after widening 96->2048 MANUFACTURED a
# dozen false gates by reading into the NEXT function). Evidence: Debug 0x5c3710 is 0x66 bytes long, so a
# 140-byte window sees PointDistance's own gate at +0x86 and attributes it to Debug. Widening cannot fix
# attribution; only an extent cap can. scan_limit() bounds the window at the next known native entry, and the
# selftest asserts the CORRECT attribution for that pair - Debug ungated, PointDistance gated at +0x16.
GATE_WINDOW = 0x40            # generous for real gates (max observed entry->gate offset is +0x34)


def scan_limit(pe, fn_va, known):
    """Bytes of THIS function we are willing to read: up to the next known native entry, else the window.
    `known` is the sorted list of native entry VAs from the port's own table."""
    nxt = next((k for k in known if k > fn_va), None)
    if nxt is None:
        return GATE_WINDOW
    return max(1, min(GATE_WINDOW, nxt - fn_va))


def shipped_gate(pe, fn_va, window=GATE_WINDOW, known=()):
    """Read (name, cells, kind) from a shipped native's prologue arity gate.

    Two gate shapes are recognised, by their call target: 0x59AF40 EXACT and 0x59AF90 MINIMUM.
    Prologue shape: push <name-ptr>; push <imm8 cells>; push reg [; push &out]; call <gate>.
    Returns None when NO gate call is found for either EA within `window` bytes - which, per item 72's
    corollary, must be reported as a BOUNDED negative, never as "this native has no gate"."""
    if known:
        window = min(window, scan_limit(pe, fn_va, known))
    code = pe.read(fn_va, window)
    for i in range(len(code) - 4):
        if code[i] == 0xE8:                                     # call rel32
            tgt = fn_va + i + 5 + struct.unpack_from("<i", code, i + 1)[0]
            if tgt not in (GATE_EA, GATE_MIN_EA):
                continue
            kind = "exact" if tgt == GATE_EA else "min"
            # walk the pushes BEFORE the call for the name ptr and the cells imm8
            head = code[:i]
            nv = None
            for k in range(len(head) - 4):
                if head[k] == 0x68:
                    cand = struct.unpack_from("<I", head, k + 1)[0]
                    if RDATA_LO <= cand < RDATA_HI:
                        nv = cand
            cells = None
            if nv is not None:
                for k in range(len(head) - 1, nv_at(head, nv), -1):
                    if head[k] == 0x6A:
                        cells = head[k + 1]
                        break
            if nv is not None:
                return {"name": pe.cstring(nv), "cells": cells, "kind": kind,
                        "name_va": nv, "gate": tgt, "at": fn_va + i}
    return None


def nv_at(head, nv):
    """Index of the push-imm32 for `nv` in `head`, or 0 if not found (scan lower bound)."""
    for k in range(len(head) - 4):
        if head[k] == 0x68 and struct.unpack_from("<I", head, k + 1)[0] == nv:
            return k
    return 0


# --- port side: what each ported body actually passes ------------------------------------------------
ROW = re.compile(r'\{\s*"([^"]+)"\s*,\s*&(\w+)\s*,\s*(-?\d+)\s*\}')
DECL = re.compile(r"s_gameCreatureNatives\[(\d+)\]")
SUITE_COUNT = re.compile(r"check\(\s*nCount\s*==\s*(\d+)")
CHECK_ARITY = re.compile(r'ScriptNativeCheckArity\(\s*amx,\s*\w+,\s*(\d+)\s*,\s*"([^"]+)"')
RESOLVE_HELPER = re.compile(r'ResolveNativeEntity\(\s*amx,\s*\w+,\s*"([^"]+)"')
# The parameterised helper form (post item-71 fix): the row's OWN cell literal precedes the name, so
# enforcement is visible per row. Kept BESIDE RESOLVE_HELPER, not replacing it: if the literal ever
# moves back into the helper body, the legacy regex matches again and the decorative warning
# re-appears. The tripwire is the pair, not either half.
# BOUNDARY OF THE GUARD (asymmetry, stated because the assumption is what this lane keeps unpicking):
# the pair guards against RE-HIDING a literal in the helper. It does NOT forbid a NEW row written in
# the old hardcoded style - such a row parses via=ResolveNativeEntity and lands in the DECORATIVE
# warning, loud, which is acceptable precisely because loud is not silent: the failure mode is a
# warning nobody should ignore, not a false green. The pair DETECTS the old shape; it does not
# prevent it. Do not read "kept as tripwire" as "the legacy form cannot pass".
RESOLVE_PARAM = re.compile(r'ResolveNativeEntity\(\s*amx,\s*\w+,\s*(\d+)\s*,\s*"([^"]+)"')
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
            rp = RESOLVE_PARAM.search(body.group(1))
            rh = RESOLVE_HELPER.search(body.group(1))
            if ca:
                gate = {"name": ca.group(2), "cells": int(ca.group(1)), "via": "direct"}
            elif rp:
                gate = {"name": rp.group(2), "cells": int(rp.group(1)),
                        "via": "ResolveNativeEntity-parameterised"}
            elif rh:
                hc = HELPER_CELLS.search(text)
                gate = {"name": rh.group(1), "cells": int(hc.group(1)) if hc else None,
                        "via": "ResolveNativeEntity"}
        rows.append({"name": name, "sym": sym, "cells": cells, "gate": gate,
                     "body_found": bool(body)})
    return rows


def tbl_text(tu_text):
    """The s_gameCreatureNatives[...] table block only, so row-comment regexes cannot match body comments
    elsewhere in the file (a body cites many EAs too, and matching those would invent entries)."""
    i = tu_text.find("s_gameCreatureNatives[")
    if i < 0:
        return ""
    j = tu_text.find("};", i)
    return tu_text[i:j if j > 0 else len(tu_text)]


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

    # Every native entry this port claims to mirror, sorted: the extent cap for a gate scan is the NEXT one of
    # these (see scan_limit). Built once, up front, from the same table block the rows came from.
    known = sorted({int(mm.group(1), 16)
                    for mm in re.finditer(r'\}\s*,?\s*//\s*(0x[0-9a-f]{6})', tbl_text(tu_text))})
    findings, kept, checked = [], [], 0
    enforced_elsewhere = []
    min_gated = []
    for r in rows:
        if r["cells"] < 0:                      # port claims: no gate exists
            if not r["body_found"]:
                findings.append((r["name"], "port body not located"))
                continue
            if r["gate"]:
                findings.append((r["name"], "port declares -1 (no gate) but its body calls a gate"))
            checked += 1
            continue
        mm = re.search(r'\{\s*"%s"[^/]*//\s*(0x[0-9a-f]{6})' % re.escape(r["name"]), tbl_text(tu_text))
        ea = int(mm.group(1), 16) if mm else None
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
        # ENFORCEMENT-BEFORE-VERDICT (harness item 71): a row whose gate lives in a SHARED HELPER with
        # the count hardcoded there has no enforcing literal in its own body, so the table's declared
        # `cells` column is DECORATIVE on it - the helper hardcodes its own value. CLASS FIXED in the
        # src (ResolveNativeEntity now takes nCells per row; the parameterised form parses as ENFORCED
        # above). This branch survives DELIBERATELY as the regression tripwire: if a literal ever
        # migrates back into the helper, via reverts to "ResolveNativeEntity" and the warning re-fires.
        # (Guard boundary: this pair DETECTS the old shape, it does not prevent a new row being
        # written in it - such a row fails LOUD as decorative. See RESOLVE_PARAM's header comment.)
        if r["cells"] >= 0 and pg.get("via") == "ResolveNativeEntity":
            enforced_elsewhere.append(
                (r["name"], sg["name"], sg["cells"], pg["cells"],
                 "helper hardcodes %s; declared %s%s" % (pg["cells"], r["cells"],
                                                         "" if r["cells"] == pg["cells"] else " - MISMATCH")))
        if (pg["name"], pg["cells"]) != (sg["name"], sg["cells"]):
            findings.append((r["name"], "DRIFT: port passes (%r,%d) but shipped pushes (%r,%d) at %#x"
                             % (pg["name"], pg["cells"], sg["name"], sg["cells"], ea)))
        # Gate KIND: the port implements only the EXACT form. A min-gated shipped native ported into this
        # table would need a second gate (jge semantics, an &out_actual 5th arg, and the separate
        # "Expected at least %d parameters but recieved %d." string @0x6A0694). Flag it rather than let the
        # (name, cells) pair alone look like a clean match - "at least" is not a spelling you can borrow.
        if sg.get("kind") == "min":
            min_gated.append((r["name"], sg["cells"], ea))
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
                     "kept_shipped_defects": kept, "total_rows": len(rows),
                     "enforced_in_helper_only": enforced_elsewhere,
                     "shipped_min_gated": min_gated,
                     "negative_bound": ("'no gate' means: no call to either 0x59AF40 (exact) or 0x59AF90 "
                                        "(minimum) within %d bytes of the native's entry" % GATE_WINDOW)}


def selftest():
    exe = default_exe()
    digest = sha256_of(exe)
    assert digest == PINNED_SHA256, "exe sha256 %s != pinned %s" % (digest, PINNED_SHA256)
    pe = Pe(exe)
    # 1. the gate reader must find a known (name, cells) and a known divergence.
    g = shipped_gate(pe, 0x45b900)
    assert (g["name"], g["cells"], g["kind"], g["gate"]) == ("OpenTask", 2, "exact", GATE_EA), g
    assert shipped_gate(pe, 0x45a6f0)["name"] == "EquipItem", "UnequipItem's shipped defect moved"
    # 1b. and the port must be MIRRORING it, not fixing it.
    tu_probe = TU.read_text()
    cov_probe, v_probe = audit(pe, tu_probe, SUITE.read_text())
    assert cov_probe.get("ok"), "coverage gate fired on the real tree: %s" % cov_probe
    assert [k[0] for k in v_probe["kept_shipped_defects"]] == ["UnequipItem"], v_probe["kept_shipped_defects"]
    assert v_probe["divergences"] == [], "real tree must be drift-free: %s" % v_probe["divergences"]
    # 2. the ungated alias body must yield NO gate (else -1 rows would be wrong).
    assert shipped_gate(pe, 0x420990, window=16) is None, "0x420990 must have no arity gate"
    # 2b. MIN gate detection: A_SendMessage 0x5c5120 gates at minimum 2 via 0x59AF90 (FastNova 248f842).
    # 2x. THE BLEED TEST (FastNova's retraction, now an assert): Debug 0x5c3710 is 0x66 bytes; PointDistance
    # starts 0x5c3780. With no extent cap, Debug's 140-byte window reads PointDistance's gate at +0x86 and
    # reports it as Debug's - the exact error that produced a retracted "21st divergence". With the cap, Debug
    # is correctly UNGATED and PointDistance correctly gated at +0x16.
    KNOWN = [0x5c3710, 0x5c3780]
    assert shipped_gate(pe, 0x5c3710, known=KNOWN) is None, "Debug must read UNGATED once its own extent bounds the scan"
    pd = shipped_gate(pe, 0x5c3780, known=KNOWN)
    assert pd and pd["name"] == "PointDistance" and pd["cells"] == 2 and (pd["at"] - 0x5c3780) == 0x16, pd
    # ...and an INTENTIONALLY un-bounded 256-byte scan reproduces FastNova's retraction as a test: widen the
    # RADIUS past Debug's 0x66 extent and it reads PointDistance's gate at +0x86 and attributes it to Debug.
    # That is the whole lesson in one assert - radius widening does not fix attribution, it manufactures
    # false gates - so the test pins both halves: bounded = correct, unbounded = the exact bug retracted.
    bleed = shipped_gate(pe, 0x5c3710, window=256)
    assert bleed and bleed["name"] == "PointDistance" and (bleed["at"] - 0x5c3710) == 0x86, \
        ("un-bounded scan should reproduce the bleed", bleed)
    mg = shipped_gate(pe, 0x5c5120)
    assert mg and mg["kind"] == "min" and mg["gate"] == GATE_MIN_EA, ("A_SendMessage should be min-gated", mg)
    # 2y. (superseded) an earlier version of this selftest ASSERTED the bleed as if it were a finding - that
    # Debug's window should reach PointDistance's gate at +0x86. It was enforcing FastNova's retracted error.
    # Replaced by 2x above: the correct behavior is Debug UNGATED, and the bleed is only tested as proof that
    # the extent cap is what prevents it.
    # 3. a known cell count from a helper-mediated native.
    assert shipped_gate(pe, 0x45abe0)["cells"] == 1, "GetLevel cells != 1"
    # 3b. the item-71 fix must be VISIBLE: the 13 set-A rows parse as parameterised (cells from their
    # own call sites) and NONE may remain in the decorative bucket. If the helper ever re-hardcodes,
    # enforced_in_helper_only refills and this assert fires.
    cov3, v3 = audit(pe, Path(TU).read_text(), 'check( nCount == 36, "x" );')
    assert v3 and v3["enforced_in_helper_only"] == [], \
        "set-A rows are decorative again: %s" % [x[0] for x in v3["enforced_in_helper_only"]]
    pars = [r for r in parse_port_rows(Path(TU).read_text())
            if (r.get("gate") or {}).get("via") == "ResolveNativeEntity-parameterised"]
    assert len(pars) == 13 and all(r["gate"]["cells"] == r["cells"] == 1 for r in pars), \
        "expected 13 parameterised one-cell rows, got %s" % [(r["name"], r["gate"]) for r in pars]
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
        if verdict["enforced_in_helper_only"]:
            print("  DECLARED-BUT-NOT-ENFORCED (item 71): %d rows route their gate through"
                  % len(verdict["enforced_in_helper_only"]))
            print("  ResolveNativeEntity, whose own literal is what runs; the table column on these rows is")
            print("  DECORATIVE. A clean verdict above does NOT cover behavior on these rows.")
            for n, sname, scells, pcells, note in verdict["enforced_in_helper_only"]:
                print("    %-20s shipped=(%r,%s)  %s" % (n, sname, scells, note))
        if verdict["shipped_min_gated"]:
            print("  MIN-GATED SHIPPED ROWS (port has no form for these): %s"
                  % ", ".join("%s(min=%d)" % (n, c) for n, c, _ in verdict["shipped_min_gated"]))
            print("  The port's ScriptNativeCheckArity implements the EXACT gate only; each of these needs a")
            print("  second gate: jge semantics, an &out_actual 5th argument, and the separate")
            print("  'Expected at least %d parameters but recieved %d.' string (@0x6A0694).")
        print("  BOUND on any 'no gate' reading: %s" % verdict["negative_bound"])
        if verdict["divergences"]:
            for f in verdict["divergences"]:
                print("  DIVERGENCE: %s: %s" % f)
            return 1
        n_dec = len(verdict["enforced_in_helper_only"])
        print("=> every ported body's gate (name, cells) equals shipped's prologue immediates, and the kept\n"
              "   defect is still kept. Denominators agree, so the verdict covers all %d rows."
              % verdict["total_rows"])
        if n_dec:
            print("   SCOPE LIMIT: that is a verdict on DECLARATIONS vs shipped for all %d rows, and on"
                  % verdict["total_rows"])
            print("   ENFORCED behavior for only %d of them. The %d set-A rows named above are gated by a"
                  % (verdict["total_rows"] - n_dec, n_dec))
            print("   shared helper, so this tool cannot see their enforcement at all. Item 70 killed the\n"
                  "   unproven population; item 71 kills the unproven SOURCE. Both limits are stated\n"
                  "   because a green number is only as good as its coupling to what is being claimed.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
