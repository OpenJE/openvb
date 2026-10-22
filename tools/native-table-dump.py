#!/usr/bin/env python3
"""tools/native-table-dump.py - dump shipped AMX native tables straight out of the PE.

Why this exists
---------------
Shipped F3 registers its script natives as three `{name-ptr, fn-ptr}` arrays that
`Script_Small::Load` (0x5a8880) hands to amx_Register (sub_5CCDE0) in this order:

    0x5a8ada  sub_5CCDE0(&amx, &off_7004C0,          -1)   float lib
    0x5a8ae7  sub_5CCDE0(&amx, &small_script_functions, -1) engine core  (@0x6FFAF8)
    0x5a8afe  sub_5CCDE0(&amx, &off_6F3750,          -1)   F3 game lib

The authoritative name -> function mapping lives ONLY in that data. No type
database lists it, so a name that aliases onto a `return 1` stub is invisible
unless the array is walked byte by byte. That aliasing is exactly what this tool
detects (OpenQuest/CloseQuest/AddQuestEntry/AddJournalEntry -> 0x420990).

Recipe (the whole point of this file)
-------------------------------------
Each table is a sequence of `{u32 name_va, u32 fn_va}` little-endian pairs that
runs until a `{0, 0}` terminator - NOT until an entry count, because shipped
records no count. `name_va` is a VA into .rdata, so it needs the same VA -> file
offset decode as the table itself.

Section-header field ORDER is VirtualSize, VirtualAddress, SizeOfRawData,
PointerToRawData (PE offsets +8, +12, +16, +20 within each 40-byte header).
Using SizeOfRawData as the span, or swapping VA/raw, silently decodes garbage:
the first attempt at this did exactly that and printed plausible-looking junk.
Therefore every run ASSERTS that known native names resolve before it prints
anything, and pins the exe identity (sha256) in its header, because a table dump
without the binary identity is not reproducible.

Usage
-----
    python3 tools/native-table-dump.py                       # default 3 tables
    python3 tools/native-table-dump.py <exe> [VA ...]         # explicit
    python3 tools/native-table-dump.py --json                 # machine readable
    python3 tools/native-table-dump.py --selftest             # gates, exit 1 on fail

Env: $OPENVB_ORIG_EXE is the default exe path.
See: discoveries/2025-10-20-scripting-abi-native-tables.md
"""

import argparse
import hashlib
import json
import os
import re
import struct
import sys
from pathlib import Path

# --- pinned identity of the reference binary (F3 demo 1.0) -------------------
PINNED_SHA256 = "4652a3d03e7e09facab0ea123568a36c9be3b44ada80c1f60dac0449b3579b87"

# --- the three tables Script_Small::Load registers, in registration order ----
DEFAULT_TABLES = {
    0x7004C0: "float lib (off_7004C0)",
    0x6FFAF8: "engine core (small_script_functions)",
    0x6F3750: "F3 game lib (off_6F3750)",
}

# --- proof that the VA decode is right, before believing ANY output -----------
# name -> fn VA. Each pairing is an independent claim: resolving the name is not
# enough, the function it points at must match too.
EXPECTED_FN = {
    "GetLevel": 0x45ABE0,
    "GetRace": 0x45AEB0,
    "Protagonist": 0x45A960,
    "OpenTask": 0x45B900,
    "CloseTask": 0x45B940,
    "AddEntry": 0x45B980,
    "LockDoor": 0x45A1B0,
    "UnlockDoor": 0x45A240,
    "GetLockDC": 0x45B570,
    "GetCurrentHP": 0x45B1B0,
    "GetMaxHP": 0x45B110,
    "float": 0x5CE5F0,
    # engine core (small_script_functions) - cross-checked against
    # discoveries/2025-10-20-scripting-abi-native-tables.md §3.2/§4 (Year 0x5C5040,
    # SecondsPassed 0x5C4F30, DialogueCall 0x5C3C30 all appear there independently).
    "Year": 0x5C5040,
    "SecondsPassed": 0x5C4F30,
    "DialogueCall": 0x5C3C30,
    "Myself": 0x5C7340,
}
MUST_RESOLVE = list(EXPECTED_FN)

# --- the dead-end alias target discovered by the first walk ------------------
QUEST_JOURNAL_STUB = 0x420990

MAX_NAME_LEN = 192          # longest shipped native name is < 20; generous cap
MAX_CODE_BYTES = 8


class BadPe(Exception):
    pass


def sha256_of(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as fh:
        for chunk in iter(lambda: fh.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


class Pe:
    """Minimal PE32 reader: VA -> file offset, bytes at VA, 8-bit peek."""

    def __init__(self, path: Path):
        self.path = path
        self.data = path.read_bytes()
        d = self.data
        if d[:2] != b"MZ":
            raise BadPe(f"{path}: missing MZ signature")
        self.lfanew = struct.unpack("<I", d[0x3C:0x40])[0]
        if not 0 < self.lfanew < len(d) - 24:
            raise BadPe(f"{path}: e_lfanew {self.lfanew:#x} outside file")
        if d[self.lfanew:self.lfanew + 4] != b"PE\0\0":
            raise BadPe(f"{path}: missing PE signature at {self.lfanew:#x}")
        coh = self.lfanew + 4
        self.number_of_sections = struct.unpack("<H", d[coh + 2:coh + 4])[0]
        self.size_of_optional_header = struct.unpack("<H", d[coh + 16:coh + 18])[0]
        if self.number_of_sections == 0 or self.number_of_sections > 96:
            raise BadPe(f"{path}: insane section count {self.number_of_sections}")
        self.opt_off = coh + 20
        magic = struct.unpack("<H", d[self.opt_off:self.opt_off + 2])[0]
        if magic == 0x10B:                    # PE32
            self.image_base = struct.unpack("<I", d[self.opt_off + 28:self.opt_off + 32])[0]
            self.is_32 = True
        elif magic == 0x20B:                  # PE32+
            self.image_base = struct.unpack("<Q", d[self.opt_off + 24:self.opt_off + 32])[0]
            self.is_32 = False
        else:
            raise BadPe(f"{path}: unknown optional-header magic {magic:#x}")
        self.sections = self._parse_sections()
        if not self.sections:
            raise BadPe(f"{path}: no loadable sections parsed")

    def _parse_sections(self):
        d, secs = self.data, []
        shdr = self.opt_off + self.size_of_optional_header
        for i in range(self.number_of_sections):
            o = shdr + i * 40
            if o + 40 > len(d):
                break
            name = d[o:o + 8].rstrip(b"\x00").decode("latin-1")
            # ORDER MATTERS: VirtualSize, VirtualAddress, SizeOfRawData, PointerToRawData
            vsize, va, rawsize, rawptr = struct.unpack("<IIII", d[o + 8:o + 24])
            chars = struct.unpack("<I", d[o + 36:o + 40])[0]
            if rawptr == 0 and rawsize == 0:
                continue                       # .bss-style: no file backing
            span = max(vsize, rawsize) if vsize else rawsize
            secs.append({"name": name, "va": va, "vsize": vsize, "rawsize": rawsize,
                         "rawptr": rawptr, "span": span, "chars": chars})
        return secs

    def off_for_va(self, va: int):
        rva = va - self.image_base
        if rva < 0:
            return None
        for s in self.sections:
            if s["va"] <= rva < s["va"] + s["span"]:
                delta = rva - s["va"]
                if delta < s["rawsize"]:
                    return s["rawptr"] + delta
                return None
        return None

    def read(self, va: int, n: int):
        o = self.off_for_va(va)
        if o is None or o + n > len(self.data):
            return None
        return self.data[o:o + n]

    def u32(self, va: int):
        b = self.read(va, 4)
        return None if b is None else struct.unpack("<I", b)[0]

    def section_of(self, va: int):
        rva = va - self.image_base
        for s in self.sections:
            if s["va"] <= rva < s["va"] + s["span"]:
                return s["name"]
        return "?"

    def cstring(self, va: int):
        """latin-1 NUL-slice with sanity gates; returns None if it is not a name."""
        buf = self.read(va, MAX_NAME_LEN + 1)
        if buf is None:
            return None
        end = buf.find(b"\x00")
        if end == 0:
            return None
        if end < 0:
            return None                       # no terminator within cap -> not a name
        raw = buf[:end]
        if not all(0x20 <= c < 0x7F for c in raw):
            return None
        return raw.decode("latin-1")


def is_code(pe: Pe, fn_va: int) -> bool:
    """Guard against a VA that is really a vftable slot rather than a function."""
    if fn_va is None:
        return False
    b = pe.read(fn_va, 1)
    if b is None:
        return False
    return (b[0] & 0xF0) in (0x50, 0x80, 0x90) or b[0] in (
        0x40, 0x48, 0x49, 0x4C, 0x4D, 0x53, 0x55, 0x56, 0x57,
        0xA1, 0xA8, 0xB8, 0xB9, 0xBA, 0xBB, 0xBE, 0xBF,
        0xC2, 0xC3, 0xC9, 0xCC, 0xE8, 0xE9, 0xEB, 0xF7,
    )


def walk_table(pe: Pe, base_va: int, hard_cap: int = 4096):
    """Walk {name,fn} pairs to the {0,0} terminator. Returns (entries, terminated)."""
    entries, terminated = [], False
    for idx in range(hard_cap):
        slot = pe.read(base_va + idx * 8, 8)
        if slot is None:
            break
        name_va, fn_va = struct.unpack("<II", slot)
        if name_va == 0 and fn_va == 0:
            terminated = True
            break
        entries.append({
            "idx": idx,
            "name_va": name_va,
            "name": pe.cstring(name_va),
            "fn_va": fn_va,
            "code": is_code(pe, fn_va),
            "bytes": (pe.read(fn_va, MAX_CODE_BYTES) or b"").hex(" "),
            "name_sec": pe.section_of(name_va),
            "fn_sec": pe.section_of(fn_va),
        })
    return entries, terminated


def alias_groups(entries):
    """fn VA -> [names] for every fn claimed by more than one name."""
    by_fn = {}
    for e in entries:
        by_fn.setdefault(e["fn_va"], []).append(e["name"] or f"#{e['idx']}")
    return {fn: names for fn, names in sorted(by_fn.items(), key=lambda kv: -len(kv[1]))
            if len(names) > 1}


def distinct_names(entries):
    return sorted({(e["name"] or "") for e in entries if e["name"]})


def gate(pe: Pe, tables):
    """Proof-of-decode. Refuse to print anything unless names that DID resolve
    pair with the shipped fn values we know independently, and unless at least
    one such (name, fn) claim is present. A mis-decoded VA (wrong section field
    order, wrong image base) resolves nothing or resolves the wrong pair, so it
    fails here instead of printing plausible-looking garbage.

    Works for any table subset: a float-lib-only dump contains no F3 names, so
    it is gated against whichever known pairs are in scope (see EXPECTED_FN).
    """
    seen = {}
    for base in tables:
        for e in tables[base]["entries"]:
            if e["name"]:
                seen[e["name"]] = e["fn_va"]
    if not seen:
        raise BadPe("GATE FAIL: no printable name of <=192 bytes resolved in any dumped "
                    "table - the VA decode is wrong (check section field order "
                    "VirtualSize,VirtualAddress,SizeOfRawData,PointerToRawData / image base).")
    hits = []
    for want, want_fn in EXPECTED_FN.items():
        if want in seen:
            if seen[want] != want_fn:
                raise BadPe(f"GATE FAIL: '{want}' -> {seen[want]:#x}, expected {want_fn:#x}. "
                            f"Name resolved but the pairing is off; do not trust this dump.")
            hits.append(want)
    if not hits:
        raise BadPe("GATE FAIL: none of the known native names are in scope, so this dump is "
                    "unverified. Known-name anchors: " + ", ".join(sorted(EXPECTED_FN)))
    return sorted(seen)


def header_lines(pe: Pe, digest: str):
    return [
        f"exe          : {pe.path}",
        f"sha256       : {digest}"
        + ("" if digest == PINNED_SHA256 else "   <<< MISMATCH vs pinned reference"),
        f"pinned sha256: {PINNED_SHA256}",
        f"file size    : {pe.path.stat().st_size} bytes",
        f"image base   : {pe.image_base:#x}",
        f"sections     : " + ", ".join(f"{s['name']}@{pe.image_base + s['va']:#x}"
                                       f"/{s['vsize']:#x}" for s in pe.sections),
    ]


def render_text(pe: Pe, digest: str, tables, notes=None):
    out = ["# native-table-dump", *header_lines(pe, digest), ""]
    if notes:
        out += [f"note: {n}" for n in notes] + [""]
    for base in tables:
        label = tables[base]["_label"]
        entries, term = tables[base]["entries"], tables[base]["terminated"]
        out += [f"## {label} @ {base:#x}",
                f"entries={len(entries)}  terminator_{{0,0}}_found={term}  "
                f"distinct_fns={len({e['fn_va'] for e in entries})}"]
        widths = [max(len(str(e['idx'])) for e in entries),
                  max(len(e['name'] or '<none>') for e in entries)] if entries else (2, 8)
        wi, wn = widths
        any_gate = any(e.get("gate") for e in entries)
        out.append(f"  {'idx':>{wi}}  {'name':<{wn}}  {'name_va':>10}  {'fn_va':>10}  "
                   f"{'cells':>5}  {'gate-name':<{wn}}  first_bytes" if any_gate else
                   f"  {'idx':>{wi}}  {'name':<{wn}}  {'name_va':>10}  {'fn_va':>10}  first_bytes")
        for e in entries:
            if any_gate:
                g = e.get("gate")
                out.append(f"  {e['idx']:>{wi}}  {(e['name'] or '<none>'):<{wn}}  "
                           f"{e['name_va']:>10x}  {e['fn_va']:>10x}  "
                           f"{(str(g['cells']) if g else '-'):>5}  "
                           f"{((g['name'] if g else '-') or '-'):<{wn}}  {e['bytes']}"
                           + ("" if e["code"] else "   [NOT a plausible code byte]")
                           + ("" if not g or g["name"] == e["name"] else
                              "   <<< gate error-name != table name"))
            else:
                out.append(f"  {e['idx']:>{wi}}  {(e['name'] or '<none>'):<{wn}}  "
                           f"{e['name_va']:>10x}  {e['fn_va']:>10x}  {e['bytes']}"
                           + ("" if e["code"] else "   [NOT a plausible code byte]"))
        groups = alias_groups(entries)
        if groups:
            out.append(f"  alias groups ({len(groups)} fn claimed by >1 name):")
            for fn, names in groups.items():
                out.append(f"    {fn:#x} <- {', '.join(names)}")
        bad = [e for e in entries if not e["code"]]
        if bad:
            out.append(f"  WARNING: {len(bad)} fn value(s) do not look like code: "
                       + ", ".join(f"{e['name']}@{e['fn_va']:#x}" for e in bad))
        out.append("")
    return "\n".join(out)


def render_json(pe: Pe, digest: str, tables):
    payload = {
        "exe": str(pe.path),
        "sha256": digest,
        "pinned_sha256": PINNED_SHA256,
        "sha256_matches_pin": digest == PINNED_SHA256,
        "image_base": pe.image_base,
        "sections": [{**s, "va_va": pe.image_base + s["va"]} for s in pe.sections],
        "tables": [
            {
                "base_va": base,
                "label": tables[base]["_label"],
                "count": len(tables[base]["entries"]),
                "terminated": tables[base]["terminated"],
                "distinct_fns": len({e["fn_va"] for e in tables[base]["entries"]}),
                "alias_groups": [
                    {"fn_va": fn, "names": names}
                    for fn, names in alias_groups(tables[base]["entries"]).items()
                ],
                "entries": [
                    {k: (f"{v:#x}" if k.endswith("_va") and isinstance(v, int) else v)
                     for k, v in e.items()}
                    for e in tables[base]["entries"]
                ],
            }
            for base in tables
        ],
    }
    return json.dumps(payload, indent=2)



# --- arity-gate claim extraction (prologue -> the shared gate at GATE_EA) -----
# Every gated native opens with the same shape:
#     push <imm32 name-ptr>   ; the name the ERROR MESSAGE will print
#     push <imm8 cells>       ; expected parameter cells
#     push <reg> push <reg>   ; params block + amx
#     call 0x59af40           ; the shared arity gate (port: ScriptNativeCheckArity)
# so the prologue itself carries two claims per native: an expected cell count AND a
# display name. The display name is NOT guaranteed to equal the table name - shipped
# lies in four places (see the audit), which a name-only comparison can never see.
GATE_EA = 0x59AF40
GATE_WINDOW = 96
MAX_CELLS = 16


def gate_claim(pe: Pe, fn_va: int):
    """Return {cells, name, name_va, at} for the first call to the arity gate."""
    b = pe.read(fn_va, GATE_WINDOW)
    if not b:
        return None
    stack, i = [], 0
    while i < len(b) - 4:
        op = b[i]
        if op == 0x68:
            stack.append((0x68, int.from_bytes(b[i + 1:i + 5], "little"))); i += 5; continue
        if op == 0x6a:
            stack.append((0x6a, b[i + 1])); i += 2; continue
        if op in (0x50, 0x51, 0x52, 0x53, 0x56, 0x57):
            stack.append((op, None)); i += 1; continue
        if op == 0xE8:
            rel = int.from_bytes(b[i + 1:i + 5], "little", signed=True)
            if fn_va + i + 5 + rel == GATE_EA:
                # cdecl: pushes are reversed, arg0 is the LAST push. Layout is
                # [name, cells, reg, reg] reading pushes forwards -> stack[-4], stack[-3].
                imm = [x for x in stack[-4:] if x[0] in (0x68, 0x6A)]
                name_va = next((v for t, v in imm if t == 0x68 and pe.cstring(v)), None)
                cells = next((v for t, v in imm if t == 0x6A and v <= MAX_CELLS), None)
                if name_va is None or cells is None:
                    return None
                return {"cells": cells, "name": pe.cstring(name_va), "name_va": name_va, "at": i}
            stack = []
            i += 5
            continue
        i += 1
    return None


def annotate_gate_claims(pe: Pe, entries):
    for e in entries:
        e["gate"] = gate_claim(pe, e["fn_va"])
    return entries


# --- port-side audit ---------------------------------------------------------
# Reusable comparison of a port's native table against what shipped registers.
# Parses the `{ "Name", &Fn, cells } // 0xEAA` rows out of a TU (READ-ONLY - the
# TU has an owner in this project and this tool never edits it), then set-compares
# against a shipped table dump: names the port claims that shipped does not,
# shipped names not yet ported, case/spelling drift, duplicate rows, EA claims in
# the port that disagree with the shipped table, and rows on either side sharing
# one implementation (port-side aliasing, the mirror of shipped's 0x420990 quad).
PORT_ROW = re.compile(r'^\s*\{\s*"([^"]+)"\s*,\s*&([A-Za-z_0-9:]+)\s*,\s*(-?\d+)\s*\}')
PORT_EA = re.compile(r'0x([0-9a-fA-F]{5,8})')


def parse_port_table(path: Path):
    rows = []
    for lineno, line in enumerate(path.read_text(errors="replace").splitlines(), 1):
        m = PORT_ROW.match(line)
        if not m:
            continue
        tail = line.split("//", 1)[1] if "//" in line else ""
        ea = PORT_EA.search(tail)
        rows.append({"line": lineno, "name": m.group(1), "fn_symbol": m.group(2),
                     "cells": int(m.group(3)),
                     "ea": int(ea.group(1), 16) if ea else None})
    return rows


def group_by(rows, key, val):
    """key value -> [val names] for keys claimed by more than one row."""
    by = {}
    for r in rows:
        by.setdefault(key(r), []).append(val(r))
    return {k: v for k, v in sorted(by.items(), key=lambda kv: -len(kv[1])) if len(v) > 1}


def audit(port_rows, shipped_entries, port_label="port", shipped_label="shipped"):
    shipped = {e["name"]: e for e in shipped_entries if e["name"]}
    port = {r["name"]: r for r in port_rows}
    low_p, low_s = {n.lower(): n for n in port}, {n.lower(): n for n in shipped}
    dup_p = group_by(port_rows, lambda r: r["name"], lambda r: f"line {r['line']}")
    dup_s = group_by(shipped_entries, lambda e: e["name"], lambda e: f"idx {e['idx']}")
    return {
        "port_count": len(port_rows), "shipped_count": len(shipped_entries),
        "exact": sorted(set(port) & set(shipped)),
        "port_only": sorted(set(port) - set(shipped)),
        "shipped_only": sorted(set(shipped) - set(port)),
        "case_drift": sorted((low_s[k], low_p[k]) for k in set(low_p) & set(low_s)
                             if low_p[k] != low_s[k]),
        "dup_port": {k: v for k, v in dup_p.items()},
        "dup_shipped": {k: v for k, v in dup_s.items()},
        "port_alias_groups": group_by(port_rows, lambda r: r["fn_symbol"], lambda r: r["name"]),
        "ea_mismatch": sorted(
            (r["name"], r["ea"], shipped[r["name"]]["fn_va"])
            for r in port_rows
            if r["name"] in shipped and r["ea"] is not None
            and (r["ea"] & 0xFFFFFFFF) != shipped[r["name"]]["fn_va"]),
        "no_gate": sorted(r["name"] for r in port_rows if r["cells"] < 0),
        "arity_mismatch": sorted(
            (r["name"], r["cells"], shipped[r["name"]]["gate"]["cells"])
            for r in port_rows if r["name"] in shipped and shipped[r["name"]]["gate"]
            and r["cells"] >= 0 and r["cells"] != shipped[r["name"]]["gate"]["cells"]),
        "gate_name_divergence": sorted(
            (e["name"], e["gate"]["name"], e["fn_va"]) for e in shipped_entries
            if e.get("gate") and e["gate"]["name"] != e["name"]),
        "shipped_ungated": sorted(e["name"] for e in shipped_entries if not e.get("gate")),
        "labels": (port_label, shipped_label),
    }


def render_audit(a, shipped_entries):
    p, s = a["labels"]
    by_name = {e["name"]: e for e in shipped_entries if e["name"]}
    out = [f"## audit: {p} vs {s}",
           f"{p} rows={a['port_count']}  {s} entries={a['shipped_count']}  "
           f"exact name matches={len(a['exact'])}  "
           f"{s} distinct fn={len({e['fn_va'] for e in shipped_entries})}", ""]
    out.append(f"### {p}-only names - registered by the port, absent in shipped: "
               f"{len(a['port_only'])}")
    out += [f"  - {i}" for i in a["port_only"]] or ["  (none)"]
    out.append("")
    out.append(f"### {s}-only names - shipped registers it, the {p} table does not: "
               f"{len(a['shipped_only'])}")
    out += [f"  - {i}  (fn {by_name[i]['fn_va']:#x}, idx {by_name[i]['idx']}, "
            f"bytes {by_name[i]['bytes']})" for i in a["shipped_only"]] or ["  (none)"]
    out.append("")
    out.append(f"### case/spelling drift: {len(a['case_drift'])}")
    out += [f"  - shipped '{sh}' vs {p} '{pt}'" for sh, pt in a["case_drift"]] or ["  (none)"]
    out.append("")
    out.append(f"### {p} EA comment disagrees with the shipped table's fn value: "
               f"{len(a['ea_mismatch'])}")
    out += [f"  - {n}: port says {px:#x}, shipped table says {sx:#x}"
            for n, px, sx in a["ea_mismatch"]] or ["  (none)"]
    out.append("")
    out.append(f"### duplicate names in the {p} table: {len(a['dup_port'])}")
    out += [f"  - {n} at {', '.join(v)}" for n, v in sorted(a["dup_port"].items())] or ["  (none)"]
    out.append(f"### duplicate names in the {s} table: {len(a['dup_shipped'])}")
    out += [f"  - {n} at {', '.join(v)}" for n, v in sorted(a["dup_shipped"].items())] or ["  (none)"]
    out.append("")
    out.append(f"### {p} names sharing one implementation symbol: "
               f"{len(a['port_alias_groups'])}")
    for sym, names in a["port_alias_groups"].items():
        fns = sorted({by_name[n]["fn_va"] for n in names if n in by_name})
        out.append(f"  - {sym} <- {', '.join(sorted(names))}"
                   + (f"  [shipped fns: {', '.join(f'{f:#x}' for f in fns)}]" if fns else ""))
    if not a["port_alias_groups"]:
        out.append("  (none)")
    out.append("")
    out.append(f"### {p} rows with cells < 0 (arity claim is 'body has no gate'): "
               f"{len(a['no_gate'])}")
    out += [f"  - {n}" for n in a["no_gate"]] or ["  (none)"]
    out.append("")
    out.append(f"### shipped arity-gate claims vs {p} cells column: "
               f"{len(a['arity_mismatch'])} mismatches")
    out += [f"  - {n}: {p} says {pc} cells, shipped gate checks {sc}"
            for n, pc, sc in a["arity_mismatch"]] or ["  (none)"]
    out.append("")
    out.append(f"### shipped natives whose arity-ERROR NAME differs from their table name: "
               f"{len(a['gate_name_divergence'])}")
    out += [f"  - table '{tn}' -> gate prints '{gn}' (fn {fx:#x})"
            for tn, gn, fx in a["gate_name_divergence"]] or ["  (none)"]
    out.append(f"### shipped natives with NO arity gate at all: {len(a['shipped_ungated'])}")
    out += [f"  - {n}" for n in a["shipped_ungated"]] or ["  (none)"]
    out.append("")
    return "\n".join(out)


def load(exe: Path, want_vas, quiet=False):
    if not exe.exists():
        raise BadPe(f"exe not found: {exe}")
    pe = Pe(exe)
    digest = sha256_of(pe.path)
    if digest != PINNED_SHA256 and not quiet:
        print(f"WARNING: {exe} sha256 {digest} != pinned {PINNED_SHA256}", file=sys.stderr)
    tables = {}
    for va in want_vas:
        entries, term = walk_table(pe, va)
        annotate_gate_claims(pe, entries)
        tables[va] = {
            "_label": DEFAULT_TABLES.get(va, f"table @ {va:#x}"),
            "entries": entries,
            "terminated": term,
        }
        if not term and not quiet:
            print(f"WARNING: table {va:#x} hit no {{0,0}} terminator within cap", file=sys.stderr)
    names = gate(pe, tables)
    return pe, digest, tables, names


def selftest(exe: Path):
    """Gates on known claims. Any failure raises -> caller exits 1."""
    pe, digest, tables, names = load(exe, sorted(DEFAULT_TABLES))
    assert digest == PINNED_SHA256, f"sha256 {digest} != pinned {PINNED_SHA256}"
    expect = {0x7004C0: 9, 0x6FFAF8: 155, 0x6F3750: 45}
    for base, want in expect.items():
        got = len(tables[base]["entries"])
        assert got == want, f"table {base:#x}: {got} entries, discovery note claims {want}"
        assert tables[base]["terminated"], f"table {base:#x}: no {{0,0}} terminator"
    f3 = tables[0x6F3750]["entries"]
    stub = alias_groups(f3).get(QUEST_JOURNAL_STUB, [])
    assert sorted(stub) == ["AddJournalEntry", "AddQuestEntry", "CloseQuest", "OpenQuest"], \
        f"quest/journal alias set changed: {stub}"
    assert all(c["code"] for c in f3), "F3 lib has a fn value that is not plausible code"
    assert not any(not c["code"] for b in tables for c in tables[b]["entries"]), \
        "a dumped fn is not plausible code (is that VA a vftable?)"
    for want in MUST_RESOLVE:
        assert any(want in n for n in names), f"gate name {want} missing"
    gates = {e["name"]: e["gate"] for e in f3 if e["gate"]}
    for nm, want in (("GetLevel", 1), ("Protagonist", 0), ("SetHP", 2), ("OpenTask", 2),
                     ("IsItemEquipped", 2), ("GetPrimaryAttribute", 2), ("LockDoor", 1)):
        assert gates[nm]["cells"] == want, f"{nm}: gate cells {gates[nm]['cells']} != {want}"
    assert gates["A_ReloadWeapon"]["name"] == "ReloadWeapon", \
        f"reload error-name: {gates['A_ReloadWeapon']['name']}"
    assert sorted(e["name"] for e in f3 if not e["gate"]) == [
        "AddJournalEntry", "AddQuestEntry", "CloseQuest", "OpenQuest"], "ungated set changed"
    # float lib must be exactly the float.inc family
    fl = distinct_names(tables[0x7004C0]["entries"])
    assert set(fl) == {"float", "floatstr", "floatmul", "floatdiv", "floatadd",
                       "floatsub", "floatfract", "floatround", "floatcmp"}, f"float lib: {fl}"
    print("selftest PASS")
    print(f"  exe sha256 pinned: {digest}")
    for base in sorted(DEFAULT_TABLES):
        t = tables[base]
        print(f"  {base:#x} {t['_label']}: {len(t['entries'])} entries, "
              f"{len({e['fn_va'] for e in t['entries']})} distinct fn, "
              f"terminator={t['terminated']}")
    print(f"  F3 quest/journal stub {QUEST_JOURNAL_STUB:#x} <- {', '.join(sorted(stub))}")
    print(f"  gate names resolved: {len(names)}")
    return 0


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.splitlines()[0],
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("exe", nargs="?", default=os.environ.get("OPENVB_ORIG_EXE"),
                    help="path to shipped F3.exe (default $OPENVB_ORIG_EXE)")
    ap.add_argument("vas", nargs="*", help="table VAs to dump (default the 3 registered libs)")
    ap.add_argument("--json", action="store_true", help="machine-readable output")
    ap.add_argument("--selftest", action="store_true", help="run the gates and exit")
    ap.add_argument("--names", action="store_true", help="print only the resolved name list")
    ap.add_argument("--audit", metavar="PORT.cpp", default=None,
                    help="set-compare a port {name,&fn,cells} table against the dumped "
                         "table (read-only; prints an audit section after the dump)")
    args = ap.parse_args(argv)

    if not args.exe:
        ap.error("no exe given and $OPENVB_ORIG_EXE is unset")
    exe = Path(args.exe)

    try:
        if args.selftest:
            return selftest(exe)
        want = [int(v, 16) if isinstance(v, str) else v for v in args.vas] or sorted(DEFAULT_TABLES)
        pe, digest, tables, names = load(exe, want)
    except BadPe as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        return 2

    if args.names:
        print("\n".join(names))
        return 0
    if args.json:
        print(render_json(pe, digest, tables))
    else:
        print(render_text(pe, digest, tables))
    if args.audit:
        port_rows = parse_port_table(Path(args.audit))
        base = 0x6F3750 if 0x6F3750 in tables else max(tables)
        if not port_rows:
            print(f"AUDIT ABORTED: no {{\"Name\", &Fn, cells}} rows parsed from "
                  f"{args.audit} - check the table shape.", file=sys.stderr)
            return 3
        print(render_audit(audit(port_rows, tables[base]["entries"],
                                 f"port ({Path(args.audit).name})",
                                 DEFAULT_TABLES.get(base, f"shipped {base:#x}")),
                           tables[base]["entries"]))
    return 0


if __name__ == "__main__":
    sys.exit(main())
