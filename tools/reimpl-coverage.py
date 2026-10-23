#!/usr/bin/env python3
"""reimpl-coverage.py — % of demangled ORIGINAL classes that exist in the reimplementation tree.

Emits ONE number (percent with two decimals) on stdout so it can drive a GLLA /loop metric.
Everything else goes to stderr.

Definition (deliberately conservative - it under-claims, which is the safe direction):
  TARGET  = every structure in artifacts/F3.json whose demangled_name is a real name
            (not a cls_0x<addr> placeholder). This is the recovered original surface.
  COVERED = the target name appears in the reimplementation as a `class X` or `struct X`
            declaration at the start of a line in src/**/*.hpp|*.h|*.cpp.

LIMITS (stated because a metric that hides its own limits manufactures false progress):
  - A reimplementation that declares the right name but with wrong size/members/vtables
    counts as covered. This measures SURFACE adoption, not fidelity. Pair it with the
    lockstep/diff-digest evidence ladder before trusting any single class.
  - Renaming a reimpl class to a NON-original name to game the metric is visible in review;
    the loop rule is to match the recovered name, not to re-title existing code.
  - Forward declarations count. `class Action;` in any header marks Action covered. If the
    loop plateau stalls on forward-decl-only wins, tighten this file, don't add aliases.
  - F3.json is the reference; it is only as good as the RE wave that last refreshed it.
"""

import argparse
import glob
import json
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DB = os.path.join(ROOT, "artifacts", "F3.json")
PLACEHOLDER = re.compile(r"^cls_0x[0-9a-fA-F]+$")
DECL = re.compile(r"^\s*(?:template\s*<[^>]*>\s*)?(?:class|struct)\s+(\w+)", re.M)


def usage_count(entry: dict) -> int:
    """Evidence weight: summed call-site usages across members + methods."""
    n = 0
    for m in (entry.get("members") or {}).values():
        n += len(m.get("usages") or [])
    for meth in entry.get("methods") or []:
        n += 1 if isinstance(meth, (int, str)) else 0
    return n


def main() -> int:
    ap = argparse.ArgumentParser(description="reimpl surface coverage metric / work queue")
    ap.add_argument("--report", type=int, metavar="N",
                    help="list top-N UNCOVERED classes ranked by evidence weight instead of printing the metric")
    args = ap.parse_args()
    with open(DB, encoding="utf-8") as fh:
        db = json.load(fh)
    targets = {
        v["demangled_name"]
        for v in db["structures"].values()
        if v.get("demangled_name") and not PLACEHOLDER.match(v["demangled_name"])
    }
    decls = set()
    for pat in ("**/*.hpp", "**/*.h", "**/*.cpp"):
        for path in glob.glob(os.path.join(ROOT, "src", pat), recursive=True):
            with open(path, encoding="utf-8", errors="ignore") as fh:
                decls.update(DECL.findall(fh.read()))
    if not targets:
        print("reimpl-coverage: no demangled targets in F3.json", file=sys.stderr)
        return 2
    covered = targets & decls
    pct = len(covered) / len(targets) * 100
    print(f"orig_targets={len(targets)} reimpl_decls={len(decls)} covered={len(covered)}", file=sys.stderr)
    if args.report:
        gaps = []
        for key, entry in db["structures"].items():
            name = entry.get("demangled_name")
            if not name or name not in targets or name in covered:
                continue
            gaps.append((usage_count(entry), name, entry.get("size"), len(entry.get("methods") or {})))
        gaps.sort(reverse=True)
        print(f"# next targets: uncovered original classes by evidence weight (usages, name, size, methods)")
        for w, name, size, nmeth in gaps[: args.report]:
            print(f"{w}\t{name}\t{size}\t{nmeth}")
        return 0
    print(f"{pct:.2f}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
