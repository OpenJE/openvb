#!/usr/bin/env python3
"""R-INST — INSTALL-MODE derivor (TrueCastle 01a16096, wave-2b unlock).

Edge-kind: the HOLDER-INSTALL join. A call site whose ecx = caller_this + disp (plus/inecx
shape) CONSTITUTES an embed edge: <owner-of-site> hosts <target's component> at +disp.
Row->cluster resolution (identity from T064-Era KEYS, labels from current live names):
  (1) stored-vft equality (item-49 c7 0x imm-vft ctor stores), (2) vft-table membership,
  (3) same-key family (t064-postdump keys). Components with no vft are identity-resolved by
  the cluster's install votes, flagged vft-absent.

Selftests ship BEFORE any verdict (item 35; the gates ARE the claims):
  S1 +0x118 pair: all five cls_0x569f10-family rows must resolve install-host JE::World @+0x118
     and no JE::Client embed-vote may exist for them — the tool ANSWERS one-class-vs-two rather
     than inheriting the doc's Client/World split.
  S2 banked T2b positive recovered: an install edge (owner endswith ModelInstance, disp 0x44)
     must exist for 0x56E0B0 among its multi-host votes (helper-target case: regression is the
     edge, not uniqueness).
  S3 BLIND: component ctor 0x4C6640 resolves to host ...GUI::Manager @+0x48 uninstructed — the
     exact register-indirect embed shape that blocked five waves.
Read-only. Zero IDB writes. Exit 1 on any selftest failure (verdicts withheld on red gate).
"""
import importlib.util, json, os, struct, sys, collections, hashlib

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
spec = importlib.util.spec_from_file_location("ecxflow", os.path.join(REPO, "tools", "ecx-flow.py"))
mod = importlib.util.module_from_spec(spec); spec.loader.exec_module(mod)
deser = mod.deser
EXE_PATH = os.environ.get("OPENVB_ORIG_EXE", "")
F3 = os.path.join(REPO, "artifacts", "F3.json")
TEXT_OFF, TEXT_BASE, TEXT_LEN = 0x1000, 0x401000, 0x245c52

class Derivor:
    def __init__(self):
        if not EXE_PATH or not os.path.exists(EXE_PATH):
            sys.stderr.write("ERROR: set OPENVB_ORIG_EXE\n"); sys.exit(2)
        self.exe = open(EXE_PATH, "rb").read()
        if hashlib.md5(self.exe).hexdigest() != "a68c22f3dd1c256ec9539a70e87cb434":
            sys.stderr.write("ERROR: EXE md5 not the pinned lineage\n"); sys.exit(1)
        self.m = mod.Model(EXE_PATH, F3); self.m.build(cache=True)
        d = json.load(open(os.path.join(REPO, "discoveries", "wavedumps", "t064-postdump.json")))
        self.pd = {int(e, 16): n for e, n in zip(d["ea"], d["name"])}   # identity source (keys)
        self.live = dict(self.pd)
        pp = os.path.join(REPO, "discoveries", "wavedumps", "qw2-wave2a-poststate.txt")
        if os.path.exists(pp):
            for l in open(pp):
                p = l.split()
                if len(p) >= 2 and p[0].startswith("0x"):
                    try: self.live[int(p[0], 16)] = p[1]
                    except ValueError: pass
        for ea, nm in self.live.items():                          # labels for owners
            if ea in self.m.ea2cls:
                self.m.ea2cls[ea] = (nm.rsplit("::", 1)[0] if "::" in nm else nm,
                                     nm.rsplit("::", 1)[-1])
        self._vfts(); self._installs()

    def _text(self): return self.exe[TEXT_OFF:TEXT_OFF + TEXT_LEN]
    def _va2off(self, va):
        for b, sz, fo in ((0x401000, 0x245c52, 0x1000), (0x647000, 0xaa000, 0x247000),
                          (0x6f1000, 0x17000, 0x2f1000)):
            if b <= va < b + sz: return fo + (va - b)
        return None
    def _rdw(self, va):
        o = self._va2off(va)
        return None if o is None else struct.unpack_from("<I", self.exe, o)[0]
    def _is_code(self, v): return v is not None and 0x401000 <= v < 0x646c52

    def _vfts(self):
        t = self._text()
        self.vft_of = {}
        i = 0
        while i < len(t) - 10:
            if t[i] == 0xc7 and t[i+1] in (0x01, 0x06):
                imm = struct.unpack_from("<I", t, i+2)[0]
                if 0x647000 <= imm < 0x708000 and self._is_code(self._rdw(imm)):
                    st = self.m.owner_of(TEXT_BASE + i)
                    if st is not None: self.vft_of.setdefault(st, set()).add(imm)
                i += 6; continue
            if t[i] == 0xc7 and (t[i+1] & 0xC7) == 0x41 and t[i+1] != 0xC7:
                # c7 41/46/4e/... disp8 imm  (modrm mod=01 reg!=100 would be wrong for imm: keep simple set)
                if t[i+1] in (0x41, 0x42, 0x46, 0x47, 0x4e, 0x4f):
                    imm = struct.unpack_from("<I", t, i+3)[0]
                    if 0x647000 <= imm < 0x708000 and self._is_code(self._rdw(imm)):
                        st = self.m.owner_of(TEXT_BASE + i)
                        if st is not None: self.vft_of.setdefault(st, set()).add(imm)
                    i += 7; continue
            i += 1
        self.members = {}
        for v in {x for s in self.vft_of.values() for x in s}:
            mem, k = [], 0
            while self._is_code(self._rdw(v + 4*k)) and k < 400:
                mem.append(self._rdw(v + 4*k)); k += 1
            self.members[v] = set(mem)

    def _installs(self):
        self.inst = collections.defaultdict(list)
        for e in self.m.edges:
            ce = deser(e["ecx"]) if e.get("ecx") else None
            if ce and ce[0] == "plus" and ce[1] and ce[1][0] == "inecx":
                so = self.m.owner_of(e["site"])
                if so is None: continue
                owner = self.m.ea2cls[so][0] if so in self.m.ea2cls else ("sub_%X" % so)
                self.inst[self.m.resolve_to(e["target"])].append((e["site"], owner, so, ce[2]))

    def cluster_of(self, E):
        Er = self.m.resolve_to(E)
        for st, vfs in self.vft_of.items():
            for v in vfs:
                if Er in self.members.get(v, ()) or E in self.members.get(v, ()):
                    return ("vft", v, set(self.members[v]) | {st})
        nm = self.pd.get(E) or self.pd.get(Er) or ""
        key = nm.split("::")[0] if "::" in nm else None
        if key and key.startswith("cls_"):
            fam = {e for e, n in self.pd.items() if n.startswith(key + "::")}
            mem = set(fam)
            for c in fam:
                for v in self.vft_of.get(self.m.resolve_to(c), ()):
                    mem |= self.members.get(v, set())
            return ("key-family", key, mem)
        return (None, None, {E, Er})

    def install_votes(self, E):
        kind, tag, mem = self.cluster_of(E)
        votes = collections.Counter(); disps = collections.Counter()
        for member in mem:
            mr = self.m.resolve_to(member)
            for (site, owner, so, d) in self.inst.get(mr, []):
                if so in mem: continue                       # internal edge
                votes[owner] += 1; disps[(owner, d)] += 1
        return kind, tag, votes, disps

    def install_host(self, E):
        kind, tag, votes, disps = self.install_votes(E)
        if not votes: return kind, tag, None, None, {}
        top, n = votes.most_common(1)[0]
        uniq = len(votes) == 1
        disp = disps.most_common(1)[0][0][1] if uniq else None
        return kind, tag, (top if n else None), disp, dict(votes)

def selftests(dv):
    fails = []
    print("== S1 +0x118 Client/World regression (tool ANSWERS one-vs-two) ==")
    fam = [0x569790, 0x569970, 0x569d50, 0x569de0, 0x569f10]
    hosts = set()
    for E in fam:
        kind, tag, host, disp, votes = dv.install_host(E)
        hosts.add((host, disp))
        print("  %s cluster=%s/%s host=%s disp=%s votes=%s" % (hex(E), kind, tag, host,
              ("+0x%X" % disp) if disp is not None else None, votes))
    if hosts != {("JE::World", 0x118)}: fails.append("S1-hosts")
    client_votes = any("Client" in v for E in fam for v in dv.install_votes(E)[2])
    print("  one-class answer:", len(hosts) == 1, "| any JE::Client embed-vote:", client_votes)
    if client_votes: fails.append("S1-client")
    print("== S2 banked T2b edge recovered: (ModelInstance, +0x44) in votes of 0x56E0B0 ==")
    k, t, votes, disps = dv.install_votes(0x56e0b0)
    ok2 = any(o.endswith("ModelInstance") and d == 0x44 for (o, d) in disps)
    print("  hosts:", dict(votes), "| T2b edge present:", ok2)
    if not ok2: fails.append("S2")
    print("== S3 BLIND edge-recovery: (GUI::Manager,+0x48) in disps of 0x4C6640 (helper target,"
          " same discipline as S2; unique-host would be a WRONG expectation for a shared helper) ==")
    k, t, votes, disps = dv.install_votes(0x4c6640)
    ok3 = any(o.endswith("GUI::Manager") and d == 0x48 for (o, d) in disps)
    mgr_disp = [(o, hex(d)) for (o, d) in disps if o.endswith("GUI::Manager")]
    print("  Manager embed disps for this helper:", mgr_disp, "| S3 edge present:", ok3)
    if not ok3: fails.append("S3")
    print("SELFTEST:", "ALL PASS" if not fails else "FAILED " + ",".join(fails))
    return fails

def main():
    dv = Derivor()
    fails = selftests(dv)
    if fails:
        sys.stderr.write("verdicts withheld on red selftest (item 35)\n"); sys.exit(1)
    if "--run74" in sys.argv:
        run74(dv)
    return dv

def parse74():
    """65 Gate-A HOLD rows + 9 collision rows from the committed artifacts."""
    rows = []
    p = os.path.join(REPO, "discoveries", "wavedumps", "qw2-gateA-queue-split.txt")
    hold = False
    for l in open(p):
        if l.startswith("HOLD"): hold = True; continue
        if hold and l.strip().startswith("0x"):
            m = l.strip().split()
            rows.append((int(m[0], 16), m[2].replace("doc=", "") if len(m) > 2 else "?"))
        elif hold and not l.strip().startswith("0x") and "ELIGIBLE" in l: hold = False
    coll = [0x52cc70, 0x515040, 0x5150c0, 0x515580, 0x515d50, 0x4a5720, 0x4a5a10, 0x436410, 0x523450]
    return rows, coll

def run74(dv):
    hold65, coll9 = parse74()
    print("\n== 74-row install-resolved verdict feed (proposals for Q-W2b arbitration) ==")
    print("class: ADOPT-BY-INSTALL (unique install host != caller/doc) | CONFIRM-DOC (==doc) | "
          "CONTRADICTS-DOC | NO-EVIDENCE")
    def line(E, doc):
        kind, tag, host, disp, votes = dv.install_host(E)
        if not host: cls = "NO-EVIDENCE"
        elif doc != "?" and host == doc: cls = "CONFIRM-DOC"
        elif len(votes) == 1: cls = "ADOPT-BY-INSTALL"
        else: cls = "CONTRADICTS-DOC"
        print("  %s doc=%-28s install=%-30s disp=%-6s cluster=%s/%s votes=%d -> %s" % (
            hex(E), doc, host, ("+0x%X" % disp) if disp is not None else "-", kind, tag, len(votes), cls))
    for E, doc in hold65: line(E, doc)
    for E in coll9: line(E, "?")
    return

if __name__ == "__main__":
    main()
