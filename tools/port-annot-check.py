#!/usr/bin/env python3
"""port-annot-check.py — HARD RULE checker (STYLE.md, 2026-10-21): every port function cites
its original-side EA. For each definition in src/ and modules/openje/src/, look for a 0xXXXXXX
citation in the function's leading comment block or first lines of body; report gaps.
Exit 1 = gaps exist (list mode), --gate = fail only files changed vs HEAD (incremental gate),
--selftest = verify the checker+gate logic on synthetic files before trusting a verdict."""
import re, sys, subprocess, tempfile
from pathlib import Path
DEF = re.compile(r'^\s{0,4}(?:[\w:<>,\*\& ]+\s)?([A-Za-z_~][\w:]*(?:::[A-Za-z_~][\w:]*)*)\s*\([^;{]*\)\s*(?:const\s*)?\{?\s*$')
CITE = re.compile(r'0x[0-9a-fA-F]{6}')
def files():
    for root in ("src", "modules/openje/src"):
        for p in Path(root).rglob("*.cpp"):
            if "tests" not in str(p): yield p
def changed_paths():
    """Worktree-vs-HEAD diff of the parent AND the openje submodule, plus untracked .cpp in
    either (new code is exactly what the incremental gate exists to police)."""
    out = set()
    for repo, prefix in ((".", ""), ("modules/openje", "modules/openje/")):
        for args in (["git", "-C", repo, "diff", "--name-only", "HEAD"],
                     ["git", "-C", repo, "ls-files", "--others", "--exclude-standard"]):
            r = subprocess.run(args, capture_output=True, text=True)
            if r.returncode != 0:
                sys.exit(f"--gate: git failed in {repo}: {r.stderr.strip()}")
            for line in r.stdout.splitlines():
                if line.strip().endswith(".cpp"):
                    out.add(prefix + line.strip())
    return out
def gate_offenders(per, changed):
    """Pure classification for selftest: gap-files that are ALSO changed vs HEAD."""
    return [f for f, t, n, sample in per if f in changed]
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
def selftest():
    fails = []
    # S1: a cited def clears, an uncited one is caught (window semantics pinned)
    with tempfile.TemporaryDirectory() as d:
        p = Path(d, "cited.cpp")
        p.write_text("int foo() {\n\t// shipped era @0x4b20a0\n\treturn 0;\n}\n")
        t, g = check(p)
        if not (t == 1 and g == []): fails.append(f"S1a cited: total={t} gaps={g}")
        p = Path(d, "uncited.cpp")
        p.write_text("int bar() {\n\treturn 1;\n}\n\n" * 1)
        t, g = check(p)
        if not (t == 1 and len(g) == 1): fails.append(f"S1b uncited: total={t} gaps={g}")
        # S2: gate classification — untouched gap file PASSES, changed gap file FAILS,
        # changed clean file PASSES
        per = [("a.cpp", 10, 9, []), ]
        if gate_offenders(per, {"b.cpp"}): fails.append("S2a untouched gap must pass")
        if len(gate_offenders(per, {"a.cpp"})) != 1: fails.append("S2b changed gap must fail")
        if gate_offenders([], {"z.cpp"}): fails.append("S2c changed-but-clean must pass")
    print("SELFTEST " + ("6/6 PASS" if not fails else "FAIL: " + "; ".join(fails)))
    return 0 if not fails else 1

if "--selftest" in sys.argv:
    sys.exit(selftest())

gate = "--gate" in sys.argv
tot=gap=0; per=[]
for f in files():
    t, g = check(f)
    tot += t; gap += len(g)
    if g: per.append((str(f), t, len(g), g[:3]))
per.sort(key=lambda x: -x[2])
if gate:
    off = gate_offenders(per, changed_paths())
    for f, t, n, sample in per:
        if f not in off:
            print(f"  stock (untouched vs HEAD): {f}: {n}/{t} uncited")
    if off:
        for f, t, n, sample in per:
            if f in off:
                print(f"GATE FAIL {f}: {n}/{t} uncited (file changed vs HEAD): "
                      + ", ".join(f"{fn}@{ln}" for fn, ln in sample))
        sys.exit(1)
    print(f"GATE PASS: 0 uncited defs among files changed vs HEAD "
          f"(stock untouched: {gap} of {tot}, {100*gap//max(tot,1)}%)")
    sys.exit(0)
for f, t, n, sample in per[:25]:
    print(f"{f}: {n}/{t} uncited (e.g. {', '.join(f'{fn}@{ln}' for fn,ln in sample)})")
print(f"TOTAL: {gap} uncited of {tot} defs ({100*gap//max(tot,1)}%) across {len(per)} files")
sys.exit(1 if gap else 0)
