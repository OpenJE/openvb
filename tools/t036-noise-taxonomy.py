#!/usr/bin/env python3
"""T-036 helper-theft noise taxonomy -- re-measurable census audit of anonymous-class attribution.

Answers one question: how much of tier-2 ("typed stub") attribution is real class mass and how much is an
artifact of how IDA and the F3.json export group code? Prints BOTH join routes for every count, because the
headline finding is that the number depends on the route (mixing them is what produced the bogus "998 noise").

Routes
  A = IDB function names   : $OPENVB_IDA_FUNCS_JSON  (default /tmp/census_p*.json, the T-029 list_funcs dump)
  B = artifacts/F3.json    : structures[cls_0x...].methods
  C = shipped F3.exe bytes : $OPENVB_ORIG_EXE        (MD5 must equal F3.json "filemd5")

Usage:  python3 tools/t036-noise-taxonomy.py [--class cls_0x49c610] [--json]
Exit:   0 ok, 2 evidence unavailable (binary MD5 mismatch / no route-A dump).
"""
import argparse
import bisect
import hashlib
import json
import os
import re
import statistics
import struct
import sys
from collections import Counter, defaultdict

GAME_BOUNDARY = 0x5FD000      # T-029 region split: below = game text, at/above = CRT/D3DX link tail
WIN = 0x2000                  # batch-1 contiguity window
CLONE_THR = 0.98              # >=0.90 measured 3.83% false-positive on unrelated same-size pairs; 0.98 -> 0.20%
CLUSTER_W = 0x1000
REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def die(msg, code=2):
    print("ERROR: %s" % msg, file=sys.stderr)
    sys.exit(code)


# ---------------------------------------------------------------- route A: IDB function list
def load_route_a(paths):
    funcs = []
    for p in paths:
        d = json.load(open(p))
        pages = d if isinstance(d, list) else [d]
        for pg in pages:
            if isinstance(pg, dict) and "data" in pg:
                funcs.extend(pg["data"])
    info = {}
    for f in funcs:
        a = int(f["addr"], 16)
        sz = int(f["size"], 16) if isinstance(f["size"], str) else int(f["size"])
        info[a] = (f["name"], sz)
    if len(funcs) != len(info):
        print("  (route A: %d items collapse to %d unique addresses -- duplicate starts in the dump)"
              % (len(funcs), len(info)))
    return info


# ---------------------------------------------------------------- route C: raw PE bytes
class PE(object):
    def __init__(self, path):
        self.data = open(path, "rb").read()
        nt = struct.unpack_from("<I", self.data, 0x3C)[0]
        if self.data[nt:nt + 4] != b"PE\0\0":
            die("%s is not a PE" % path)
        nsec = struct.unpack_from("<H", self.data, nt + 6)[0]
        sizeopt = struct.unpack_from("<H", self.data, nt + 20)[0]
        o = nt + 24 + sizeopt
        self.secs = []
        for _ in range(nsec):
            nm = self.data[o:o + 8].rstrip(b"\0").decode("latin1")
            vsz, va, rsz, praw = struct.unpack_from("<IIII", self.data, o + 8)
            self.secs.append((nm, 0x400000 + va, vsz, praw, rsz))
            o += 40

    def read(self, va, length):
        for _nm, sva, vsz, praw, rsz in self.secs:
            if sva <= va < sva + vsz:
                off = praw + (va - sva)
                n = min(length, rsz - (va - sva), len(self.data) - off)
                return self.data[off:off + n] if n > 0 else b""
        return b""


# ---------------------------------------------------------------- helpers
def norm_ops(b):
    """Zero 32-bit immediates/displacements and rel32 targets: clone families differ only there."""
    out = bytearray(b)
    i = 0
    while i < len(b):
        op = b[i]
        if op in (0x68, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF) and i + 5 <= len(b):
            out[i + 1:i + 5] = b"\0\0\0\0"
            i += 5
            continue
        if op in (0xE8, 0xE9) and i + 5 <= len(b):
            out[i + 1:i + 5] = b"\0\0\0\0"
            i += 5
            continue
        if op in (0x8D, 0x8B, 0x89, 0x88, 0x8A) and i + 1 < len(b) and (b[i + 1] >> 6) == 2 and i + 6 <= len(b):
            out[i + 2:i + 6] = b"\0\0\0\0"
            i += 6
            continue
        i += 1
    return bytes(out)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--f3json", default=os.path.join(REPO, "artifacts", "F3.json"))
    ap.add_argument("--exe", default=os.environ.get("OPENVB_ORIG_EXE"))
    ap.add_argument("--funcs", default=None, help="route-A dump (glob or dir); default /tmp/census_p*.json")
    ap.add_argument("--class", dest="one", help="trace one anon class end to end")
    ap.add_argument("--json", action="store_true")
    a = ap.parse_args()

    db = json.load(open(a.f3json))
    structs = db["structures"]

    want = db.get("filemd5", "").lower()

    pat = a.funcs or "/tmp/census_p*.json"
    import glob as _glob
    paths = sorted(_glob.glob(pat)) if any(c in pat for c in "*?[") else (
        sorted(_glob.glob(os.path.join(pat, "census_p*.json"))) if os.path.isdir(pat) else [pat])
    paths = [p for p in paths if os.path.exists(p)]
    have_route_a = bool(paths)

    info = load_route_a(paths) if have_route_a else {}
    starts = sorted(info)
    bodies = {}
    exe_ok = False
    if a.exe and os.path.exists(a.exe):
        pe = PE(a.exe)
        got = hashlib.md5(pe.data).hexdigest()
        exe_ok = (not want) or (got == want)
        if not exe_ok:
            print("WARNING: %s MD5 %s != F3.json filemd5 %s -- byte evidence disabled" % (a.exe, got, want))
        else:
            bodies = {k: pe.read(k, v[1]) for k, v in info.items() if v[1] > 0}
            bodies = {k: v for k, v in bodies.items() if len(v) == info[k][1]}
    else:
        print("WARNING: no $OPENVB_ORIG_EXE -- route C (byte clone families) unavailable")

    def containing(ea):
        i = bisect.bisect_right(starts, ea) - 1
        if i < 0 or ea >= starts[i] + info[starts[i]][1]:
            return None
        return starts[i]

    def idb_owner(n):
        """Owner class implied by an IDB function name; '' when the name carries none (sub_/plain)."""
        if n.startswith("cls_0x"):
            return ""
        p = n.split("::")
        if len(p) >= 3:
            return p[0] + "::" + p[1]
        if len(p) >= 2:
            return p[0]
        return ""

    def ident(x, y):
        bx, by = bodies.get(x), bodies.get(y)
        if not bx or not by or len(bx) != len(by):
            return 0.0
        return sum(1 for u, v in zip(bx, by) if u == v) / len(bx)

    anon = {k: v for k, v in structs.items() if re.match(r"^cls_0x[0-9a-fA-F]+$", k)}
    embedders = defaultdict(set)
    for k, v in structs.items():
        for _off, m in v.get("members", {}).items():
            if m.get("struc"):
                embedders[m["struc"]].add(k)

    # route A: functions whose IDB name is cls_0x...::
    route_a = {x for x in starts
               if x < GAME_BOUNDARY and info[x][0].startswith("cls_0x") and "::" in info[x][0]}
    # route B: DB method EAs of anon structs
    b_all = {int(e, 16) for v in anon.values() for e in v.get("methods", {})}
    b_start = {e for e in b_all if e in info}
    b_game = {e for e in b_start if e < GAME_BOUNDARY}
    nonstart = b_all - b_start
    conflict = {x for x in b_game if not info[x][0].startswith("cls_0x")}
    named_conf = {x for x in conflict if re.match(r"^(JE|F3|net)::", info[x][0])}
    tier1 = {x for x in starts if x < GAME_BOUNDARY and (
        re.match(r"^(JE|F3|net)::", info[x][0]) or re.match(r"^(JE_|F3_)", info[x][0]))}

    fam = defaultdict(list)
    if bodies:
        for k, v in info.items():
            if k < GAME_BOUNDARY and k in bodies:
                fam[hashlib.sha1(norm_ops(bodies[k])).hexdigest()].append(k)
    dupfam = {h: sorted(v) for h, v in fam.items() if len(v) > 1}

    # ---- per class categories
    recs = {}
    cat = Counter()
    for k, v in sorted(anon.items()):
        es = sorted(int(e, 16) for e in v.get("methods", {}) if int(e, 16) in info)
        if not es:
            continue
        us = set()
        for _off, m in v.get("members", {}).items():
            for u in m.get("usages", []):
                c = containing(int(u, 16))
                if c:
                    us.add(c)
        vt = {int(e["ea"], 16) for t in v.get("vftables", {}).values() for e in t.get("entries", {}).values()}
        med = statistics.median(es)
        ratio = sum(1 for x in es if abs(x - med) <= WIN) / len(es)
        c_local = Counter()
        for x in es:
            n = info[x][0]
            sib = bool(bodies) and x in bodies and len(
                fam.get(hashlib.sha1(norm_ops(bodies[x])).hexdigest(), [])) > 1
            if x >= GAME_BOUNDARY:
                c = "2-off-region(link-tail)"
            elif idb_owner(n):
                c = "3a-conflict(IDB-names-a-real-owner)"
            elif not n.startswith("cls_0x"):
                c = "3b-conflict(IDB-leaves-it-unnamed)"
            elif sib:
                c = "4-clone-family"
            elif (x in us) or (x in vt):
                c = "5-evidenced-method"
            else:
                c = "6-unproven(check-prototype)"
            c_local[c] += 1
            cat[c] += 1
        recs[k] = dict(n=len(es), ratio=ratio, med=int(med), es=es, cnt=dict(c_local),
                       ev=sum(1 for x in es if x in us or x in vt),
                       ev_u=sum(1 for x in es if x in us),
                       nemb=len(embedders.get(k, ())), size=v.get("size"))

    N = sum(cat.values())
    noise = (cat["2-off-region(link-tail)"]
             + cat["3a-conflict(IDB-names-a-real-owner)"]
             + cat["3b-conflict(IDB-leaves-it-unnamed)"] + len(nonstart))

    # ---- rank correlation contiguity vs evidence (the falsifiable claim)
    def spearman(xs, ys):
        def rk(v):
            o = sorted(range(len(v)), key=lambda i: v[i])
            r = [0] * len(v)
            for i, x in enumerate(o):
                r[x] = i
            return r
        rx, ry = rk(xs), rk(ys)
        mx, my = sum(rx) / len(rx), sum(ry) / len(ry)
        num = sum((p - mx) * (q - my) for p, q in zip(rx, ry))
        den = (sum((p - mx) ** 2 for p in rx) * sum((q - my) ** 2 for q in ry)) ** .5
        return num / den if den else 0.0

    big = [r for r in recs.values() if r["n"] >= 8]
    rho = spearman([r["ratio"] for r in big], [r["ev"] / r["n"] for r in big]) if len(big) > 3 else float("nan")
    rho_u = spearman([r["ratio"] for r in big], [r["ev_u"] / r["n"] for r in big]) if len(big) > 3 else float("nan")

    def gate(r, k):
        cl = r["cnt"].get("4-clone-family", 0)
        cf = r["cnt"].get("3a-conflict(IDB-names-a-real-owner)", 0)
        cfu = r["cnt"].get("3b-conflict(IDB-leaves-it-unnamed)", 0)
        if r["n"] >= 3 and cf >= max(1, int(0.8 * r["n"])) and cfu == 0:
            own = Counter(idb_owner(info[x][0])
                          for x in r["es"] if idb_owner(info[x][0]))
            top = own.most_common(1)
            return "R36-b RETIRE: IDB already names %d/%d%s (every member named, none unnamed)" % (
                cf, r["n"], " as " + top[0][0] if top else "")
        if r["n"] < 8:
            return "R36-c ratio is VACUOUS at n<8 -- never rank this class on contiguity"
        if r["nemb"] >= 3:
            return "R36-e composite: report as embedded struct + host band, not one class"
        if cl >= 0.5 * r["n"]:
            return "R36-d clone family: N bodies for one source method"
        if r["ratio"] >= 0.5 and r["ev"] >= 0.6 * r["n"]:
            return "OK rename candidate (dense and evidenced)"
        if r["ratio"] < 0.4 and r["n"] >= 40:
            return "R36-f low density but large: check prototypes + xref, density never vetoes"
        return "weak signal"

    after = b_game - conflict - tier1

    rep = dict(routes=dict(route_a=len(route_a), route_b_start=len(b_start), route_b_game=len(b_game),
                           db_declared=len(b_all), nonstart=len(nonstart), conflict=len(conflict),
                           named_conflict=len(named_conf), tier1=len(tier1)),
               taxonomy=dict(cat), attributed=N, noise_floor=noise, rho=rho, rho_usage_only=rho_u,
               after_rule=len(after))
    if a.json:
        print(json.dumps(rep, indent=1))
    else:
        print("=== T-036 noise taxonomy ===")
        print("routes: A(IDB cls_ names, game)=%d  B(DB methods that are IDB starts)=%d  B game-only=%d"
              % (len(route_a), len(b_start), len(b_game)))
        print("        DB-claimed EAs that are NOT function starts=%d  route conflict=%d (named JE/F3/net=%d)"
              % (len(nonstart), len(conflict), len(named_conf)))
        print("        route A and B overlap=%d  (in A not B=%d, in B not A=%d)"
              % (len(route_a & b_game), len(route_a - b_game), len(b_game - route_a)))
        print("\ncategories over %d attributed functions:" % N)
        for c in sorted(cat):
            print("  %-32s %5d  %5.1f%%" % (c, cat[c], 100 * cat[c] / N))
        print("\n  ARTIFACT FLOOR = off-region + route-conflict + non-start = %d" % noise)
        print("  batch-1 claimed 998 helper-theft noise; measured %d -> %.0f fewer were real methods"
              % (noise, 998 - noise))
        print("\n  rho(contiguity, evidence) over the %d classes with n>=8:" % len(big))
        print("    usage-only            = %+.3f" % rho_u)
        print("    usage OR vftable slot = %+.3f" % rho)
        hi = [r for r in recs.values() if r["ratio"] >= 0.5]
        vac = sum(1 for r in hi if r["n"] <= 2)
        print("  VACUOUS: %d of the %d classes with ratio>=0.5 hold <=2 functions, where ratio==1.00 by"
              % (vac, len(hi)))
        print("           construction. Contiguity neither ranks those nor tracks attribution quality;")
        print("           it tracks where the linker placed the code.")
        print("\ncensus re-run under R36-a..c: attributed %d (%.1f%% of game 8,876); delta vs A %+d, vs B %+d"
              % (len(after), 100 * len(after) / 8876, len(after) - len(route_a), len(after) - len(b_game)))
        print("\nbatch-1 headline candidates through the R36 gate:")
        for k in ["cls_0x4d8d70", "cls_0x50dbb0", "cls_0x55dc80", "cls_0x50db20", "cls_0x424a70",
                  "cls_0x4542d0", "cls_0x668cd8", "cls_0x49c610", "cls_0x6bb908", "cls_0x4b28a0"]:
            if k in recs:
                r = recs[k]
                print("  %-14s n=%3d ratio=%.2f ev=%3.0f%% embed=%2d  %s"
                      % (k, r["n"], r["ratio"], 100 * r["ev"] / r["n"], r["nemb"], gate(r, k)))
        print("\ngate distribution over classes with n>=8:")
        for x, c in Counter(gate(r, k) for k, r in recs.items() if r["n"] >= 8).most_common():
            print("  %-74s %d" % (x, c))
    if a.one:
        if a.one not in recs:
            die("class %s not found among anon classes" % a.one, 1)
        r = recs[a.one]
        v = anon[a.one]
        print("\n=== trace %s ===" % a.one)
        print("size=%s members=%d vftables=%d embedders=%d ratio=%.2f" % (
            r["size"], len(v.get("members", {})), len(v.get("vftables", {})), r["nemb"], r["ratio"]))
        print("category counts: %s" % r["cnt"])
        print("gate: %s" % gate(r, a.one))
        print("IDB owner of its functions:")
        own = Counter(idb_owner(info[x][0]) or ("cls_0x-anon" if info[x][0].startswith("cls_0x")
                                                else "(no owner in name)") for x in r["es"])
        for o, c in own.most_common(8):
            print("   %-40s %d" % (o, c))
        print("sample functions:")
        for x in r["es"][:12]:
            sib = [y for y in r["es"] if y != x and ident(x, y) >= CLONE_THR]
            print("   0x%06x size=0x%-6x clone-of=%s  %s"
                  % (x, info[x][1], ",".join(hex(y) for y in sib[:3]) or "-", info[x][0][:50]))
    if not have_route_a and not exe_ok:
        die("no route-A dump and no verified executable: nothing measurable")


if __name__ == "__main__":
    main()
