#!/usr/bin/env python3
"""T-054: offline ecx-flow owner-attribution analysis for shipped F3.exe.

WHAT THIS IS
  A static `this`(ecx)-propagation engine built ONLY from the raw PE bytes
  ($OPENVB_ORIG_EXE, disassembled with binutils objdump) + the checked-in
  function/EA dump artifacts/F3.json (OOAnalyzer export). It never touches the
  live IDA MCP sessions and writes NOTHING to any IDB. The tool proposes flow
  evidence; a writer wave applies it under ledger (T-033 discipline).

JOINS IMPLEMENTED (the four classes the batch chain lacked):
  A. singleton-this joins  -- `mov $G,%ecx` / `lea G,%ecx` (this == &global) and
                              `mov G,%ecx` (this == *global, pointer slot)
                              observed at a call edge into the queried band, plus
                              embed edges this == &global+disp / in-ecx+disp.
                              (b5 template: Gfx_D3D <- &dword_7082F8.)
  B. thunk / tail-call edges -- single-`jmp` forwarders (incl. the virtual-base
                              `sub [ecx-4],ecx ; jmp T` adjusted thiscall thunks)
                              and tail `jmp`s; callers of a thunk count as callers
                              of the resolved target.
  C. vftable-region scan   -- raw runs of code pointers in .rdata/.data/.data1
                              (wider than the b5 0x640000-0x680000 window): slots
                              into the band or into thunks resolving into it, the
                              INSTALL sites (imm table-VA stored to [obj]), and
                              `call *slot(vft)` dispatch sites constrained by the
                              slot offset, with the object-expr at each site.
  D. allocator/ctor callers -- `new(size)` -> ecx -> ctor edge; MSVC ctors return
                              `this` in eax, so a following `[base+disp] <- %eax`
                              store is a HOST INSTALL (b6 Gfx_AnimMeshInstance
                              <- [VFX_Model+0x2C4] shape). Weak tier: any
                              [this+disp] store within 0x60B of a heap-ctor edge.

BOUNDARIES (read before quoting any negative):
  * Owner attribution uses a self-derived function-start set (call/jmp targets,
    F3.json EAs, aligned starts after ret/jmp, table entries, thunks). It
    approximates IDA's enumeration; a site maps to the nearest known start at or
    below it, so an unnamed tail region can be attributed to the previous head.
  * F3.json is the 2024 export: its names are PRE-rename (b2..b6 IDB names are
    not reflected). Treat F3.json names as locators, not as current names.
  * Register state model: cleared at function starts and at unconditional-jmp
    targets; eax/ecx/edx cleared across calls (caller-saved), ebx/esi/edi kept
    (MSVC callee-saved); ebp/esp treated as opaque. Values stored to memory and
    reloaded lose identity unless re-derivable; loops may carry one stale value
    per register (flow-insensitive approximation).
  * ANY EMPTY result is only evidence after `selftest` is GREEN in the same
    code build (harness item 35). Band mode refuses verdicts on a red gate
    unless --allow-red is passed.
  * Item 43 (locator rule): pad TARGETS are usually chunks; the head is the
    containing function. head_hint() grades call-head vs pad-chunk with a
    containment candidate, but the LIVE IDB read-back is the sole headness
    authority -- buckets are flow facts, headness is a locator fact.

Usage:
  tools/ecx-flow.py selftest
  tools/ecx-flow.py band --cls cls_0x445350
  tools/ecx-flow.py band --addrs 0x507810,0x507580,0x5067c0 --label epoch
  tools/ecx-flow.py families --cls cls_0x50dbb0  # per-entry LIVE/DELEG/PAD/ISO census
  tools/ecx-flow.py cases                        # the three T-054 registered cases
  tools/ecx-flow.py share-check --addrs 0x434250,0x434120  # item-44 rule-1 pre-queue
                                                             # gate; exit 1 = SHARING/BLOCKED
  tools/ecx-flow.py global 0x707e48
Exit codes: 0 ok, 1 self-test red / data missing / share-check BLOCKED, 2 inputs missing.
"""

import argparse
import collections
import hashlib
import json
import os
import re
import subprocess
import sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DEFAULT_EXE = os.environ.get("OPENVB_ORIG_EXE", "")
DEFAULT_F3JSON = os.path.join(REPO, "artifacts", "F3.json")
CACHE_DIR = os.path.join(REPO, "obj", "t054")

OBJDUMPS = ["i686-w64-mingw32-objdump", "objdump"]

# operator new(size)/new[](size) wrappers; verified 0x6054b0 loads size from
# [esp+8], mallocs, returns the pointer in eax; 0x605554 is the array twin.
ALLOCATORS = {0x6054B0: "new", 0x605554: "new[]"}

REGS = ("eax", "ebx", "ecx", "edx", "esi", "edi", "ebp", "esp")
CALLER_SAVED = ("eax", "ecx", "edx")
SUBREG = {
    "al": "eax", "ah": "eax", "ax": "eax",
    "bl": "ebx", "bh": "ebx", "bx": "ebx",
    "cl": "ecx", "ch": "ecx", "cx": "ecx",
    "dl": "edx", "dh": "edx", "dx": "edx",
    "si": "esi", "di": "edi", "bp": "ebp", "sp": "esp",
}


def find_objdump(exe):
    for c in OBJDUMPS:
        try:
            out = subprocess.run([c, "-h", exe], capture_output=True, text=True, timeout=60)
            if out.returncode == 0 and ".text" in out.stdout:
                return c
        except (OSError, subprocess.TimeoutExpired):
            continue
    return None


LINE_RE = re.compile(r"^\s+([0-9a-f]+):\t((?:[0-9a-f]{2} )+)\t?(.*)$")
MEM_RE = re.compile(r"^(?:(-?0x[0-9a-f]+|-?\d+))?\((%[a-z]+)(?:,(%[a-z]+),(\d+))?\)$")
ABS_RE = re.compile(r"^0x[0-9a-f]+$")
BR_RE = re.compile(r"^0x([0-9a-f]+)$")


def norm_reg(tok):
    if not tok or not tok.startswith("%"):
        return None
    r = tok[1:].lower()
    if r in REGS:
        return r
    return SUBREG.get(r)


class Insn(object):
    __slots__ = ("addr", "mn", "ops", "nbytes")

    def __init__(self, addr, mn, ops, nbytes):
        self.addr = addr
        self.mn = mn
        self.ops = ops
        self.nbytes = nbytes

    @property
    def dst(self):
        return self.ops[-1] if self.ops else ""


# ---------------------------------------------------------------------------
# expressions: ('addr',v) ('contents',v) ('plus',base,d) ('deref',base,d)
#               ('heap',size) ('ctorret',ctor,size,site) ('inecx',)
# ---------------------------------------------------------------------------


def flatten(e):
    while True:
        if not e:
            return None
        if e[0] == "plus":
            b = flatten(e[1])
            if b and b[0] == "addr":
                e = ("addr", (b[1] + e[2]) & 0xFFFFFFFF)
                continue
            if b and b[0] == "plus":
                e = ("plus", b[1], b[2] + e[2])
                continue
            if b:
                return ("plus", b, e[2])
            return ("plus", e[1], e[2])
        if e[0] == "deref":
            b = flatten(e[1])
            if b and b[0] == "addr":
                return ("contents", (b[1] + e[2]) & 0xFFFFFFFF)
            return ("deref", b if b else e[1], e[2])
        return e


def efmt(e):
    if e is None:
        return "?"
    k = e[0]
    if k == "addr":
        return "&0x%X" % e[1]
    if k == "contents":
        return "*[0x%X]" % e[1]
    if k == "plus":
        sign = "-" if e[2] < 0 else "+"
        return "%s%s0x%X" % (efmt(e[1]), sign, abs(e[2]))
    if k == "deref":
        sign = "-" if e[2] < 0 else "+"
        return "*[%s%s0x%X]" % (efmt(e[1]), sign, abs(e[2]))
    if k == "heap":
        return "new(0x%X)" % e[1]
    if k == "ctorret":
        return "ctor(0x%X)->this" % e[1]
    if k == "inecx":
        return "in-ecx"
    if k == "stk":
        return "stk(%s)" % (e[1],)
    return str(e)


def ser(e):
    if e is None:
        return None
    out = list(e)
    if e[0] in ("plus", "deref") and isinstance(e[1], tuple):
        out[1] = ser(e[1])
    return out


def deser(v):
    if v is None:
        return None
    v = tuple(v)
    if v[0] in ("plus", "deref"):
        return (v[0], deser(v[1]), v[2])
    return v


# ---------------------------------------------------------------------------


class Model(object):
    def __init__(self, exe, f3json_path, quiet=False):
        self.exe = exe
        self.quiet = quiet
        with open(exe, "rb") as f:
            self.blob = f.read()
        self.exe_md5 = hashlib.md5(self.blob).hexdigest()
        with open(f3json_path) as f:
            self.f3 = json.load(f)
        want = self.f3.get("filemd5", "").lower()
        if want and want != self.exe_md5:
            sys.stderr.write(
                "WARNING: EXE md5 %s != F3.json filemd5 %s: dump and image disagree;"
                " re-export the dump before trusting any conclusion.\n" % (self.exe_md5, want))
        self.secs = []
        self.text_sec = None
        self.insns = []
        self.by_addr = {}
        self.starts = set()          # primary heads (call/F3/aligned-after-terminator)
        self.starts_sorted = []
        self.all_heads = set()       # primary + branch targets (for tables/thunks)
        self.jmp_targets = set()
        self.jcc_targets = set()
        self.call_targets = collections.Counter()
        self.ea2cls = {}
        self.tables = {}
        self.f3_tables = {}
        self.slotmap = collections.defaultdict(list)
        self.thunks = {}
        self.thunks_done = False
        self.adjusted_thunks = set()
        self.edges = []        # dicts: site,kind,target,raw_target,ecx,owner
        self.indirect = []     # dicts: site,slot,obj,obj_disp,ecx,owner
        self.stores = []       # dicts: site,base,disp,val,owner
        self.pad_installs = {}  # pad head VA -> [push sites]
        self.pads = {}
        self.installs = collections.defaultdict(list)  # table VA -> (site, obj-expr)
        self.global_writes = collections.defaultdict(list)

    # ---------------- parse ----------------

    def build(self, cache=True):
        cpath = os.path.join(CACHE_DIR, "flow-%s.json" % self.exe_md5)
        self.od = find_objdump(self.exe)
        if not self.od:
            sys.stderr.write("ERROR: no working PE-i386 objdump found\n")
            sys.exit(2)
        self.secs = self._sections()
        self.text_sec = next(s for s in self.secs if s[0] == ".text")
        if cache and os.path.exists(cpath):
            self._load_cache(cpath)
            return
        out = subprocess.run([self.od, "-d", self.exe], capture_output=True, text=True,
                             check=True).stdout
        self._parse_dump(out)
        self._static_index()
        self._scan_tables()
        self._flow_pass()
        if cache:
            os.makedirs(CACHE_DIR, exist_ok=True)
            self._save_cache(cpath)

    def _sections(self):
        out = subprocess.run([self.od, "-h", self.exe], capture_output=True,
                             text=True, check=True).stdout
        secs = []
        for line in out.splitlines():
            m = re.match(r"\s*\d+\s+(\S+)\s+([0-9a-f]+)\s+([0-9a-f]+)\s+[0-9a-f]+\s+([0-9a-f]+)",
                         line)
            if m:
                name, size, vma, fo = m.groups()
                secs.append((name, int(vma, 16), int(size, 16), int(fo, 16)))
        return secs

    def _parse_dump(self, text):
        pending = None  # (nbytes_needed,) continuation handling
        for line in text.splitlines():
            m = LINE_RE.match(line)
            if not m:
                continue
            a, bs, rest = m.groups()
            a = int(a, 16)
            nbytes = len(bs.split())
            rest = rest.strip()
            if not rest:
                # byte continuation of previous insn
                if self.insns and a == self.insns[-1].addr + self.insns[-1].nbytes:
                    self.insns[-1].nbytes += nbytes
                continue
            parts = rest.split(None, 1)
            mn = parts[0].lower()
            ops_s = parts[1] if len(parts) > 1 else ""
            if mn == ".byte":
                ins = Insn(a, ".byte", [], nbytes)
            else:
                ins = Insn(a, mn, self._operands(ops_s), nbytes)
            self.insns.append(ins)
        self.by_addr = {i.addr: i for i in self.insns}

    OP_SPLIT = re.compile(r",(?![^()]*\))")

    def _operands(self, s):
        if not s:
            return []
        # strip operand-size suffix tokens like "data16 " / "ptr "
        s = re.sub(r"\b(ptr|data16|data32)\b\s*", "", s)
        return [o.strip() for o in self.OP_SPLIT.split(s) if o.strip()]

    # ---------------- index ----------------

    def _static_index(self):
        for i in self.insns:
            if i.mn == ".byte":
                continue
            if i.mn.startswith("call"):
                t = self._branch_target(i)
                if t is not None:
                    self.call_targets[t] += 1
            elif i.mn.startswith("j"):
                t = self._branch_target(i)
                if t is None:
                    continue
                if i.mn == "jmp":
                    self.jmp_targets.add(t)
                else:
                    self.jcc_targets.add(t)
        for struct in self.f3["structures"].values():
            for m in struct.get("methods", {}).values():
                ea = int(m["ea"], 16)
                self.ea2cls[ea] = (struct["name"], m["name"])
                self.starts.add(ea)
        term_end, gap_pad = None, False
        for i in self.insns:
            boundary = (i.mn.startswith("ret") or i.mn.startswith("j")
                        or i.mn == "int3")
            pad = i.mn in ("nop", ".byte", "int3")
            if (i.addr % 16 == 0 and not boundary and not pad and term_end is not None
                    and gap_pad and 0 < i.addr - term_end <= 0x30):
                self.starts.add(i.addr)
            if boundary:
                term_end = i.addr + i.nbytes
                gap_pad = True
            elif not pad:
                gap_pad = False
        self.starts |= set(self.call_targets)
        self.starts = {s for s in self.starts if s in self.by_addr}
        self._collect_data_code_ptrs()
        self._collect_pads()
        self.starts_sorted = sorted(self.starts)
        self._collect_jump_tables()
        self.jmp_targets |= set(self.itab_targets)
        self.all_heads = (self.starts | self.jmp_targets | self.jcc_targets) & set(self.by_addr)

    def _branch_target(self, ins):
        if not ins.ops:
            return None
        op = ins.ops[0]
        if op.startswith("*"):
            return None
        m = BR_RE.match(op)
        return int(m.group(1), 16) if m else None

    ITAB_RE = re.compile(r"^\*(0x[0-9a-f]+)\(,%[a-z]+,(\d+)\)$")

    def _collect_data_code_ptrs(self):
        """All 4-aligned dwords in .rdata/.data/.data1 that point into .text.
        Map value -> [referring VAs] (table-referenced code addresses: vfts,
        atexit pads, EH handler arrays, jump tables in data)."""
        lo, hi = self.text_sec[1], self.text_sec[1] + self.text_sec[2]
        refs = collections.defaultdict(list)
        for name, va, size, fo in self.secs:
            if name == ".text":
                continue
            buf = self.blob[fo:fo + size]
            n = len(buf) - (len(buf) % 4)
            for i in range(0, n, 4):
                v = int.from_bytes(buf[i:i + 4], "little")
                if lo <= v < hi:
                    refs[v].append(va + i)
        self.data_code_refs = dict(refs)
        self.data_code_ptrs = set(refs)

    def _collect_pads(self):
        """MSVC SEH/vector-iterator pad thunks:
          `mov|lea disp(%ebp),%ecx [; add $d,%ecx] ; jmp|call T`
        AND the pad's own VA is table-referenced in .rdata/.data (EH handler /
        atexit arrays). Registered as heads so tail/cleanup flow is attributed;
        `jmp` pads also get thunk resolution."""
        self.pads = {}
        refs = getattr(self, "data_code_ptrs", set())
        n = len(self.insns)
        for i in range(n):
            ins = self.insns[i]
            if ins.mn not in ("mov", "lea") or len(ins.ops) != 2:
                continue
            if norm_reg(ins.ops[-1]) != "ecx" or not ins.ops[0].endswith("(%ebp)"):
                continue
            j = i + 1
            adj = False
            d = 0
            if j < n and self.insns[j].mn == "add" and len(self.insns[j].ops) == 2 \
                    and norm_reg(self.insns[j].ops[-1]) == "ecx" \
                    and self.insns[j].ops[0].startswith("$"):
                adj = True
                d = int(self.insns[j].ops[0][1:], 16)
                j += 1
            if j >= n or self.insns[j].mn not in ("jmp", "call"):
                continue
            t = self._branch_target(self.insns[j])
            if t is None or ins.addr not in refs:
                continue
            self.pads[ins.addr] = (t, adj, self.insns[j].mn, d)
            self.starts.add(ins.addr)
            if not self.thunks_done and self.insns[j].mn == "jmp":
                self.thunks[ins.addr] = t
                if adj:
                    self.adjusted_thunks.add(ins.addr)

    def _collect_jump_tables(self):
        """Absolute jump tables: `jmp *0xTAB(,%ecx,4)` -- resurrect their cases
        as (indirect) branch targets so dead-region waking can re-enter them."""
        self.itab_targets = set()
        self.itab_by_site = {}
        lo, hi = self.text_sec[1], self.text_sec[1] + self.text_sec[2]
        for i in self.insns:
            if i.mn != "jmp" or not i.ops:
                continue
            m = self.ITAB_RE.match(i.ops[0])
            if not m:
                continue
            tab = int(m.group(1), 16)
            ts = []
            for k in range(512):
                v = self.rd(tab + k * 4)
                if v is None or not (lo <= v < hi):
                    break
                ts.append(v)
            if ts:
                self.itab_by_site[i.addr] = ts
                self.itab_targets.update(ts)

    def _clear_targets(self):
        """Register-state clearing at intra-function merge points, refined by the
        write-set of the path NOT taken:
          * backward jmp to t (loop header): drop caller-saved + anything written
            in [t, src];
          * forward jmp src->t: drop anything written in [src, t] (fallthrough
            path may have executed them); keep the rest;
          * jcc target t (forward): drop anything written in [src, t];
          * cross-function targets: full clear (handled via starts).
        """
        clear = {}
        idx_at = {}
        end_prev = {}
        for n, i in enumerate(self.insns):
            idx_at[i.addr] = n
            end_prev[i.addr + i.nbytes] = i

        def writes_in(lo, hi):
            rs = set()
            a = idx_at.get(lo)
            b = idx_at.get(hi)
            if a is None or b is None:
                return None
            if b < a:
                a, b = b, a
            for i in self.insns[a:b + 1]:
                if not i.ops:
                    continue
                if i.mn in ("cmp", "test", "push") or i.mn.startswith("j"):
                    if i.mn != "push":
                        continue
                if i.mn == "call":
                    rs.update(CALLER_SAVED)
                    continue
                if i.mn in ("leave", "ret", "int3"):
                    rs.update(REGS)
                    continue
                dst = i.ops[-1]
                r = norm_reg(dst) if dst.startswith("%") and ":" not in dst else None
                if r:
                    rs.add(r)
                if i.mn in ("pop", "xchg"):
                    if i.mn == "pop":
                        rs.add("esp")
                    else:
                        r2 = norm_reg(i.ops[0]) if i.ops[0].startswith("%") else None
                        if r2:
                            rs.add(r2)
            return rs

        def add(t, regs):
            cur = clear.get(t, set())
            if regs is None:
                cur |= set(REGS)
            else:
                cur |= regs
            clear[t] = cur

        for i in self.insns:
            if not i.mn.startswith("j"):
                continue
            t = self._branch_target(i)
            if t is None or t in self.starts:
                continue
            if self.owner_of(t) != self.owner_of(i.addr):
                clear[t] = set(REGS)
                continue
            # single-predecessor taken-edge target (hot/cold split): if nothing
            # falls through into t, arriving state is exactly the taken path's
            if not self._has_fallthrough(t, end_prev):
                continue
            if i.mn == "jmp":
                if t < i.addr:
                    add(t, writes_in(t, i.addr))
                else:
                    add(t, writes_in(i.addr, t))
            else:
                add(t, writes_in(i.addr, t))
        return clear

    def _has_fallthrough(self, t, end_prev):
        """Does control reach t by executing straight through its predecessor?
        Walk back over padding; a jmp/ret/terminator or section start means no."""
        cur = t
        while True:
            prev = end_prev.get(cur)
            if prev is None:
                return False
            if prev.mn in ("nop", ".byte", "int3"):
                cur = prev.addr
                continue
            if prev.mn == "jmp" or prev.mn.startswith("ret"):
                return False
            return True

    def owner_of(self, addr):
        s = self.starts_sorted
        if not s or addr < s[0]:
            return None
        lo, hi = 0, len(s) - 1
        while lo < hi:
            mid = (lo + hi + 1) // 2
            if s[mid] <= addr:
                lo = mid
            else:
                hi = mid - 1
        return s[lo]

    def owner_name(self, start):
        if start in self.ea2cls:
            c, n = self.ea2cls[start]
            return "%s::%s" % (c, n)
        return "sub_%X" % start

    def va2off(self, va):
        for name, sva, size, fo in self.secs:
            if sva <= va < sva + size:
                return fo + (va - sva)
        return None

    def rd(self, va, n=4):
        off = self.va2off(va)
        if off is None or off + n > len(self.blob):
            return None
        return int.from_bytes(self.blob[off:off + n], "little")

    # ---------------- tables ----------------

    def _looks_like_entry(self, v, lo, hi):
        if v in self.starts:
            return True
        ins = self.by_addr.get(v)
        return ins is not None and v % 16 == 0 and ins.mn in (
            "push", "sub", "mov", "jmp", "dec", "inc", "xor", "add")

    def _scan_tables(self):
        lo, hi = self.text_sec[1], self.text_sec[1] + self.text_sec[2]
        for name, va, size, fo in self.secs:
            if name not in (".rdata", ".data", ".data1"):
                continue
            buf = self.blob[fo:fo + size]
            n = len(buf) - (len(buf) % 4)
            i = 0
            while i + 4 <= n:
                run = []
                j = i
                while j + 4 <= n:
                    v = int.from_bytes(buf[j:j + 4], "little")
                    if not (lo <= v < hi and v % 4 == 0) or not self._looks_like_entry(v, lo, hi):
                        break
                    run.append(v)
                    j += 4
                if len(run) >= 2:
                    tva = va + i
                    self.tables[tva] = run
                    self.all_heads |= {v for v in run if v in self.by_addr}
                    for idx, tgt in enumerate(run):
                        self.slotmap[tgt].append((tva, idx))
                    i = j
                else:
                    i += 4
        for struct in self.f3["structures"].values():
            for tv in struct.get("vftables", {}).values():
                self.f3_tables[int(tv["ea"], 16)] = struct["name"]

    # ---------------- thunks ----------------

    def _resolve_thunks(self):
        if self.thunks_done:
            return
        self.thunks_done = True
        for s in sorted(self.all_heads):
            ins = self.by_addr.get(s)
            if ins is None:
                continue
            chain, cur, adj = [], s, False
            while True:
                i = self.by_addr.get(cur)
                if i is None:
                    break
                if i.mn == "jmp" and len(i.ops) == 1:
                    nxt = self._branch_target(i)
                    if nxt is None or nxt in chain or nxt == cur:
                        break
                    chain.append(cur)
                    cur = nxt
                    continue
                if not chain and not adj and i.mn in ("sub", "add") and len(i.ops) == 2 \
                        and norm_reg(i.ops[-1]) == "ecx":
                    adj = True
                    chain.append(s)
                    cur = cur + i.nbytes
                    continue
                break
            if chain and cur != s:
                for t in chain:
                    self.thunks[t] = cur
                if adj:
                    self.adjusted_thunks.add(s)

    def resolve_to(self, ea):
        self._resolve_thunks()
        seen = set()
        while ea in self.thunks and ea not in seen:
            seen.add(ea)
            ea = self.thunks[ea]
        return ea

    # ---------------- flow pass ----------------

    def _expr(self, op, state, addr_mode=False):
        """Static expr of a source operand; addr_mode for lea (address, not value)."""
        if not op:
            return None
        if op.startswith("%"):
            if ":" in op:  # segment override %fs:0x0 -- opaque
                return None
            r = norm_reg(op)
            return state.get(r) if r else None
        if op.startswith("$"):
            try:
                return ("addr", int(op[1:], 16))
            except ValueError:
                return None
        if ABS_RE.match(op):
            v = int(op, 16)
            return ("addr", v) if addr_mode else ("contents", v)
        m = MEM_RE.match(op)
        if m:
            d = self._disp_int(m.group(1))
            base, idx = m.group(2), m.group(3)
            if idx:
                return None
            br = norm_reg(base)
            be = state.get(br)
            if addr_mode:
                if be is None:
                    return None
                return flatten(("plus", be, d))
            if be is None:
                return ("stk", d) if br in ("esp", "ebp") else ("deref", None, d)
            if br in ("esp", "ebp"):
                # stack-frame slot: identity only when the same model wrote it
                return ("stk", d)
            return flatten(("deref", be, d))
        return None

    def _flow_pass(self):
        clear_at = self._clear_targets()
        targets_all = self.jmp_targets | self.jcc_targets
        state = {}
        pending = {}      # branch target -> snapshot of the taken edge's state
        last_push = None
        dead = False      # fallthrough is unreachable (after ret/jmp)
        for ins in self.insns:
            a = ins.addr
            mn = ins.mn
            ops = ins.ops
            if a in self.starts:
                state = {"ecx": ("inecx",)}
                last_push = None
                dead = False
            elif a in targets_all and dead:
                state = dict(pending.pop(a, {}))
                dead = False
            elif a in clear_at and not dead:
                for r in clear_at[a]:
                    state.pop(r, None)
            if dead and a not in self.starts and a not in targets_all:
                continue
            if mn in ("nop", ".byte", "int3", "cld", "std"):
                continue
            if mn in ("leave", "ret", "retf"):
                state = {}
                dead = True
                continue
            if mn == "jmp":
                t = self._branch_target(ins)
                ts = [t] if t is not None else self.itab_by_site.get(a, [])
                for x in ts:
                    pending[x] = dict(state)
                self._resolve_thunks()
                if (t is not None and t in self.starts and a not in self.thunks
                        and self.owner_of(a) != t):
                    self.edges.append({"site": a, "kind": "tail-jmp",
                                       "target": self.resolve_to(t), "raw_target": t,
                                       "ecx": ser(flatten(state.get("ecx"))),
                                       "owner": self.owner_of(a)})
                state = {}
                dead = True
                continue
            if mn.startswith("j"):
                t = self._branch_target(ins)
                if t is not None:
                    pending[t] = dict(state)
                continue
            if mn == "push":
                op0 = ops[0] if ops else ""
                if op0.startswith("$"):
                    try:
                        last_push = int(op0[1:], 16)
                    except ValueError:
                        last_push = None
                    if last_push in getattr(self, "pads", {}):
                        self.pad_installs.setdefault(last_push, []).append(a)
                continue
            if mn == "pop":
                r = norm_reg(ins.dst)
                if r:
                    state[r] = ("stk", "pop")
                continue
            if mn.startswith("call"):
                self._handle_call(ins, state, last_push)
                last_push = None
                continue
            self._handle_write(ins, state)
        self.pending_final = pending

    def _handle_call(self, ins, state, last_push):
        a = ins.addr
        ops = ins.ops
        dst_owner = self.owner_of(a)
        ecx_e = flatten(state.get("ecx"))
        if ops and ops[0].startswith("*"):
            op = ops[0][1:]
            m = MEM_RE.match(op)
            r = norm_reg(op)
            if m:
                d = self._disp_int(m.group(1))
                be = state.get(norm_reg(m.group(2))) if m.group(2) else None
                if be and be[0] == "deref" and be[1] is not None:
                    self.indirect.append({"site": a, "slot": d, "obj": ser(flatten(be[1])),
                                          "obj_disp": be[2], "ecx": ser(ecx_e),
                                          "owner": dst_owner})
            elif r:
                v = state.get(r)
                if v and v[0] == "addr":
                    t = self.resolve_to(v[1])
                    self.edges.append({"site": a, "kind": "call-knownreg", "target": t,
                                       "ecx": ser(ecx_e), "owner": dst_owner})
            for rr in CALLER_SAVED:
                state.pop(rr, None)
            state["eax"] = ("stk", "callret")
            return
        t = self._branch_target(ins)
        if t is None:
            for rr in CALLER_SAVED:
                state.pop(rr, None)
            return
        if t in ALLOCATORS:
            self.edges.append({"site": a, "kind": "alloc:" + ALLOCATORS[t], "target": t,
                               "ecx": ser(ecx_e), "owner": dst_owner,
                               "size": last_push or 0})
            for rr in CALLER_SAVED:
                state.pop(rr, None)
            state["eax"] = ("heap", last_push or 0)
            return
        resolved = self.resolve_to(t)
        kind = "call"
        if t != resolved:
            kind = "call-via-thunk@0x%X" % t
        if ecx_e and ecx_e[0] == "heap":
            self.ctor_pending = (resolved, ecx_e[1], a)
        else:
            self.ctor_pending = None
        self.edges.append({"site": a, "kind": kind, "target": resolved, "raw_target": t,
                           "ecx": ser(ecx_e), "owner": dst_owner})
        for rr in CALLER_SAVED:
            state.pop(rr, None)
        if self.ctor_pending:
            cp = self.ctor_pending
            state["eax"] = ("ctorret", cp[0], cp[1])
        else:
            state["eax"] = ("stk", "callret")

    def _handle_write(self, ins, state):
        mn, ops, dst = ins.mn, ins.ops, ins.dst
        if mn in ("cmp", "test", "scas", "repe", "repne"):
            return
        dr = norm_reg(dst) if dst.startswith("%") and ":" not in dst else None
        if mn == "xchg" and len(ops) == 2:
            ra, rb = norm_reg(ops[0]), norm_reg(dst)
            if ra and rb:
                state[ra], state[rb] = flatten(state.get(rb)), flatten(state.get(ra))
            return
        if mn in ("mov", "lea") or mn.startswith("cmov") or mn.startswith("set") \
                or mn in ("movzbl", "movzwl", "movbe", "movsb", "movsw"):
            if dr:
                if mn.startswith("set"):
                    state[dr] = ("stk", "setcc")
                    return
                src_op = ops[0] if ops else ""
                src = self._expr(src_op, state, addr_mode=(mn == "lea"))
                if src is None and mn != "lea" and self._mem_like(src_op):
                    src = ("stk", "load")
                if src is not None:
                    state[dr] = flatten(src)
                else:
                    state.pop(dr, None)
                return
            if mn == "mov" and self._mem_like(dst):
                val = flatten(self._expr(ops[0], state))
                m = MEM_RE.match(dst)
                if m and not m.group(3):  # no indexed dst
                    d = self._disp_int(m.group(1))
                    base = norm_reg(m.group(2))
                    if base in ("esp", "ebp"):
                        # stack slot save: keep heap/ctorret identity if present
                        if val and val[0] in ("heap", "ctorret"):
                            self.stores.append({"site": ins.addr, "base": ser(("stk",)),
                                                "disp": d, "val": ser(val),
                                                "owner": self.owner_of(ins.addr)})
                        return
                    be = flatten(state.get(base)) if base else None
                    if be is None and val is None:
                        return
                    if be is not None and (
                            val is None or val[0] in ("heap", "ctorret", "addr",
                                                      "contents", "plus", "inecx", "deref")):
                        self.stores.append({"site": ins.addr, "base": ser(be), "disp": d,
                                            "val": ser(val), "owner": self.owner_of(ins.addr)})
                        # HOST INSTALL of a heap-ctor result
                        if val and val[0] == "ctorret":
                            self.installs["C:0x%X" % val[1]].append(
                                (ins.addr, ser(be), d))
                        if val and val[0] == "addr" and self._is_table(val[1]):
                            self.installs["T:0x%X" % val[1]].append((ins.addr, ser(be), d))
                elif ABS_RE.match(dst):
                    v = int(dst, 16)
                    self.global_writes[v].append((ins.addr, ser(flatten(self._expr(ops[0], state)))))
                    if val and val[0] == "addr" and self._is_table(val[1]):
                        self.installs["T:0x%X" % v].append((ins.addr, ser(("addr", v)), 0))
                return
            return
        if mn in ("add", "sub") and dr and len(ops) == 2:
            src_op = ops[0]
            if src_op.startswith("$"):
                imm = int(src_op[1:], 16)
                e = state.get(dr)
                if e and e[0] in ("addr", "plus", "deref"):
                    delta = imm if mn == "add" else -imm
                    if e[0] == "addr":
                        state[dr] = flatten(("plus", e, delta))
                    else:
                        state[dr] = flatten(("plus", e[1] if e[0] == "plus" else e,
                                             (e[2] if e[0] == "plus" else 0) + delta))
                else:
                    state.pop(dr, None)
            else:
                state.pop(dr, None)
            return
        if mn in ("inc", "dec", "and", "or", "xor", "neg", "not", "imul", "mul",
                  "shl", "shr", "sar", "adc", "sbb", "bswap", "cdq", "lea"):
            if mn == "xor" and len(ops) == 2 and ops[0] == dst and dr:
                state[dr] = ("addr", 0)
                return
            if dr:
                state.pop(dr, None)
            else:
                r2 = norm_reg(ops[-1]) if ops else None
                if r2:
                    state.pop(r2, None)
            return
        # anything else: conservatively clear the register destination
        if dr:
            state.pop(dr, None)

    def _mem_like(self, op):
        return bool(op) and (MEM_RE.match(op) or ABS_RE.match(op))

    @staticmethod
    def _disp_int(s):
        if not s:
            return 0
        try:
            return int(s, 16)
        except ValueError:
            return int(s, 10)

    def _is_table(self, v):
        return v in self.tables

    # ---------------- cache ----------------

    def _save_cache(self, path):
        doc = {"md5": self.exe_md5, "edges": self.edges, "indirect": self.indirect,
               "stores": self.stores,
               "pad_installs": {hex(k): v for k, v in self.pad_installs.items()},
               "installs": {k: v for k, v in self.installs.items()},
               "tables": {hex(k): v for k, v in self.tables.items()},
               "thunks": {hex(k): v for k, v in self.thunks.items()},
               "adjusted": [hex(x) for x in self.adjusted_thunks],
               "global_writes": {hex(k): v for k, v in self.global_writes.items()}}
        tmp = path + ".tmp"
        with open(tmp, "w") as f:
            json.dump(doc, f)
        os.replace(tmp, path)

    def _load_cache(self, path):
        with open(path) as f:
            doc = json.load(f)
        if doc.get("md5") != self.exe_md5:
            sys.stderr.write("cache md5 mismatch -- rebuild without --use-cache\n")
            sys.exit(1)
        out = subprocess.run([self.od, "-d", self.exe], capture_output=True, text=True,
                             check=True).stdout
        self._parse_dump(out)
        self._static_index()
        self.edges = doc["edges"]
        self.indirect = doc["indirect"]
        self.stores = doc["stores"]
        self.pad_installs = {int(k, 16): v for k, v in doc.get("pad_installs", {}).items()}
        self.installs = collections.defaultdict(list, doc["installs"])
        self.tables = {int(k, 16): v for k, v in doc["tables"].items()}
        self.thunks.update({int(k, 16): v for k, v in doc["thunks"].items()})
        self.thunks_done = True
        self.adjusted_thunks = {int(x, 16) for x in doc["adjusted"]}
        self.global_writes = collections.defaultdict(
            list, {int(k, 16): [tuple(x) for x in v] for k, v in doc["global_writes"].items()})
        for tva, run in self.tables.items():
            for idx, tgt in enumerate(run):
                self.slotmap[tgt].append((tva, idx))
        for struct in self.f3["structures"].values():
            for tv in struct.get("vftables", {}).values():
                self.f3_tables[int(tv["ea"], 16)] = struct["name"]

    # ---------------- band analysis ----------------

    def owner_class(self, owner_start):
        if owner_start is None:
            return "?"
        if owner_start in getattr(self, "pads", {}):
            return "PAD-THUNK@0x%X" % owner_start
        if owner_start in self.ea2cls:
            return self.ea2cls[owner_start][0]
        return "sub_%X" % owner_start

    def head_hint(self, E):
        """Offline head-ness evidence for a locator: 'call-head' (directly called
        somewhere -> near-certain function entry), 'pad-chunk' (only pad/jmp
        reached, likely a secondary chunk of an earlier head in the live IDB --
        writer MUST read back the containing function), with the nearest
        call-head below as the containment candidate. Never claim IDB head-
        status offline; this is evidence, the IDB is authority (item 43)."""
        import bisect
        if E in self.call_targets:
            return "call-head", E
        ct = sorted(self.call_targets)
        i = bisect.bisect_left(ct, E) - 1
        return "non-call-head (writer read-back required)", (ct[i] if i >= 0 else None)

    def enclosing_fn(self, va):
        """Largest non-pad function head strictly below va (the scope a pad/EH
        handler is compiled inside)."""
        s = self.starts_sorted
        import bisect
        i = bisect.bisect_left(s, va)
        while i > 0:
            i -= 1
            if s[i] not in self.pads:
                return s[i]
        return None

    def bucket_entries(self, band_eas):
        """T-057 taxonomy as an executable rule. Returns dict
        E -> (bucket, evidence) where bucket is:
          LIVE  - direct external edge from a non-pad owner
          PAD   - external edges ONLY from registered pad thunks
                  (evidence bit: pad VA dword-referenced in .rdata/.data --
                   registration REQUIRES it, so a pad edge IS the evidence)
          DELEG - no direct non-pad external edge, but transitive intra-band
                  callers have one (includes tail-jmp-only thunks WITHOUT data
                  ref: those are NOT pads -> DELEG, per the ratified rule)
          ISO   - none of the above
        """
        band = set(band_eas)
        band_r = set(band) | {self.resolve_to(e) for e in band}
        ext = collections.defaultdict(list)
        callers = collections.defaultdict(set)
        for e in self.edges:
            if e["kind"].startswith("alloc"):
                continue
            if e["target"] in band_r:
                o = self.owner_of(e["site"])
                if o in band_r and o != e["target"]:
                    callers[e["target"]].add(o)
                elif o not in band_r:
                    ext[e["target"]].append(e)
        out = {}
        for E in sorted(band):
            es = ext.get(E, [])
            nonpad = [e for e in es if e["owner"] not in self.pads]
            pads = [e for e in es if e["owner"] in self.pads]
            if nonpad:
                out[E] = ("LIVE", {"n": len(nonpad),
                                   "owners": collections.Counter(
                                       self.owner_class(e["owner"])
                                       for e in nonpad),
                                   "edges": nonpad})
            elif pads:
                ph = sorted({e["owner"] for e in pads})
                out[E] = ("PAD", {"pads": ph,
                                  "kinds": [self.pads[p][2] for p in ph],
                                  "data_refs": [self.data_code_refs.get(p, [])[:2]
                                               for p in ph]})
            else:
                seen = set()
                stack = [E]
                up = []
                while stack:
                    x = stack.pop()
                    for c in callers.get(x, ()):
                        if c not in seen:
                            seen.add(c)
                            stack.append(c)
                            up += [e for e in ext.get(c, [])
                                   if e["owner"] not in self.pads]
                if up:
                    out[E] = ("DELEG", {"via": sorted(seen), "edges": up,
                                        "owners": collections.Counter(
                                            self.owner_class(e["owner"])
                                            for e in up)})
                else:
                    out[E] = ("ISO", {})
        return out

    def _holder_field_key(self, ce):
        """Field-level terminal identity (fb3d03f refinement): key by the TERMINAL
        field a path reads, not by the prefix chain that located its base.
          ('contents', G)                  -> ("g", G)       absolute slot
          ('deref', ('addr', G), d)        -> ("g", G + d)   absolute slot
          ('deref', <computed base>, d)    -> ("o", d)       object-relative terminal
                                                 field: in-ecx-rooted, chained-load-
                                                 rooted and untraced-rooted variants
                                                 of the same slot collapse to ONE
                                                 field (prefix sites stay visible;
                                                 proving the bases IDENTIC remains the
                                                 wave's read-back job -- the offset is
                                                 the honest common denominator).
          plus-on-computed-base            -> ("o", disp)    member ADDRESS of a
                                                 loaded object at that offset.
        """
        if not ce:
            return ("?", 0)
        k = ce[0]
        if k == "contents":
            return ("g", ce[1])
        if k == "deref":
            b = ce[1]
            if b and b[0] == "addr":
                return ("g", (b[1] + ce[2]) & 0xFFFFFFFF)
            return ("o", ce[2])
        if k == "plus":
            b = flatten(ce[1]) if ce[1] else None
            if b and b[0] in ("deref", "contents", "stk"):
                return ("o", ce[2])
            return ("?", ce[2])
        return ("?", 0)

    def share_check(self, cand_eas):
        """Harness item 44, rule 1 -- the executable pre-queue gate.
        Input: candidate EAs a wave intends to MINT as members of a host.
        Per candidate, over EXTERNAL edges only (intra-set calls excluded):
          view constants = distinct adjusted-this shapes (in-ecx+disp / &G+disp);
          holder fields  = distinct field-access paths (*[...] chains, *global);
        >=2 view constants OR >=2 holder fields        => per-candidate SHARING
        zero external edges for the candidate           => NO-EDGE (soft: leaf helper
          inside a family is legal; overall NO-EVIDENCE only if the WHOLE set is dead)
        otherwise SINGLE-OWNER (plain-this / heap / untraced edges count as callers
        but carry no shape claim).
        Overall: SHARING if ANY candidate shares (rule 1 is per-fn strict); else
        NO-EVIDENCE if the set has zero external edges at all; else PASS."""
        cand = set(cand_eas)
        cand_r = set(cand) | {self.resolve_to(e) for e in cand}
        details = []
        any_edge_global = False
        for E in sorted(cand):
            targets = {E} | {self.resolve_to(E)}
            vcons = collections.defaultdict(list)
            holders = collections.defaultdict(list)
            plain = heaps = 0
            ctor_fns = set()
            for e in self.edges:
                if e["kind"].startswith("alloc") or e["target"] not in targets:
                    continue
                o = self.owner_of(e["site"])
                if o in cand_r:
                    continue
                any_edge_global = True
                ce = deser(e["ecx"])
                if ce is None:
                    plain += 1
                    continue
                k = ce[0]
                if k == "plus":
                    b = flatten(ce[1]) if ce[1] else None
                    if b and b[0] == "inecx":
                        key = ("this+0x%X" % ce[2]) if ce[2] >= 0 else ("this-0x%X" % -ce[2])
                        vcons[key].append(e["site"])
                    elif b and b[0] == "addr":
                        vcons["&0x%X%s0x%X" % (b[1], "+" if ce[2] >= 0 else "-",
                                               abs(ce[2]))].append(e["site"])
                    else:
                        holders[self._holder_field_key(flatten(ce))].append(e["site"])
                elif k == "contents":
                    holders[self._holder_field_key(ce)].append(e["site"])
                elif k == "deref":
                    holders[self._holder_field_key(ce)].append(e["site"])
                elif k == "inecx":
                    plain += 1
                elif k in ("heap", "ctorret"):
                    heaps += 1
                    ctor_fns.add(o)
                else:
                    plain += 1
            if len(vcons) >= 2 or len(holders) >= 2:
                verdict = "SHARING"
            elif not (vcons or holders or plain or heaps):
                verdict = "NO-EDGE"
            else:
                verdict = "SINGLE-OWNER"
            own = collections.Counter()
            for e in self.edges:
                if e["target"] in targets:
                    o = self.owner_of(e["site"])
                    if o not in cand_r and o is not None:
                        own[self.owner_name(o).split("::")[0]] += 1
            details.append((E, verdict, dict(vcons), dict(holders), plain, heaps,
                            own.most_common(3), len(ctor_fns)))
        if any(d[1] == "SHARING" for d in details):
            overall = "SHARING"
        elif not any_edge_global:
            overall = "NO-EVIDENCE"
        else:
            overall = "PASS"
        return overall, details

    def share_check_report(self, cand_eas, label):
        overall, det = self.share_check(cand_eas)
        L = []
        P = L.append
        P("== share-check %s: item-44 rule-1 pre-queue gate (terminal-FIELD keying per "
          "fb3d03f; prefix chains listed under the field they read) ==" % label)
        for (E, v, vc, ho, pl, hp, own, csites) in det:
            P("  0x%-7X %-13s view-constants=%s holder-fields=%s plain-this=%d heap=%d "
              "ctor-sites=%d suggested-owners=%s" % (
                  E, v,
                  ", ".join("%s(%d@%s)" % (k, len(s), hex(s[0]))
                            for k, s in sorted(vc.items())) or "-",
                  ", ".join("f%s:%s(%d)" % (a, hex(d) if a == "g" else "+0x%X" % d, len(ps))
                            for (a, d), ps in sorted(ho.items(), key=lambda kv: str(kv[0]))) or "-",
                  pl, hp, csites, own))
            byfield = collections.defaultdict(list)
            for kk, ss in ho.items():
                byfield[kk] += ss
            for kk, ss in sorted(byfield.items(), key=lambda kv: str(kv[0])):
                if len(ss) > 1:
                    P("        field %s read via %d paths at sites %s" % (
                        kk, len(ss), ", ".join(hex(x) for x in ss[:6])))
        P("VERDICT: %s  (exit %d -- SHARING/NO-EVIDENCE are queue-blocking)" % (
            overall, 0 if overall == "PASS" else 1))
        return "\n".join(L)

    def pads_report(self):
        L = []
        P = L.append
        kinds = collections.Counter(p[2] + ("-adj" if p[1] else "")
                                    for p in self.pads.values())
        P("== pads census: %d registered pad thunks (rule: ebp->ecx producer "
          "[+disp adjust] then jmp|call T AND pad VA dword-referenced in "
          ".rdata/.data) ==" % len(self.pads))
        P("RULE (item 43): pad TARGETS are usually chunks; the head is the containing")
        P("function. Grade every locator with head_hint() before queueing; the live IDB")
        P("read-back is the sole authority on headness (buckets are flow facts, headness")
        P("is a locator fact).")
        P("kinds: %s" % dict(kinds))
        P("")
        seh = sorted(p for p, v in self.pads.items() if v[2] == "call")
        P("EH-`call` pads (%d) -- per-pad cleanup-owner table:" % len(seh))
        by_target_cls = collections.Counter()
        for p in seh:
            t, adj, _k, d = self.pads[p]
            t_r = self.resolve_to(t)
            enc = self.enclosing_fn(p)
            refs = [hex(x) for x in self.data_code_refs.get(p, [])][:2]
            cls_t = self.ea2cls.get(t_r, ("sub_%X" % t_r, "?"))[0]
            role = self.ea2cls.get(t_r, (None, "?"))[1] if t_r in self.ea2cls else "?"
            by_target_cls[cls_t] += 1
            ins = self.pad_installs.get(p, [])
            P("  pad 0x%X -> 0x%X (%s %s) scope=%s adj=%s data-ref=%s push=%s" % (
                p, t_r, cls_t, role,
                self.owner_name(enc) if enc else "-",
                ("+0x%X" % d if d else ""), refs,
                [hex(i) for i in ins[:2]]))
        P("")
        P("EH-pad target classes: %s" % by_target_cls.most_common(20))
        jmp_pads = [p for p, v in self.pads.items() if v[2] == "jmp"]
        # group the (large) vector/atexit jmp-pad population by target-class of
        # the dtor they dispatch to, with adjustment histogram
        tgt_cls = collections.Counter()
        adj_h = collections.Counter()
        arrays = 0
        for p in jmp_pads:
            t, adj, _k, d = self.pads[p]
            t_r = self.resolve_to(t)
            tgt_cls[self.ea2cls.get(t_r, ("sub_%X" % t_r, "?"))[0]] += 1
            if adj:
                adj_h["+0x%X" % d] += 1
            refs = self.data_code_refs.get(p, [])
            if len(refs) >= 2:
                arrays += 1
        P("vector/atexit `jmp` pads: %d ; in multi-ref arrays: %d" % (len(jmp_pads), arrays))
        P("  top dtor-target classes: %s" % tgt_cls.most_common(15))
        P("  adjustment histogram (member slot destroyed): %s" % adj_h.most_common(15))
        P("")
        P("evidence-bit rule: PAD requires the pad VA to be dword-referenced in a data")
        P("section (registration precondition here); tail-jmp thunks WITHOUT a data-ref")
        P("count as DELEG -- the ratified T-057 taxonomy, executable in bucket_entries().")
        return "\n".join(L)

    def families_report(self, band_eas, label):
        band = set(band_eas)
        band_r = set(band) | {self.resolve_to(e) for e in band}
        self._resolve_thunks()
        rev = collections.defaultdict(list)
        for t, r in self.thunks.items():
            rev[r].append(t)
        L = []
        P = L.append
        P("== families %s: %d entries, %d tables scanned (.rdata+.data+.data1), "
          "%d thunks resolved ==" % (label, len(band), len(self.tables), len(self.thunks)))
        ext_by_entry = collections.defaultdict(list)
        intra = 0
        for e in self.edges:
            if e["kind"].startswith("alloc"):
                continue
            if e["target"] in band_r:
                o = self.owner_of(e["site"])
                if o in band_r:
                    intra += 1
                else:
                    ext_by_entry[e["target"]].append(e)
        P("external edges into band: %d ; intra-band: %d" %
          (sum(len(v) for v in ext_by_entry.values()), intra))
        # intra-band call graph for delegation resolution (Q1 second hop)
        intra_edges = collections.defaultdict(list)
        for e in self.edges:
            if e["kind"].startswith("alloc"):
                continue
            o = self.owner_of(e["site"])
            if e["target"] in band_r and o in band_r:
                intra_edges[e["target"]].append(o)
        rows = []
        for E in sorted(band):
            es = ext_by_entry.get(E, [])
            oc = collections.Counter(self.owner_class(x["owner"]) for x in es)
            shapes = collections.Counter()
            disps = collections.Counter()
            for x in es:
                ce = deser(x["ecx"])
                shapes[ce[0] if ce else "none"] += 1
                if ce and ce[0] == "plus":
                    b = ce[1]
                    if b and b[0] == "inecx":
                        disps["in-ecx%+d" % ce[2]] += 1
                    elif b and b[0] == "addr":
                        disps["&0x%X%+d" % (b[1], ce[2])] += 1
            slots = list(self.slotmap.get(E, ()))
            for t in rev.get(E, ()):
                slots += [(tv, ix) for (tv, ix) in self.slotmap.get(t, ())]
            rows.append((E, es, oc, shapes, disps, slots))
        # family grouping by dominant owner class
        fams = collections.defaultdict(list)
        for r in rows:
            E, es, oc, shapes, disps, slots = r
            if not es:
                top = "ZERO-EXTERNAL"
                share = 0.0
            else:
                top, c = oc.most_common(1)[0]
                share = c / len(es)
            fams[top].append((E, es, oc, shapes, disps, slots, share))
        for top, items in sorted(fams.items(), key=lambda kv: -len(kv[1])):
            n_e = sum(len(x[1]) for x in items)
            owner_known = top != "ZERO-EXTERNAL" and all(
                x[6] >= 0.55 for x in items) and len(items) and n_e > 0
            P("")
            P("FAMILY %s  entries=%d edges=%d dominant_share_min=%.2f" % (
                top, len(items), n_e,
                min((x[6] for x in items), default=0.0)))
            for (E, es, oc, shapes, disps, slots, share) in items:
                P("  0x%X edges=%d owner_share=%.2f owners=%s" % (
                    E, len(es), share,
                    ", ".join("%s:%d" % (k, v) for k, v in oc.most_common(4))))
                if not es:
                    # Q1 second hop: who calls the band-entry that calls this?
                    del_g = collections.Counter()
                    up_shapes = collections.Counter()
                    for I in intra_edges.get(E, ()):
                        for e2 in ext_by_entry.get(I, []):
                            del_g[self.owner_class(e2["owner"])] += 1
                            ce2 = deser(e2["ecx"])
                            up_shapes[ce2[0] if ce2 else "none"] += 1
                        slots_I = self.slotmap.get(I, [])
                        if slots_I:
                            del_g["VFT-OF-CALLER 0x%X@%s" % (I, slots_I[0])] += 0
                    if del_g:
                        P("        [Q1-delegated] via intra-band callers %s: "
                          "external owners=%s shapes=%s" % (
                              [hex(x) for x in sorted(set(intra_edges.get(E, ())))][:6],
                              ", ".join("%s:%d" % (k, v) for k, v in del_g.most_common(6)),
                              dict(up_shapes)))
                    else:
                        via_pad = [(p, adj) for p, (t, adj, _k, _d) in self.pads.items()
                                   if self.resolve_to(t) == E]
                        if via_pad:
                            for p, adj in via_pad[:3]:
                                inst = self.pad_installs.get(p, [])
                                hh, alt = self.head_hint(E)
                                P("        [Q1-padthunk] evidence=dataref@%s "
                                  "pad 0x%X (%s%s) -> entry; entry-headness=%s%s; "
                                  "push-installs=%s%s"
                                  % (
                                      [hex(x) for x in self.data_code_refs.get(p, [])][:2],
                                      p, self.pads[p][2], ",adj" if adj else "",
                                      hh, (" (containment candidate 0x%X)" % alt) if alt and alt != E else "",
                                      [hex(x) for x in inst[:4]] or "none",
                                      "installed-by is the candidate host" if inst else ""))
                        else:
                            P("        [Q1-delegated] no external flow through any "
                              "intra-band caller either")
                    thk_E = sorted(rev.get(E, ()))
                    tabs_E = sorted({tv for t in thk_E
                                     for (tv, ix) in self.slotmap.get(t, ())})
                    if thk_E or tabs_E:
                        P("        [Q2] thunks into entry=%s tables-via-thunk=%s" % (
                            [hex(t) for t in thk_E], [hex(t) for t in tabs_E]))
                if disps:
                    P("        disps: %s" % ", ".join("%s(%d)" % (k, v)
                                                      for k, v in disps.most_common(6)))
                P("        shapes: %s ; vft-slots-into-entry: %d%s" % (
                    dict(shapes), len(slots),
                    " " + str(slots[:4]) if slots else ""))
            # item-41 triplet, family level
            all_slots = sum(len(x[5]) for x in items)
            thk = sorted({t for x in items for t in rev.get(x[0], ())})
            P("  [Q2] table->thunk->family: slots=%d thunks=%s" % (
                all_slots, [hex(t) for t in thk] or "none"))
            callers = collections.Counter()
            for x in items:
                for e in x[1]:
                    callers[self.owner_class(e["owner"])] += 1
            for clsname, cnt in callers.most_common(6):
                # Q1: who calls the callers? top owning classes' own tables + callers
                heads = [ea for ea, (c, _n) in self.ea2cls.items() if c == clsname]
                own_tabs = sorted({tv for ea in heads for (tv, ix) in self.slotmap.get(ea, ())})
                up = collections.Counter()
                for ea in heads:
                    for e2 in self.edges:
                        if e2["target"] == ea and self.owner_of(e2["site"]) != ea:
                            up[self.owner_class(self.owner_of(e2["site"]))] += 1
                P("  [Q1] caller-class %s: its own vft tables=%s ; called-by=%s" % (
                    clsname, [hex(t) for t in own_tabs[:4]] or "none",
                    ", ".join("%s:%d" % (k, v) for k, v in up.most_common(4)) or "none"))
        return "\n".join(L)

    def band_report(self, band_eas, label):
        band = set(band_eas)
        band_r = set(band) | {self.resolve_to(e) for e in band}
        L = []
        P = L.append
        P("== band %s: %d entries: %s ==" % (label, len(band),
                                             ",".join("0x%X" % x for x in sorted(band))))
        for e in band:
            if e in self.ea2cls:
                c, n = self.ea2cls[e]
                P("   entry 0x%X = %s::%s" % (e, c, n))

        def site_in_band(s):
            o = self.owner_of(s)
            return o in band_r or o in band

        ext = [e for e in self.edges if e["target"] in band_r
               and e["kind"] != "alloc:new" and not site_in_band(e["site"])]
        intra = [e for e in self.edges if e["target"] in band_r and site_in_band(e["site"])]
        P("[A/D] external call edges into band: %d (intra-band %d)" % (len(ext), len(intra)))
        byhost = collections.Counter()
        first = {}
        for e in ext:
            o = self.owner_name(e["owner"])
            byhost[o] += 1
            first.setdefault(o, e)
        for o, c in byhost.most_common(20):
            e = first[o]
            P("   %6d calls from %s  e.g. site 0x%X -> 0x%X this=%s%s" % (
                c, o, e["site"], e["target"], efmt(deser(e["ecx"])),
                " [via " + e["kind"] + "]" if e["kind"].startswith("call-via") else ""))

        pin_a = collections.Counter()
        pin_c = collections.Counter()
        embeds = []
        heaps = []
        for e in ext + intra:
            ce = deser(e["ecx"])
            if not ce:
                continue
            if ce[0] == "addr":
                pin_a[ce[1]] += 1
            elif ce[0] == "contents":
                pin_c[ce[1]] += 1
            elif ce[0] == "plus" and ce[1] and ce[1][0] == "addr":
                embeds.append((ce, e))
            elif ce[0] == "plus" and ce[1] and ce[1][0] == "inecx":
                embeds.append((ce, e))
            elif ce[0] == "heap":
                heaps.append(e)
        if pin_a:
            P("[A] this==&global at band call sites:")
            for g, c in pin_a.most_common(8):
                P("   %6d  &0x%X" % (c, g))
        if pin_c:
            P("[A] this==*global (pointer-slot singleton) at band call sites:")
            for g, c in pin_c.most_common(8):
                P("   %6d  *[0x%X]" % (c, g))
        if embeds:
            P("[A] embed edges (this = base+disp) : %d" % len(embeds))
            for ce, e in embeds[:20]:
                P("   0x%X -> 0x%X this=%s (owner %s)" % (
                    e["site"], e["target"], efmt(ce), self.owner_name(e["owner"])))
        if heaps:
            P("[D] heap-ctor edges into band (allocator/ctor caller join): %d" % len(heaps))
            for e in heaps[:10]:
                P("   site 0x%X new(0x%X) -> ctor 0x%X (owner %s)" % (
                    e["site"], deser(e["ecx"])[1], e["target"], self.owner_name(e["owner"])))
                key = "C:0x%X" % e["target"]
                for (s, be, d) in self.installs.get(key, []):
                    if abs(s - e["site"]) < 0x80:
                        P("     + install site 0x%X: [ %s + 0x%X ] = ctor result" % (
                            s, efmt(deser(be)), d & 0xFFFFFFFF))
                # weak tier: [this+disp] stores nearby
                near = [st for st in self.stores
                        if st["owner"] == e["owner"] and 0 < st["site"] - e["site"] <= 0x60
                        and deser(st["base"]) and deser(st["base"])[0] in ("inecx", "plus")
                        and st["base"] not in ("stk",)]
                for st in near[:4]:
                    P("     ~ nearby field store 0x%X: [ %s + 0x%X ] <- %s" % (
                        st["site"], efmt(deser(st["base"])), st["disp"] & 0xFFFFFFFF,
                        efmt(deser(st["val"]))))

        self._resolve_thunks()
        ths = sorted({t for t, r in self.thunks.items() if r in band_r})
        P("[B] thunks resolving into band: %d" % len(ths))
        for t in ths:
            callers = [e for e in self.edges if e.get("raw_target") == t]
            P("   thunk 0x%X (%s) -> 0x%X ; direct callers %d: %s" % (
                t, "adj" if t in self.adjusted_thunks else "plain", self.thunks[t],
                len(callers), ", ".join("0x%X(%s)" % (c["site"], self.owner_name(c["owner"]))
                                        for c in callers[:6])))

        # second hop: vft membership of the HOST heads found above
        host_heads = collections.Counter(e["owner"] for e in ext if e["owner"])
        if host_heads:
            P("[C+] vft membership of host functions (second hop):")
            rev = collections.defaultdict(list)
            for t, r in self.thunks.items():
                rev[r].append(t)
            for o, c in host_heads.most_common(12):
                found = list(self.slotmap.get(o, ()))
                for t in rev.get(o, ()):
                    found += [(tv, ix) for (tv, ix) in self.slotmap.get(t, ())
                              if t in self.thunks]
                    found.append(("thunk-", t))
                if found:
                    desc = []
                    for f in found:
                        if f[0] == "thunk-":
                            desc.append("via thunk 0x%X" % f[1])
                        else:
                            tv, ix = f
                            desc.append("table 0x%X(%s) idx%d" % (
                                tv, self.f3_tables.get(tv, "?"), ix))
                    P("   host 0x%X (%s): %s" % (o, self.owner_name(o), "; ".join(desc[:6])))

        hits = set()
        for ea in band_r:
            for (tva, idx) in self.slotmap.get(ea, ()):
                hits.add((tva, idx, ea))
        P("[C] raw vft scan (all data sections): slots into band: %d ; tables scanned: %d" %
          (len(hits), len(self.tables)))
        tabset = collections.defaultdict(list)
        for tva, idx, ea in hits:
            tabset[tva].append((idx, ea))
        for tva, es in sorted(tabset.items()):
            P("   table 0x%X (%s) len %d slots into band: %s" % (
                tva, self.f3_tables.get(tva, "unnamed"), len(self.tables[tva]),
                ", ".join("idx%d(+0x%X)->0x%X" % (i, i * 4, e) for i, e in sorted(es))))
            for (s, be, d) in self.installs.get("T:0x%X" % tva, [])[:8]:
                P("      INSTALL 0x%X: [ %s ] <- table (owner %s)" % (
                    s, efmt(deser(be)), self.owner_name(self.owner_of(s))))
            for i, e in sorted(es):
                off = i * 4
                ds = [x for x in self.indirect if x["slot"] == off]
                P("      dispatch sites with slot 0x%X: %d" % (off, len(ds)))
                cnt = collections.Counter(self.owner_name(x["owner"]) for x in ds)
                for o, c in cnt.most_common(6):
                    ex = next(x for x in ds if self.owner_name(x["owner"]) == o)
                    obj = deser(ex["obj"])
                    P("         %4d  from %s  e.g. 0x%X obj=%s%s" % (
                        c, o, ex["site"], efmt(obj),
                        "+0x%X" % ex["obj_disp"] if ex["obj_disp"] else ""))
        return "\n".join(L)

    def global_report(self, g):
        L = []
        P = L.append
        P("== global 0x%X ==" % g)
        addr_edges = [e for e in self.edges if deser(e["ecx"]) == ("addr", g)]
        cont_edges = [e for e in self.edges if deser(e["ecx"]) == ("contents", g)]
        plus_edges = [e for e in self.edges
                      if deser(e["ecx"]) and deser(e["ecx"])[0] == "plus"
                      and deser(deser(e["ecx"])[1]) and deser(e["ecx"])[1][0] in ("addr", "inecx")
                      and ((deser(e["ecx"])[1][0] == "addr"
                            and deser(e["ecx"])[1][1] + deser(e["ecx"])[2] == g))]
        P("this==&global edges: %d ; this==*global edges: %d ; embed-into-global edges: %d" %
          (len(addr_edges), len(cont_edges), len(plus_edges)))
        tg = collections.Counter()
        tg2 = collections.Counter()
        for e in addr_edges:
            tg[e["target"]] += 1
        for e in cont_edges:
            tg2[e["target"]] += 1
        P("targets called with this==&0x%X (top 15):" % g)
        for t, c in tg.most_common(15):
            P("   %5d  0x%X (%s)" % (c, t, self.owner_name(t)))
        if tg2:
            P("targets called with this==*[0x%X]:" % g)
            for t, c in tg2.most_common(15):
                P("   %5d  0x%X (%s)" % (c, t, self.owner_name(t)))
        for e in plus_edges[:10]:
            P("   embed-from site 0x%X -> 0x%X this=%s (%s)" % (
                e["site"], e["target"], efmt(deser(e["ecx"])), self.owner_name(e["owner"])))
        P("direct global writes: %s" % [hex(w[0]) for w in self.global_writes.get(g, [])[:12]])
        # who takes address-of via lea/push without ecx (e.g. stores it to a field)?
        P("tables whose slots target globals? n/a")
        return "\n".join(L)


# ---------------------------------------------------------------------------
# selftest (harness item 35 gate)
# ---------------------------------------------------------------------------

GFX_D3D = [0x490060, 0x4900A0, 0x502BE0, 0x503320, 0x503360, 0x503390, 0x503400, 0x503920]
ANIM_CTOR = 0x4A5BE0
TEXMGR = [0x51F160, 0x51F340, 0x51F3E0, 0x51F570, 0x520A60, 0x520FC0, 0x5212E0, 0x521CA0]


def selftest(m):
    results = []

    def check(name, ok, detail):
        results.append((name, ok, detail))

    band = set(GFX_D3D)
    n = [e for e in m.edges if e["target"] in band
         and deser(e["ecx"]) == ("addr", 0x7082F8)]
    check("T1 Gfx_D3D <- &0x7082F8 singleton-this join", len(n) >= 100,
          "%d edges this==&0x7082F8 into Gfx_D3D band (b5: verified 170+ sites)" % len(n))

    heapn = [e for e in m.edges if e["target"] == ANIM_CTOR
             and deser(e["ecx"]) and deser(e["ecx"])[0] == "heap"]
    strong = [i for i in m.installs.get("C:0x%X" % ANIM_CTOR, [])]
    weak = []
    for e in heapn:
        weak += [s for s in m.stores if s["owner"] == e["owner"]
                 and 0 < s["site"] - e["site"] <= 0x60
                 and deser(s["base"]) and deser(s["base"])[0] in ("inecx", "plus")]
    check("T2 Gfx_AnimMeshInstance ctor <- allocator/caller join (b6-corrected shape)",
          len(heapn) >= 4 and (len(strong) + len(weak)) >= 2,
          "%d new(0xC8)->ctor_0x4A5BE0 edges; %d strong [this+disp] installs, "
          "%d weak nearby field stores (b5's '+0x200 embed' corrected by b6 to "
          "heap-arg at +0x2C0 / store +0x2C4)" % (len(heapn), len(strong), len(weak)))

    embed = [e for e in m.edges if e["target"] == 0x56E0B0
             and deser(e["ecx"]) == ("plus", ("inecx",), 0x44)]
    check("T2b embed detector known-positive (ModelInstance ctor -> 0x56E0B0 @ this+0x44)",
          len(embed) >= 1, "%d edges this=in-ecx+0x44 (site 0x564D3C family)" % len(embed))

    bandt = set(TEXMGR)
    nt = [e for e in m.edges if e["target"] in bandt
          and deser(e["ecx"]) == ("contents", 0x70B200)]
    check("T3 Gfx_Texture_Manager <- *[0x70B200] pointer-slot join", len(nt) >= 5,
          "%d edges this==*[0x70B200] into manager band (b6 join reproduced)" % len(nt))

    # T5: item-44 rule-1 regression on the BANKED rulings (T-064 wave outcome):
    #  [B] set must read SHARING (the falsification); 445350/507810 mints must read PASS.
    v1, _ = m.share_check([0x434250])
    v2, _ = m.share_check([0x434120])
    hot = mod_band = [0x445350, 0x445380, 0x4455D0, 0x445970, 0x445A40, 0x445E20,
                       0x446080, 0x4461D0]
    v3, _ = m.share_check(hot)
    v4, _ = m.share_check([0x507810])
    fam = resolve_cls_eas(m, "cls_0x490150") or []
    v5, _ = m.share_check(fam)
    check("T5 share-check gate reproduces the banked item-44 rulings "
          "(incl. fb3d03f field-keying counterexample)",
          v1 == "SHARING" and v2 == "SHARING" and v3 == "PASS" and v4 == "PASS"
          and v5 == "PASS",
          "0x434250=%s 0x434120=%s cls_0x445350-band=%s 0x507810=%s "
          "cls_0x490150-family=%s (expected SHARING/SHARING/PASS/PASS/PASS per "
          "T-064 outcome + T-054 mints + fb3d03f Gfx_RenderManager "
          "single-terminal-field truth)" % (v1, v2, v3, v4, v5))

    # T4: T-057 21-row PAD snapshot + 0-orphan invariant, via the executable
    # bucket rule (PAD requires the pad's data-ref evidence; tail-jmp-only -> DELEG)
    snap = {
        "cls_0x50dbb0": {0x48EE00, 0x4933D0, 0x493400, 0x49F290, 0x4B51F0, 0x4B5220,
                         0x4BC020, 0x507550, 0x50DAF0, 0x520340, 0x54ECC0, 0x54ECF0,
                         0x572EE0, 0x593500, 0x5CB270},
        "cls_0x50db20": set(),
        "cls_0x4542d0": {0x422920, 0x4A40B0, 0x4BB6B0, 0x54ED20, 0x57F2D0, 0x5984F0},
    }
    ok4 = True
    detail4 = []
    iso_tot = 0
    for cls, want in snap.items():
        eas = resolve_cls_eas(m, cls)
        if eas is None:
            ok4 = False
            detail4.append("%s missing from dump" % cls)
            continue
        b = m.bucket_entries(eas)
        got = {E for E, (k, _ev) in b.items() if k == "PAD"}
        iso_tot += sum(1 for _E, (k, _ev) in b.items() if k == "ISO")
        if got != want:
            ok4 = False
            detail4.append("%s PAD mismatch extra=%s missing=%s" % (
                cls, sorted(hex(x) for x in got - want),
                sorted(hex(x) for x in want - got)))
    if ok4:
        detail4.append("21/21 PAD rows exact + 0-orphan across the 3 bands")
    check("T4 T-057 21-row PAD snapshot reproduces + 0-orphan invariant", ok4 and iso_tot == 0,
          "; ".join(detail4))

    ok = all(r[1] for r in results)
    for name, passed, detail in results:
        print("%s %s\n    %s" % ("PASS" if passed else "FAIL", name, detail))
    return ok


def resolve_cls_eas(m, cls):
    for struct in m.f3["structures"].values():
        if struct["name"] == cls:
            return [int(k, 16) for k in struct["methods"]]
    for struct in m.f3["structures"].values():
        if struct["name"].endswith("_" + cls.replace("cls_", "")):
            return [int(k, 16) for k in struct["methods"]]
    return None


def main():
    ap = argparse.ArgumentParser(prog="ecx-flow.py")
    ap.add_argument("mode", choices=["selftest", "band", "global", "cases", "families",
                                     "pads", "share-check"])
    ap.add_argument("--cls")
    ap.add_argument("--addrs")
    ap.add_argument("--label", default=None)
    ap.add_argument("globpos", nargs="?", default=None, help="VA for global mode")
    ap.add_argument("--exe", default=DEFAULT_EXE)
    ap.add_argument("--f3json", default=DEFAULT_F3JSON)
    ap.add_argument("--no-cache", action="store_true")
    ap.add_argument("--allow-red", action="store_true")
    args = ap.parse_args()

    if not args.exe or not os.path.exists(args.exe):
        sys.stderr.write("ERROR: EXE not found (set $OPENVB_ORIG_EXE or --exe)\n")
        return 2
    m = Model(args.exe, args.f3json)
    m.build(cache=not args.no_cache)

    if args.mode == "selftest":
        return 0 if selftest(m) else 1

    gate_ok = selftest(m) if args.mode in (
        "band", "global", "cases", "families", "pads", "share-check") else True

    if args.mode == "share-check":
        if not gate_ok and not args.allow_red:
            print("\nGATE RED (item 35) -- share-check verdicts suppressed.")
            return 1
        eas = None
        label = args.label or args.cls or "candidates"
        if args.cls:
            eas = resolve_cls_eas(m, args.cls)
            if eas is None:
                print("cls not found in F3.json dump: %s" % args.cls)
                return 2
        elif args.addrs:
            eas = [int(x, 16) for x in args.addrs.split(",")]
        else:
            print("share-check mode needs --cls or --addrs")
            return 2
        print(m.share_check_report(eas, label))
        _overall, _det = m.share_check(eas)
        return 0 if _overall == "PASS" else 1

    if args.mode == "pads":
        if not gate_ok and not args.allow_red:
            print("\nGATE RED (item 35) -- pads census suppressed.")
            return 1
        print(m.pads_report())
        return 0

    if args.mode == "cases":
        if not gate_ok and not args.allow_red:
            print("\nGATE RED (item 35) -- band verdicts suppressed.")
            return 1
        cases = [("cls_0x445350 (hotkey, b5 escalated)",
                  resolve_cls_eas(m, "cls_0x445350"), None),
                 ("cls_0x4b28a0 (GUI root, owner-unresolved)",
                  resolve_cls_eas(m, "cls_0x4b28a0"), None),
                 ("cls_0x507810 epoch (byte_707CE0 anchor, b6 negative)",
                  [0x507810, 0x507580, 0x5067C0, 0x506890], None)]
        for label, eas, _ in cases:
            print(m.band_report(eas, label))
            print()
        return 0

    if args.mode in ("band", "families"):
        if not gate_ok and not args.allow_red:
            print("\nGATE RED (item 35): selftest positive failed; band verdicts "
                  "suppressed -- fix the scanner first or pass --allow-red.")
            return 1
        eas = None
        label = args.label or args.cls or "band"
        if args.cls:
            eas = resolve_cls_eas(m, args.cls)
            if eas is None:
                print("cls not found in F3.json dump: %s" % args.cls)
                return 1
        elif args.addrs:
            eas = [int(x, 16) for x in args.addrs.split(",")]
        else:
            print("%s mode needs --cls or --addrs" % args.mode)
            return 2
        if args.mode == "families":
            print(m.families_report(eas, label))
        else:
            print(m.band_report(eas, label))
        return 0

    if args.mode == "global":
        g = args.globpos or args.cls
        if not g:
            print("global mode needs a VA")
            return 2
        if not gate_ok and not args.allow_red:
            print("\nGATE RED (item 35) -- fix the scanner first or pass --allow-red.")
            return 1
        print(m.global_report(int(g, 16)))
        return 0

    return 2


if __name__ == "__main__":
    sys.exit(main())
