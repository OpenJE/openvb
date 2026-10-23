#!/usr/bin/env python3
"""tools/build-orphan-check.py - name what the build actually compiled (item 76).

WHY THIS EXISTS
---------------
`cmake.toml` enumerates sources EXPLICITLY (no glob), and the cmkr regeneration guard is
INACTIVE (item 60/62), so `CMakeLists.txt` is hand-synced. Two facts follow, and both were
demonstrated in this repo rather than assumed:

  * `msvc-build` rc0 means "the 15 files in target.F3 link". It does NOT mean "the tree
    compiles". A TU in neither list is invisible to it.
  * `run-all-tests` compiles PER-SUITE explicit source lists. So a TU can be
    suite-green and build-blind at the same time, and BOTH results are honest about
    different file sets. Saying "gates green" without naming the set is the lie this tool
    exists to prevent.

TWO KINDS OF ORPHAN, AND THEY ARE OPPOSITE PROBLEMS
---------------------------------------------------
An orphan TU that duplicates a symbol already provided by a compiled TU is DEAD WEIGHT:
adding it to the build fails with LNK2005, and the correct ruling is delete or absorb.
An orphan TU that provides a symbol a COMPILED TU references is a LATENT LNK2019: the image
is only linking because some other definition happens to satisfy the reference, or because
the reference is not reached from the built target.

This tool classifies instead of just counting, and it FAILS ONLY ON THE LATENT-LINK-ERROR
CLASS. A detector that cries on all seven known orphans gets disabled inside a week; one
that fires only when the image is actually lying about its own linkability earns the gate
slot. Dead weight is reported as a PARK list, which is a decision to make, not a red build.

Usage:
    python3 tools/build-orphan-check.py            # classify, exit 1 only on LNK2019 class
    python3 tools/build-orphan-check.py --all      # also exit nonzero on dead-weight orphans
    python3 tools/build-orphan-check.py --json
"""

import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

# Where sources are enumerated. Parent build lists are the ONLY enumerators: submodules are
# pulled in through the parent's set(), so matching basenames against a submodule build file is
# a proxy substitution - that mistake produced a bogus "19 of 32 orphans" count before this
# tool existed. We never look for a submodule-local build file.
BUILD_FILES = ("cmake.toml", "CMakeLists.txt")

# A definition: something that OWNS a symbol. We take qualified function definitions and
# namespace-scoped names, deliberately conservatively (see `unresolved_is_real`).
# The repo's own style indents DEFINITIONS with a tab (`\tint GameActionMoveTo::OnServerUpdate
# ( unsigned ms ) {`), so an unanchored `^` pattern matched nothing and every TU reported zero
# definitions - which silently downgraded the whole latent-LNK2019 class to "dead weight" and
# still printed PASS. Leading indentation is therefore part of the pattern, and the two
# self-guards below refuse to emit a verdict when the parse is empty.
DEF_RE = re.compile(
    r"^[ \t]+(?:[A-Za-z_][\w:<>,*&\s]*?)\s+((?:[A-Za-z_]\w*::)+~?[A-Za-z_]\w*)\s*\(",
    re.M,
)
NS_RE = re.compile(r"^\s*namespace\s+([A-Za-z_]\w*)\s*\{", re.M)


VIRT_RE = re.compile(r"\bvirtual\b[^;{]*?([A-Za-z_][\w]*)\s*\(", re.M)


def virtual_names():
    """Member names declared `virtual` in any header under src/ or the openje tree.

    A virtual call compiles to a vftable slot load, not a symbol reference, so matching a
    virtual name at a call site proves nothing about linkability. Both of this tool's first
    two 'latent' hits were exactly this: JE::ActionMoveTo::Update_0x55b600 is declared
    `virtual int Update_0x55b600(...)` at ActionMoveTo.hpp:131. Excluding them keeps the FAIL
    list to real undefined-symbol risk instead of dispatch noise."""
    out = set()
    for h in list(ROOT.glob("src/**/*.h*")) + list(ROOT.glob("modules/openje/src/**/*.h*")):
        out |= set(VIRT_RE.findall(h.read_text(errors="replace")))
    return out


def listed_sources():
    """Every file named by either parent build list, as basenames."""
    names = set()
    for bf in BUILD_FILES:
        p = ROOT / bf
        if p.exists():
            found = re.findall(r'"([^"\n]*?\.cpp)"', p.read_text(errors="replace"))
            names |= {Path(f).name for f in found}
    return names


def all_tus():
    out = []
    for pat in ("src/**/*.cpp", "modules/openje/src/**/*.cpp"):
        out += sorted(ROOT.glob(pat))
    return out


def definitions_of(path):
    """Symbols this TU appears to define, incl. namespace-qualified reconstruction."""
    txt = path.read_text(errors="replace")
    names = set(DEF_RE.findall(txt))
    # unqualified defs inside `namespace X {` blocks: collect the namespace labels so a
    # duplicate can be matched on either spelling; ambiguity is resolved conservatively.
    return names, set(NS_RE.findall(txt))


def main(argv):
    as_json = "--json" in argv
    strict = "--all" in argv

    listed = listed_sources()
    compiled_names = listed  # same set: being named by a build list is what "compiled" means
    tus = all_tus()

    orphans = []
    provided_by_compiled = {}
    for t in tus:
        defs, nss = definitions_of(t)
        if t.name in compiled_names:
            for d in defs:
                provided_by_compiled.setdefault(d, set()).add(t.name)
                for n in nss:                       # qualified alias spelling
                    provided_by_compiled.setdefault(f"{n}::{d}", set()).add(t.name)
        else:
            orphans.append((t, defs, nss))

    virts = virtual_names()
    dead, latent = [], []
    for t, defs, nss in orphans:
        # Which of this TU's definitions are ALSO defined by a compiled TU?
        dup = {d for d in defs if provided_by_compiled.get(d, set()) - {t.name}}
        uniq = defs - dup
        if not uniq:
            dead.append((t, sorted(defs)[:4], len(defs), len(dup)))
        else:
            # Unique definitions only matter if something the build DOES compile references them.
            refs = set()
            for c in tus:
                if c.name in compiled_names and c != t:
                    txt = c.read_text(errors="replace")
                    for u in uniq:
                        leaf = u.split("::")[-1]
                        if leaf in virts:
                            continue          # vftable dispatch, not a link reference
                        # Match the QUALIFIED name. A bare leaf match is noise: this tool's
                        # first two 'latent' hits were both artifacts of it - SkinSet::Get /
                        # ::At / ::Find looked 'referenced by StringFormat.cpp' purely because
                        # `Get(` appears there, and ActionMoveTo::Update was a virtual call.
                        pat = r"\b" + re.escape(u.replace("::", "::").split("::")[-2]) + r"::" + re.escape(leaf) + r"\s*\("
                        if re.search(pat, txt):
                            refs.add((u, c.name))
            (latent if refs else dead).append(
                (t, sorted(uniq)[:4], len(uniq), sorted(refs)[:4]))

    # GUARD ON THE DETECTOR ITSELF. A listing regex that matches nothing would otherwise
    # report every TU as an orphan and, because no orphan would then look "referenced",
    # print a clean PASS. That is the vacuous-green failure this tool exists to catch, so
    # it is fatal here rather than a warning: the count must be plausible before the verdict
    # means anything. (Caught exactly this way on the tool's own first run: listed=0, 49
    # "orphans", PASS.)
    if len(compiled_names) < 20:
        print(f"FATAL: only {len(compiled_names)} sources parsed from {BUILD_FILES}; the build-list")
        print("       regex is broken or the lists moved. Refusing to emit a verdict.")
        return 2
    if not provided_by_compiled:
        print("FATAL: no definitions parsed from any compiled TU - the definition regex is")
        print("       broken; a latent-LNK2019 PASS from here would be vacuous.")
        return 2
    if len(orphans) == len(tus):
        print("FATAL: every TU is an orphan - that is a parser failure, not a finding.")
        return 2

    rel = lambda p: str(p.relative_to(ROOT))
    print("=" * 72)
    print("build-orphan-check  (item 76: a gate claim must name what the gate compiled)")
    print(f"  build lists      : {', '.join(BUILD_FILES)}")
    print(f"  sources listed   : {len(compiled_names)} basenames")
    print(f"  TUs on disk      : {len(tus)}  (src/** + modules/openje/src/**)")
    print(f"  orphans          : {len(orphans)}")
    print()
    print(f"  LATENT LNK2019 (unique def referenced by a COMPILED TU) : {len(latent)}")
    for t, uniq, n, refs in latent:
        print(f"    {rel(t)}  defines {uniq}{' ...' if n > 4 else ''}")
        for u, c in refs:
            print(f"        referenced by built {c} via {u}(")
    print(f"  DEAD WEIGHT / unreferenced (delete-or-absorb PARK list)  : {len(dead)}")
    for row in dead:
        t = row[0]
        print(f"    {rel(t)}  ({row[2]} defs, dup-with-compiled={row[1][:2] if row[1] else ''})")
    print()
    ok = not latent
    print("VERDICT:", "PASS - the image is not lying about its linkability" if ok
          else f"FAIL - {len(latent)} TU(s) provide symbols a built TU references")
    print("Bound (item 73): orphan status is decided by basename membership in the parent")
    print("  build lists only. A .cpp compiled via a variable/expanding list, an OBJECT lib,")
    print("  or a generator expression could read as an orphan here; both lists were read in")
    print("  full and enumerate explicitly, so within THIS repo the set difference is exact.")
    print("  Definition detection is regex-based, not a linker view: it catches duplicate and")
    print("  referenced-unique symbols, and will miss template/inline and macro-generated ones.")
    if as_json:
        print(json.dumps({
            "listed": sorted(compiled_names),
            "orphans": [rel(t) for t, _, _ in orphans],
            "latent": [rel(t) for t, _, _, _ in latent],
            "dead": [rel(t) for t, _, _, _ in dead],
        }, indent=2))
    if strict and orphans:
        return 1
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
