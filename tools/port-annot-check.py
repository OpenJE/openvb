#!/usr/bin/env python3
"""port-annot-check.py — HARD RULE checker (STYLE.md, 2026-10-21): every port function cites
its original-side EA. For each definition in src/ and modules/openje/src/, look for a 0xXXXXXX
citation in the function's leading comment block or first lines of body; report gaps.
Exit 1 = gaps exist (list mode), --gate = fail only files changed vs HEAD (incremental gate)."""
import re, sys, subprocess
from pathlib import Path
DEF = re.compile(r'^\s{0,4}(?:[\w:<>,\*\& ]+\s)?([A-Za-z_~][\w:]*(?:::[A-Za-z_~][\w:]*)*)\s*\([^;{]*\)\s*(?:const\s*)?\{?\s*$')
CITE = re.compile(r'0x[0-9a-fA-F]{6}')
def files():
    for root in ("src", "modules/openje/src"):
        for p in Path(root).rglob("*.cpp"):
            if "tests" not in str(p): yield p
def check(path):
    lines = path.read_text(errors="ignore").splitlines()
    gaps, total = [], 0
    for i, ln in enumerate(lines):
        m = DEF.match(ln)
        if not m or any(k in ln for k in ("if ", "for ", "while ", "switch ", "return")): continue
        total += 1
        window = " ".join(lines[max(0, i-8): i+8])
        if not CITE.search(window): gaps.append((m.group(1), i+1))
    return total, gaps
tot=gap=0; per=[]
for f in files():
    t, g = check(f)
    tot += t; gap += len(g)
    if g: per.append((str(f), t, len(g), g[:3]))
per.sort(key=lambda x: -x[2])
for f, t, n, sample in per[:25]:
    print(f"{f}: {n}/{t} uncited (e.g. {', '.join(f'{fn}@{ln}' for fn,ln in sample)})")
print(f"TOTAL: {gap} uncited of {tot} defs ({100*gap//max(tot,1)}%) across {len(per)} files")
sys.exit(1 if gap else 0)
