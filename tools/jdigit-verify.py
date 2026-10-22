#!/usr/bin/env python3
"""jdigit-verify.py — R-HEADERSAN per-site verifier (READ-ONLY; renames stay human-reviewed).

The 2024-export-era openje headers carry ~48 names whose EA/mask embeds a literal 'j' (not a hex
digit): virt_meth_0x4aje00, mbr_0xje0, cls_0x5debje. Every such name is a LIE about an address.
This tool does NOT assume a uniform flip. Per site it substitutes the j with each hex digit and
queries the F3.json truth set (methods, vftable entries, member offsets, struct keys), classifying
CONFIRMED (exactly one candidate IN THE OWNING CLASS) / WRONG-OWNER (candidates exist, but only
under other classes) / AMBIG / NONE. Load is O(json once) - no bulk scans (safety law).

usage:
  tools/jdigit-verify.py                 # report all sites, per-file, with verdicts
  tools/jdigit-verify.py --only <name>   # single site deep-dive (candidate set shown)
  tools/jdigit-verify.py --selftest      # synthetic truth table, all four verdicts exercised
"""
import json, re, sys, glob
from pathlib import Path

REPO = Path(__file__).resolve().parent.parent
JRE  = re.compile(r'\b([A-Za-z_][A-Za-z0-9_]*_0x)([0-9a-fA-F]*j[0-9a-fA-F]*)\b')

def load_truth():
    d = json.loads((REPO / "artifacts" / "F3.json").read_text())
    st = d["structures"]
    methods, vft_eas, members, names = {}, {}, {}, set()
    for key, s in st.items():
        demo = s.get("demangled_name") or key
        names.add(key); names.add(demo)
        for ea in (s.get("methods") or {}):
            methods.setdefault(ea, []).append(key)
        for _v in (s.get("vftables") or {}).values():
            for e in (_v.get("entries") or {}).values():
                vft_eas.setdefault(e["ea"], []).append(key)
        for off in (s.get("members") or {}):
            members.setdefault(demo, set()).add(off)
            members.setdefault(key, set()).add(off)
    return st, methods, vft_eas, members, names

def candidates(hexpart):
    """Every single-hex substitution of the (first) j, lower-case EAs. Names with TWO j's
    get the j x j product (<=256) - still bounded; multi-j truth needs BOTH slots to fit."""
    low = hexpart.lower()
    if low.count('j') == 0:
        return []
    if low.count('j') == 1:
        i = low.index('j')
        out = list(dict.fromkeys("0x" + low[:i] + h + low[i+1:] for h in "0123456789abcdef"))
        if "je" in low:   # CORRUPTION LAW 2026-10-22: 'je' is the export artifact of 'f3'
                         # (Window vft slots 35-39 run; Edit/AreaMapWindow f30-f3c member
                         # sandwich between f2c and f40). Broadens the CANDIDATE SET only -
                         # the per-site unique-in-owner rule still does the confirming.
            out.append("0x" + low.replace("je", "f3", 1))
        return out
    out = []
    for h1 in "0123456789abcdef":
        s1 = low[:low.index('j')] + h1 + low[low.index('j')+1:]
        for h2 in "0123456789abcdef":
            i2 = s1.index('j')
            out.append("0x" + s1[:i2] + h2 + s1[i2+1:])
    out = list(dict.fromkeys(out))
    # CORRUPTION LAW (per-site verified before any use, 2026-10-22): 'je' is the export artifact
    # of 'f3' - five Window sites map to ITS OWN vft slots 35-39 and both mbr families land in the
    # DB's f30-f3c run between f2c and f40. Added to the candidate SET; the per-site verdicts
    # (unique-in-owner etc.) still do the confirming - this broadens candidates, never the trust.
    if "je" in low:
        out.append("0x" + low.replace("je", "f3", 1))
    return out

def owner_key(path, text, st, names):
    """Best owning-struct key: cls_0xNNNN file base, else the class/struct whose NAME matches
    the file stem (declared or forward), else the FIRST DECLARED (brace-bodied) type in-file.
    Forward-decls of OTHER classes must never win - an early owner bug here proposed
    'Entity' as owner of RLECircle.hpp, the exact false-confirm the row exists to forbid."""
    m = re.match(r'cls_0x([0-9a-fA-F]+)', path.stem)
    if m and ("cls_0x" + m.group(1).lower()) in names:
        return "cls_0x" + m.group(1).lower()
    stem = path.stem
    # exact stem match first (demangled ending in the stem, or key ending in it)
    for k, s in st.items():
        if (s.get("demangled_name") or "").split("::")[-1] == stem or k.endswith(stem):
            return k
    # template/mangled fallback: stem appears as a WHOLE token inside the mangled/demangled
    # key (boundary-checked so 'World' can never lock onto 'GameWorld' mangling)
    bnd = re.compile(r'(?<![A-Za-z0-9_])' + re.escape(stem) + r'(?![A-Za-z0-9_])')
    for k, s in st.items():
        if bnd.search(k) or bnd.search(s.get("demangled_name") or ""):
            return k
    # else: first 'class|struct NAME {' DECLARED in this file (no semicolon = not a fwd decl)
    for cm in re.finditer(r'\b(?:class|struct)\s+([A-Za-z_][A-Za-z0-9_]*)\s*[^;{]*\{', text):
        nm = cm.group(1)
        for k, s in st.items():
            if (s.get("demangled_name") or "").split("::")[-1] == nm:
                return k
        if nm in names:
            return nm
    return None

def code_lines(text):
    return [ln.split("//")[0] for ln in text.splitlines()]

ANCH = re.compile(r'\b(mbr|cls|virt_meth|meth|ctor|dtor|deldtor|operator)_0x([0-9a-fA-F]{2,})')

def anchor_vals(code_of, skip_line, kind):
    """Same-KIND ascending anchors only - the member block and the method block are each
    sorted, but a max/min across BOTH blocks is meaningless (v1 bug: a method anchor at
    0x4abc60 was 'bracketing' a MEMBER site). One file, one kind, one sandwich."""
    vals = []
    for idx, cl in enumerate(code_of):
        if idx == skip_line:
            vals.append(None); continue
        v = None
        for mm in ANCH.finditer(cl):
            fam = "mbr" if mm.group(1) == "mbr" else "meth"
            if fam == kind:
                v = int(mm.group(2), 16); break
        vals.append(v)
    return vals

def sandwich(code_of, line, kind):
    """Neighbor brackets, not global min/max: a sorted block's positional fingerprint is the
    value on each ADJACENT same-kind line. Global brackets assumed numeric ordering, but
    Edit.hpp sorts LEXICOGRAPHICALLY (mbr_0x1000 sits right after mbr_0x10) - there the global
    bracket came out lo=0x1038 > hi=0xf40 and VETOED the correct dedup winner. Neighbors are
    order-agnostic; if they still disagree (lo>hi), the bracket is simply not applied."""
    vals = anchor_vals(code_of, line, kind)
    lo = next((x for x in reversed(vals[:line]) if x is not None), None)
    hi = next((x for x in vals[line+1:] if x is not None), None)
    if lo is not None and hi is not None and lo > hi:
        lo = hi = None          # not a monotone block: no positional claim
    declared = {"0x%x" % x for x in vals if x is not None}
    return (lo, hi, declared)

def classify(full_name, hexpart, owner, st, methods, vft_eas, members, names, ctx=None):
    cands = candidates(hexpart)
    if full_name.startswith("cls_"):
        hits = [c for c in cands if c[2:] in names or c in names
                or any(k.lower().endswith(c.lower()) for k in names)]
    elif full_name.startswith("mbr_"):
        hits = [c for c in cands if owner and c in (members.get(owner) or set())]
    else:
        hits = [c for c in cands if c in methods or c in vft_eas]
        invf = [c for c in hits if owner and (c in vft_eas and owner in vft_eas[c]
                or c in methods and owner in methods[c])]
        if len(hits) == 1 and not invf:
            return "WRONG-OWNER", cands, hits
        if len(invf) == 1:
            return "CONFIRMED", cands, invf
    if len(hits) == 1:
        return "CONFIRMED", cands, hits
    if len(hits) > 1 and ctx:
        # CONTEXT TIE-BREAKS (per-site, not blanket): (1) these decl lists are ASCENDING - an
        # ordered sandwich lo < c < hi is a positional fingerprint; (2) a candidate offset/name
        # ALREADY DECLARED in the same file cannot be the same line's identity (duplicate).
        lo, hi, declared = ctx
        keep = [c for c in hits if not (declared and c in declared)]
        if lo is not None and hi is not None:
            keep = [c for c in (keep or hits) if lo <= int(c, 16) <= hi]
        if len(keep) == 1:
            return "CONFIRMED", cands, keep
    return ("AMBIG", cands, hits) if hits else ("NONE", cands, [])

def selftest():
    """Synthetic truth table exercising every verdict path - a checker without a selftest is
    a docstring pretending to be a tool (this lane's own port-annot lesson)."""
    fails = []
    methods = {"0x4af1e0": ["own"], "0x4abe00": ["other"], "0x99ffee": ["other"]}
    vft_eas = {"0x4af1e0": ["own"]}
    members = {"own": {"0x1e0", "0x1e4"}}
    names = {"own", "other", "cls_0x4beeb0"}
    st = {"own": {}}
    def cls(full, hx, owner, ctx=None):
        return classify(full, hx, owner, st, methods, vft_eas, members, names, ctx)
    # S7: owner resolution must prefer the file-stem class and NEVER a first-seen forward decl
    st7 = {"OCTREE_Ring2D": {"demangled_name": "OCTREE_Ring2D"},
           "Entity": {"demangled_name": "Entity"}}
    from pathlib import Path as P7
    txt7 = ("struct Entity;\n"
            "namespace JE { namespace OCTREE { class Ring2D { int mbr_0x0; }; } }\n")
    ok7 = owner_key(P7("/x/OCTREE_Ring2D.hpp"), txt7, st7, {"OCTREE_Ring2D", "Entity"})
    if ok7 != "OCTREE_Ring2D": fails.append(f"S7 owner={ok7}")
    v, _, hits = cls("virt_meth_0x4afje0", "4afje0", "own")
    if not (v == "CONFIRMED" and hits == ["0x4af1e0"]): fails.append(f"S1 {v} {hits}")
    v, _, hits = cls("meth_0x45je00", "45je00", "own")
    if v != "NONE": fails.append(f"S2 {v}")
    v, _, hits = cls("virt_meth_0x4aje00", "4aje00", "own")
    if v != "WRONG-OWNER": fails.append(f"S3 {v} {hits}")
    methods["0x4a0e00"] = ["own"]
    v, _, hits = cls("virt_meth_0x4aje00", "4aje00", "own")
    if not (v == "CONFIRMED" and hits == ["0x4a0e00"]): fails.append(f"S4a {v} {hits}")
    # owner-context tiebreak: two DB candidates, exactly one in the OWNER class -> CONFIRMED
    # with the owner hit only (the intended semantics of confirm-per-site).
    v, _, _ = cls("mbr_0xje0", "je0", "own")
    if v != "CONFIRMED": fails.append(f"S5 member {v}")
    v, _, hits = cls("cls_0x4beej0", "4beej0", "own")
    if not v in ("CONFIRMED", "AMBIG"): fails.append(f"S6 cls-sweep {v}")
    # S8: the corruption law discovered 2026-10-22 (Window vft slots 35-39 run + Edit/
    # AreaMapWindow f30-f3c member sandwich): 'je' is the export artifact of 'f3'.
    methods["0x58f3e0"] = ["own"]
    v, _, hits = cls("virt_meth_0x58jee0", "58jee0", "own")
    if not (v == "CONFIRMED" and hits == ["0x58f3e0"]): fails.append(f"S8 je-f3 {v} {hits}")
    # S10: ordered-sandwich tiebreak - two owner candidates, only one inside (lo,hi)
    methods["0x44f340"] = ["own"]; methods["0x44f9e0"] = ["own"]
    v, _, hits = cls("virt_meth_0x44fj40", "44fj40", "own", (0x44f000, 0x44f500, set()))
    if not (v == "CONFIRMED" and hits == ["0x44f340"]): fails.append(f"S10 sandwich {v} {hits}")
    # S11: duplicate-exclusion tiebreak - candidates BOTH under owner, one already declared
    # in the same file -> the other wins
    methods["0x44fee0"] = ["own"]
    v, _, hits = cls("meth_0x44fje0", "44fje0", "own", (None, None, {"0x44f9e0"}))
    if not (v == "CONFIRMED" and hits == ["0x44fee0"]): fails.append(f"S11 dup {v} {hits}")
    # S12: anchors must stay SAME-KIND and NEIGHBOR-based (lexicographic files like Edit.hpp
    # would give a nonsense global bracket; the adjacent lines are the positional truth)
    co = ["dword mbr_0xf2c;", "dword mbr_0xje0;", "undefined4 meth_0x4abc60();",
          "dword mbr_0xf40;"]
    lo, hi, dec = sandwich(co, 1, "mbr")
    if not (lo == 0xf2c and hi == 0xf40): fails.append(f"S12 lo={lo and hex(lo)} hi={hi and hex(hi)}")
    co12b = ["dword mbr_0x10;", "dword mbr_0x1000;", "dword mbr_0xf2c;", "dword mbr_0xje0;",
             "dword mbr_0xf40;"]
    lo2, hi2, dec2 = sandwich(co12b, 3, "mbr")
    if not (lo2 == 0xf2c and hi2 == 0xf40): fails.append(f"S12c lex-neighbors lo={hex(lo2)} hi={hex(hi2)}")
    v, _, hits = classify("mbr_0xje0", "je0", "own", st, methods, vft_eas,
                     {"own": {"0xf30", "0xde0", "0xee0"}}, names, (lo, hi, dec))
    if not (v == "CONFIRMED" and hits == ["0xf30"]): fails.append(f"S12b {v} {hits}")
    # S9: owner resolution must find TEMPLATE classes by substring (the RLECircle<Entity*> site
    # was orphaned to WRONG-OWNER until owner_key could see '.?AV?$RLECircle@...' keys).
    st9 = {".?AV?$RLECircle@PAUEntity@@@@": {"demangled_name": "$RLECircle<Entity *>"}}
    ok9 = owner_key(P7("/x/RLECircle.hpp"), "class RLECircle { };", st9, set(st9))
    if ok9 != ".?AV?$RLECircle@PAUEntity@@@@": fails.append(f"S9 template owner={ok9}")
    print("SELFTEST " + ("12/12 PASS" if not fails else "FAIL: " + "; ".join(fails)))
    return 0 if not fails else 1

def main():
    if "--selftest" in sys.argv:
        sys.exit(selftest())
    only = sys.argv[sys.argv.index("--only")+1] if "--only" in sys.argv else None
    st, methods, vft_eas, members, names = load_truth()
    files = sorted(glob.glob(str(REPO / "modules/openje/src/**/*.hpp"), recursive=True))
    report, counts = {}, {"CONFIRMED": 0, "WRONG-OWNER": 0, "AMBIG": 0, "NONE": 0}
    for f in files:
        p = Path(f); text = p.read_text(errors="ignore")
        owner = owner_key(p, text, st, names)
        code_of = code_lines(text)
        for line, cl in enumerate(code_of):
            for m in JRE.finditer(cl):
                base, hexpart = m.group(1), m.group(2)
                full = base + hexpart
                if only and only not in full and only != p.name:
                    continue
                kind = "mbr" if base.startswith("mbr_") else "meth"
                ctx = sandwich(code_of, line, kind)
                v, cands, hits = classify(full, hexpart, owner, st, methods, vft_eas, members,
                                          names, ctx)
                counts[v] += 1
                report.setdefault(str(p.relative_to(REPO)), []).append((line+1, full, owner, v, hits))
    for f, sites in sorted(report.items()):
        print(f"\n== {f}")
        for line, full, owner, v, hits in sites:
            print(f"  L{line:<5} {full:<28} owner={owner or '?':<28} {v:<12} {','.join(hits) or '-'}")
    print(f"\nTOTAL sites: {sum(counts.values())} -> {counts}")
    sys.exit(0)

if __name__ == "__main__":
    main()
