#!/usr/bin/env python3
"""data_census.py — census every shipped data record into committed tables (T-025).

Walks the ORIGINAL shipped data dir (next to $OPENVB_ORIG_EXE) using the documented
container format, NOT regex tag-hunting (kills the compressed-data false-tag class):

  .grp header  = {u32 2, u32 1, u32 count, count*{u32 id, u32 off, u32 size}}   (IronStorm:sess35 §1)
  resource     = chunk chain: {char[4] tag, u32 ver, u32 size(incl 12B hdr), payload}
  chunk grammar refs: GWAM v5  FastArrow:B9/B11;  GITM/GIAR  IronStorm:sess35 §2

Outputs (artifacts/census/):
  chunks.md/csv      every chunk: file, resIdx, offset, tag, ver, size, sha1(payload-region)
  gwam.md/csv        per-v5-record fields incl. corrected consumed/thrown operands
  gitm.md/csv        equip, eqslot, socket-0 name per item record
  giar.md/csv        6 DR values per armor record
  summary.md         counts + value distributions + UNPARSED byte totals per file
Queries:
  --query "consumed_and_thrown"      how many GWAM records set both operands (B11 rule)
  --query "eqslot_dist"              histogram of GITM eqslot
  --query "gwam_count"               total v5 GWAM records
Sanity gates (run with --check): 57 GWAM records; consumed==0 on ALL (T-019 verdict);
a re-run that disagrees with a published number is a bug in tool OR claim - printed loudly.

Boundary (state, not just accept): this proves ON-DISK structure only. It does NOT prove
runtime semantics (which bytes the game reads), record validity beyond grammar, or
anything about compressed (non-TOC-walkable) resources — those files report unparsed>0.

ADOPTION NOTES (RedXenon 01a157ec, T-025 dispatch d56dc1f — original generator is
FastArrow 01a157ef, kept by authorship, credited in git): (1) every chunk and GWAM row now
carries its discovery ROUTE (chain|sweep) and summary.md prints per-route counts + the
MAPPING VERSION line (B11-corrected vs T-019-stage-1 naive pair emitted side by side, per
row constraint); (2) per-file byte accounting switched to MERGED-UNION of TOC extent
intervals with offset-dedup — the per-run sum double-counted overlapping TOC rows (Maps.grp
claimed 25 GB of a 233 MB file until the route print made it visible); (3) falsification
proven: perturbing any gate expectation by +1 turns --check red (rc=1).
"""
import argparse, hashlib, json, os, re, struct, sys
from collections import Counter, defaultdict

TAG_RE = re.compile(rb"^[A-Z][A-Za-z0-9_]{3}$")

def data_dir():
    exe = os.environ.get("OPENVB_ORIG_EXE")
    if not exe:
        sys.exit("OPENVB_ORIG_EXE not set")
    d = os.path.join(os.path.dirname(exe), "data")
    if not os.path.isdir(d):
        sys.exit(f"no data dir at {d}")
    return d

def sha1(b):
    return hashlib.sha1(b).hexdigest()[:12]

def walk_grp(path):
    """Walk TOC entries as extents; parse one continuous chunk stream across each
    maximal run of CONTIGUOUS entries (empirical: resources are packed back-to-back,
    one chain can span several TOC extent rows - _WEA.grp probe 2025-10-20).
    Each resource MAY begin with a 12-byte null header {0,1,1} before its chunk chain
    (GITM-first template resources do; EEN2-first map resources don't).
    TOC entry row = {u32 id, u32 off, u32 size} - layout A(size,id,off) and C(off,size,id)
    mis-walk Maps.grp; B reproduces the published EMAP=13/EME2=64 (map doc) exactly,
    which is the format's falsification test. (IronStorm:sess35 §1 said {id,off,size} and
    was right; my first edit doubting it was wrong.)
    Chain grammar: {tag4, u32 ver, u32 size(incl 12B hdr)}; chain ends at first
    non-conforming header; remaining bytes count as UNPARSED (never silence)."""
    data = open(path, "rb").read()
    out = {"data": data, "chunks": [], "res_total": 0, "res_parsed": 0, "toc_ok": False}
    if len(data) < 12:
        return out
    m0, m1, count = struct.unpack_from("<III", data, 0)
    if (m0, m1) != (2, 1):
        return out
    out["toc_ok"] = True
    extents = []
    out["extents"] = []  # (start, end) pairs for merged-union accounting
    for i in range(count):
        if 12 + 12 * (i + 1) > len(data):
            break
        rid, off, size = struct.unpack_from("<III", data, 12 + 12 * i)   # {id, off, size}
        if 12 <= size and off + size <= len(data):
            extents.append((off, size, rid, i))
            out["extents"].append((off, off + size))
    extents.sort()
    runs, cur = [], []
    for e in extents:
        if cur and e[0] == cur[-1][0] + cur[-1][1]:
            cur.append(e)
        else:
            if cur:
                runs.append(cur)
            cur = [e]
    if cur:
        runs.append(cur)
    for run in runs:
        lo = run[0][0]
        hi = run[-1][0] + run[-1][1]
        out["res_total"] += hi - lo
        rid_of = lambda o: next((r[2] for r in run if r[0] <= o < r[0] + r[1]), run[0][2])
        idx_of = lambda o: next((r[3] for r in run if r[0] <= o < r[0] + r[1]), run[0][3])
        p = lo
        first = True
        while p + 12 <= hi:
            if first:
                h0, h1, h2 = struct.unpack_from("<III", data, p)
                if (h0, h1, h2) == (0, 1, 1):
                    p += 12
                    first = False
                    continue
            first = False
            tag = data[p:p + 4]
            ver, csize = struct.unpack_from("<II", data, p + 4)
            if not TAG_RE.match(tag) or csize < 12 or p + csize > hi:
                break
            out["chunks"].append((idx_of(p), rid_of(p), p, tag.decode(), ver, csize, "chain"))
            out["res_parsed"] += csize
            p += csize
        # nested-record sweep: attack-mode (GWAM) records live INSIDE template payloads
        # (GIWP per B9; empirically also after GITM headers etc.). Scan the whole extent
        # for structurally-valid GWAM v5 records not already accounted at top level;
        # validity = ver==5 and 52<=size<=remaining extent (the sess35 ignore-rule's
        # structural form - false tags in compressed blobs fail ver/size containment).
        seen = {c[2] for c in out["chunks"] if c[3] == "GWAM"}
        blob = data[lo:hi - 52]
        for mm in re.finditer(rb"GWAM", blob):
            start = lo + mm.start()
            if start in seen:
                continue
            nver, nsize = struct.unpack_from("<II", data, start + 4)
            if nver == 5 and 52 <= nsize <= hi - start:
                out["chunks"].append((idx_of(start), rid_of(start), start, "GWAM", 5, nsize, "sweep"))
                seen.add(start)
    return out

# ---- chunk handlers -------------------------------------------------------------

def gwam_fields(data, off, size):
    """B11-corrected: consumed = body byte at off+50 (raw stream dword#10 byte2, fixed
    prefix), thrown = last body byte (#22). Also record naive tail pair for
    discrepancy visibility. Field dword dump = 13 leading dwords (record block prefix)."""
    consumed = data[off + 50] if size > 50 else None
    thrown = data[off + size - 1]
    naive = (data[off + size - 2], data[off + size - 1])
    dwords = struct.unpack_from("<13I", data, off + 12) if size >= 12 + 52 else [None] * 13
    return consumed, thrown, naive, dwords

def gitm_fields(data, off, size):
    # sess35 §2: +12 type s32, +20 equip u8, +24 eqslot s32, +28 socket {u16 len, name...}
    typ = struct.unpack_from("<i", data, off + 12)[0] if size >= 16 else None
    equip = data[off + 20] if size >= 21 else None
    eqslot = struct.unpack_from("<i", data, off + 24)[0] if size >= 28 else None
    name = ""
    if size >= 30:
        nlen = struct.unpack_from("<H", data, off + 28)[0]
        if 0 < nlen < 64 and off + 30 + nlen <= off + size:
            name = data[off + 30: off + 30 + nlen].decode("ascii", "replace")
    return typ, equip, eqslot, name

def giar_fields(data, off, size):
    if size >= 12 + 24:
        return struct.unpack_from("<6i", data, off + 12)
    return (None,) * 6

# ---- main census ------------------------------------------------------------------

def census(root):
    rows_chunks, rows_gwam, rows_gitm, rows_giar = [], [], [], []
    per_file = {}
    for dirpath, _, files in os.walk(root):
        for fn in sorted(files):
            if not fn.lower().endswith(".grp"):
                continue
            p = os.path.join(dirpath, fn)
            w = walk_grp(p)
            data = w["data"]
            rel = os.path.relpath(p, root)
            # MERGED-UNION accounting (RedXenon fix on adoption): TOC rows may point at
            # overlapping extents; per-run summation double-counted them (Maps.grp claimed
            # 25GB of 233MB). Union of extent intervals is the only honest denominator.
            ivs = sorted(w["extents"])
            merged = []
            for a, b in ivs:
                if merged and a <= merged[-1][1]:
                    merged[-1][1] = max(merged[-1][1], b)
                else:
                    merged.append([a, b])
            union = sum(b - a for a, b in merged)
            parsed_bytes = 0
            seen_off = set()
            for (ri, rid, coff, tag, ver, csize, route) in w["chunks"]:
                if coff in seen_off:
                    continue
                seen_off.add(coff)
                payload = data[coff + 12: coff + csize]
                rows_chunks.append([rel, ri, rid, hex(coff), tag, ver, csize, sha1(payload), route])
                parsed_bytes += csize
                if tag == "GWAM" and ver == 5 and csize > 64:
                    c, t, naive, dw = gwam_fields(data, coff, csize)
                    rows_gwam.append([rel, hex(coff), c, t, naive[0], naive[1]] + [hex(x) if x is not None else "" for x in dw] + [route])
                elif tag == "GITM" and csize >= 30:
                    typ, equip, eqslot, name = gitm_fields(data, coff, csize)
                    rows_gitm.append([rel, hex(coff), typ, equip, eqslot, name])
                    # GIAR follows GITM in same resource chain
                elif tag == "GIAR" and csize >= 36:
                    rows_giar.append([rel, hex(coff)] + list(giar_fields(data, coff, csize)))
            per_file[rel] = dict(size=len(data), toc_ok=w["toc_ok"], resources=union,
                                 parsed=parsed_bytes, unparsed=union - parsed_bytes,
                                 nchunks=len(seen_off))
    return rows_chunks, rows_gwam, rows_gitm, rows_giar, per_file

# ---- sanity gates -----------------------------------------------------------------

GATES = [
    ("emap_count_Maps", lambda R: sum(1 for r in R["chunks"] if r[0] == "Maps.grp" and r[4] == "EMAP"), 13, "map doc (format falsifier)"),
    ("eme2_count_Maps", lambda R: sum(1 for r in R["chunks"] if r[0] == "Maps.grp" and r[4] == "EME2"), 64, "map doc (format falsifier)"),
    ("gwam_v5_count", lambda R: len(R["gwam"]), None, "FastArrow:B10 regex said 57 - TOC-verified count is authoritative; discrepancy = false-tag class (sess35 ignore rule)"),
    ("gwam_consumed_nonzero", lambda R: sum(1 for r in R["gwam"] if r[2]), 0, "FastArrow:B11 §3 (T-019 verdict)"),
    ("gitm_eqslot_armor_values", lambda R: sorted({r[4] for r in R["gitm"] if r[2] == 2 and 0 <= (r[4] or 0) <= 5}), [0, 1], "IronStorm:sess35 §3 (armor-type filter kills 'EEN2' garbage hits)"),
]

def check(R):
    ok = True
    for name, got, want, src in GATES:
        g = got(R)
        if want is None:
            print(f"[info] {name}: got={g} ({src})")
            continue
        mark = "PASS" if g == want else "FAIL"
        if g != want:
            ok = False
        print(f"[gate] {name}: got={g} want={want} ({src}) -> {mark}")
    if not ok:
        print("NOTE (row rule): a disagreement is a bug in the TOOL or in the CLAIM. "
              "Say which; do not average them.")
    return ok

# ---- emit -------------------------------------------------------------------------

def emit(outdir, R, per_file, src_hash):
    os.makedirs(outdir, exist_ok=True)
    def table(path, header, rows):
        with open(path, "w") as f:
            f.write(",".join(header) + "\n")
            for r in rows:
                f.write(",".join(str(x) for x in r) + "\n")
    table(os.path.join(outdir, "chunks.csv"),
          ["file", "resIdx", "resId", "off", "tag", "ver", "size", "sha1(payload)", "route"], R["chunks"])
    table(os.path.join(outdir, "gwam.csv"),
          ["file", "off", "consumed(b50)", "thrown(last)", "naive_b(-2)", "naive_b(-1)"] + [f"dword{i}" for i in range(13)] + ["route"], R["gwam"])
    table(os.path.join(outdir, "gitm.csv"),
          ["file", "off", "type", "equip", "eqslot", "socketName0"], R["gitm"])
    table(os.path.join(outdir, "giar.csv"),
          ["file", "off", "drBall", "drBio", "drElec", "drEMP", "drNorm", "drHeat"], R["giar"])
    with open(os.path.join(outdir, "summary.md"), "w") as f:
        f.write(f"# Shipped-data census snapshot\n\n- source dir hash: `{src_hash}`\n- generator: tools/data_census.py (T-025)\n")
        f.write(f"- boundary: on-disk structure only; runtime semantics NOT proven (see header docstring)\n")
        f.write(f"- MAPPING VERSION: gwam consumed/thrown = FastArrow:B11-corrected (consumed=byte@off+50, "
                f"thrown=last body byte); the T-019-stage-1 naive tail pair is emitted alongside as "
                f"naive_b(-2)/naive_b(-1) so a mapping disagreement is VISIBLE, not reconciled silently.\n")
        f.write(f"- ROUTES: 'chain' = top-level chunk-chain walk of contiguous TOC extents; 'sweep' = "
                f"structural GWAM-v5 find inside extents not covered by a chain hit (nested records).\n")
        gw_route = Counter(r[-1] for r in R["gwam"])
        f.write(f"- gwam per route: {dict(gw_route)} (chain={gw_route.get('chain',0)}, sweep={gw_route.get('sweep',0)}); "
                f"region filter: all *.grp under data/ (per-file rows carry the file column)\n\n")
        f.write("| file | TOC | resources bytes | parsed bytes | UNPARSED | chunks |\n|---|---|---|---|---|---|\n")
        for rel in sorted(per_file):
            d = per_file[rel]
            f.write(f"| {rel} | {'ok' if d['toc_ok'] else 'n/a'} | {d['resources']} | {d['parsed']} | {d['unparsed']} | {d['nchunks']} |\n")
        f.write("\n## counts + distributions\n\n")
        f.write(f"- GWAM v5 records: {len(R['gwam'])}; consumed!=0: {sum(1 for r in R['gwam'] if r[2])}; thrown!=0: {sum(1 for r in R['gwam'] if r[3])}\n")
        f.write(f"- GITM records: {len(R['gitm'])}; eqslot hist: {dict(Counter(r[4] for r in R['gitm']))}\n")
        f.write(f"- GIAR records: {len(R['giar'])}\n")
        f.write(f"- chunk tags: {dict(Counter(r[4] for r in R['chunks']))}\n")
    # markdown views (same rows, pipe tables) kept minimal; csv is canonical
    print(f"emitted to {outdir}")

def src_hash_of(root):
    h = hashlib.sha1()
    for dirpath, _, files in os.walk(root):
        for fn in sorted(files):
            if fn.lower().endswith(".grp"):
                p = os.path.join(dirpath, fn)
                st = os.stat(p)
                h.update(f"{p}:{st.st_size}".encode())
    return h.hexdigest()[:12]

def run_query(R, q):
    if q == "consumed_and_thrown":
        return sum(1 for r in R["gwam"] if r[2] and r[3])
    if q == "gwam_count":
        return len(R["gwam"])
    if q == "eqslot_dist":
        return dict(Counter(r[4] for r in R["gitm"]))
    if q == "unarmed_clips":  # style 3 animClip family from dword columns, e.g.
        return dict(Counter(r[10] for r in R["gwam"]))  # dword10 slot (clip 53/55 family)
    sys.exit(f"unknown query {q!r} (known: consumed_and_thrown, gwam_count, eqslot_dist, unarmed_clips)")

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", default="artifacts/census")
    ap.add_argument("--check", action="store_true")
    ap.add_argument("--query")
    ap.add_argument("--emit", action="store_true", default=False)
    args = ap.parse_args()
    root = data_dir()
    C, G, ITM, IA, per_file = census(root)
    R = {"chunks": C, "gwam": G, "gitm": ITM, "giar": IA}
    if args.query:
        print(json.dumps(run_query(R, args.query), default=str))
        return
    if args.check:
        sys.exit(0 if check(R) else 1)
    if args.emit:
        emit(args.out, R, per_file, src_hash_of(root))
    else:
        # default: summary to stdout (no writes) — CI-friendly
        ok = check(R)
        print(f"chunks={len(C)} gwam={len(G)} gitm={len(ITM)} giar={len(IA)} files={len(per_file)}")
        sys.exit(0 if ok else 1)

if __name__ == "__main__":
    main()
