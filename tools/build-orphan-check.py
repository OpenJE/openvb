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
# A definition is either qualified (`int Foo::Bar(`) OR unqualified inside one or more
# `namespace X {` blocks - and in THIS repo most are the latter (`src/F3/Display.cpp` opens
# `namespace F3 { namespace Display {` then defines a bare `bool CreateGameWindow( uint )`).
# The first version of this tool matched only qualified definitions and read CRLF as noise, so
# four TUs reported "0 defs" and were then classified as DEAD WEIGHT / duplicate on ZERO
# evidence - a silent mis-classification, which is worse than a crash. A parse that yields no
# definitions is now its own class (parse-blind), never dead weight.
DEF_Q = re.compile(r"^[ \t]+(?:[A-Za-z_][\w:<>,*&\s]*?)\s+"
                   r"((?:[A-Za-z_]\w*::)+~?[A-Za-z_]\w*)\s*\(([^)]*)\)", re.M)
DEF_U = re.compile(r"^[ \t]+(?:static\s+|inline\s+|virtual\s+|explicit\s+)*"
                   r"([A-Za-z_][\w:<>&*]*?)\s+([A-Za-z_]\w*)\s*\(([^)]*)\)\s*(?:const)?\s*\{",
                   re.M)
NS_OPEN = re.compile(r"^\s*namespace\s+([A-Za-z_]\w*)[^;]*\{\s*$", re.M)
CLS_OPEN = re.compile(r"^\s*(?:class|struct)\s+([A-Za-z_]\w*)[^;{]*\{\s*$", re.M)
STRIP = [(re.compile(r"//[^\n]*"), ""), (re.compile(r'"(?:\\.|[^"\\])*"'), '""'),
         (re.compile(r"'(?:\\.|[^'\\])*'"), "''")]


def _clean(txt):
    txt = txt.replace("\r\n", "\n").replace("\r", "\n")
    for rx, rep in STRIP:
        txt = rx.sub(rep, txt)
    return txt


def _scope_stack(txt):
    """Reconstruct namespace/class nesting line by line so unqualified defs get their real
    qualified name. Brace-depth tracked over the whole text; only openings that look like a
    scope header are pushed, everything else counts as depth."""
    lines = txt.split("\n")
    stack = []                      # list of (name, depth_at_open)
    depth = 0
    scopes = []                     # parallel: scope label per line index
    for ln in lines:
        head = ln.strip()
        label = None
        if NS_OPEN.match(ln) or CLS_OPEN.match(ln):
            m = NS_OPEN.match(ln) or CLS_OPEN.match(ln)
            label = m.group(1)
        opens, closes = ln.count("{"), ln.count("}")
        scopes.append("::".join([n for n, _ in stack] + ([label] if label else [])))
        if label is not None and opens:
            stack.append((label, depth))
        depth += opens - closes
        while stack and depth <= stack[-1][1] and opens - closes < 0:
            stack.pop()
        if depth < 0:
            depth = 0
    return scopes, lines


def definitions_of(path):
    """(qualified_name, signature) pairs this TU appears to define."""
    txt = _clean(path.read_text(errors="replace"))
    out = set()
    for m in DEF_Q.finditer(txt):
        out.add((m.group(1), " ".join(m.group(2).split())))
    scopes, lines = _scope_stack(txt)
    for i, ln in enumerate(lines):
        if i >= len(scopes):
            continue
        for m in DEF_U.finditer(ln + "\n"):
            name, args = m.group(2), " ".join(m.group(3).split())
            if name in ("if", "for", "while", "switch", "return", "else", "catch"):
                continue
            qual = f"{scopes[i]}::{name}" if scopes[i] else name
            out.add((qual, args))
    return out, txt


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


VIRT_RE = re.compile(r"\bvirtual\b[^;{]*?([A-Za-z_][\w]*)\s*\(", re.M)


def virtual_names():
    """Member names declared `virtual` in any header. A virtual call compiles to a vftable
    slot load, not a symbol reference, so matching a virtual name at a call site says nothing
    about linkability. Both of this tool's first two 'latent' hits were this artifact."""
    out = set()
    for h in list(ROOT.glob("src/**/*.h*")) + list(ROOT.glob("modules/openje/src/**/*.h*")):
        out |= set(VIRT_RE.findall(_clean(h.read_text(errors="replace"))))
    return out


def main(argv):
    as_json = "--json" in argv
    strict = "--all" in argv

    listed = listed_sources()
    compiled_names = listed  # same set: being named by a build list is what "compiled" means
    tus = all_tus()
    # basenames are the join key both build lists actually use; the same basename in two dirs
    # would silently merge two files, so it is detected rather than assumed away.
    compiled_tus = {t for t in tus if t.name in compiled_names}
    dup_basenames = {t.name for t in tus if sum(1 for u in tus if u.name == t.name) > 1}

    parse_blind, listed_unparsed = [], []
    provided = {}                    # (qualified_name, sig) -> {files}
    all_defs = {}
    for t in tus:
        defs, _ = definitions_of(t)
        all_defs[t] = defs
        # PARSE-BLIND IS NOT AN ORPHAN CLASS. A LISTED TU whose defs do not parse is a
        # statement about the parser, not about the build. Collecting it unconditionally is how
        # three COMPILED files (main.cpp, CommandRegistry.cpp, ModelCache.cpp - each named in
        # BOTH build lists) were counted as orphans, and why 43 listed + 11 orphans came to 54
        # against 50 TUs on disk. Reported separately below so the blind spot stays visible
        # without polluting the census.
        if not defs:
            (listed_unparsed if t.name in compiled_names else parse_blind).append(t)
        if t.name in compiled_names:
            for d in defs:
                provided.setdefault(d, set()).add(t.name)

    virts = virtual_names()
    dead, latent, twins = [], [], []
    for t in all_tus():
        if t.name in compiled_names:
            continue
        defs = all_defs[t]
        if not defs:
            # already in parse_blind; do not also file it as dead weight
            continue
        names_here = {n for n, _g in defs}
        dup = {d for d in defs if provided.get(d, set()) - {t.name}}
        # same NAME, different SIGNATURE: an overload twin, NOT a link collision. The
        # Display.cpp case: `bool F3::Display::CreateGameWindow(uint)` here vs the compiled
        # `BOOL F3::Display::CreateGameWindow(int)` in F3.cpp - different mangled names, so
        # registering Display.cpp produces no LNK2005. Keying duplicates on name alone would
        # call every overload pair a collision forever.
        dup_names = {n for n, _g in dup}
        twin = {d for d in defs
                if d[0] in {n for n, _ in provided} and d not in dup and d[0] not in dup_names}
        uniq = defs - dup - twin
        if twin and not uniq:
            twins.append((t, sorted(f"{n}({g})" for n, g in twin)[:3]))
            continue
        if not uniq:
            dead.append((t, sorted(f'{a}({b})' for a, b in dup)[:3], len(defs),
                             'DUPLICATE: signature also defined by a built TU'))
            continue
        refs = set()
        for c in tus:
            if c.name in compiled_names and c != t:
                txt = _clean(c.read_text(errors="replace"))
                for u, _g in uniq:
                    leaf = u.split("::")[-1]
                    if leaf in virts:
                        continue                    # vftable dispatch, not a link reference
                    qual = re.escape(u.replace("::", "::"))
                    if re.search(r"\b" + qual + r"\s*\(", txt) or \
                       (u.count("::") == 0 and re.search(r"\b" + re.escape(leaf) + r"\s*\(", txt)):
                        refs.add((u, c.name))
        if refs:
            latent.append((t, sorted(u for u, _g in uniq)[:4], len(uniq), sorted(refs)[:4]))
        else:
            # UNIQUE definitions that nothing in the build references. GameWeapon.cpp is this
            # case, and an earlier print line labelled it "dup-with-compiled" using the same
            # tuple slot as the real duplicates - a label asserting the OPPOSITE of its own
            # data, which is how a correct bucket became a wrong ruling: "delete an active
            # shard mid-wave". Reason strings are per-row now, so the label cannot drift.
            dead.append((t, sorted(f'{u}()' for u, _g in uniq)[:3], len(uniq),
                         'UNREFERENCED: unique defs, no reference from any built TU'))

    orphans = [x[0] for x in dead] + [x[0] for x in latent] + [x[0] for x in twins] \
            + list(parse_blind)
    # OUTER IDENTITY. The disjoint-partition check below cannot do this: a partition of the
    # WRONG set satisfies disjointness happily - which is exactly how today's 11 passed it while
    # containing three built files. So: every on-disk TU is listed XOR orphan.
    disk = set(tus)
    both = compiled_tus & set(orphans)
    unaccounted = disk - compiled_tus - set(orphans)
    if both or unaccounted or len(orphans) != len(set(orphans)):
        print(f"FATAL: outer identity violated: on-disk={len(disk)} listed={len(compiled_tus)}"
              f" orphans={len(orphans)} (distinct={len(set(orphans))})")
        if both:
            print(f"  listed AND orphan (parser/census bug): {sorted(x.name for x in both)}")
        if unaccounted:
            print(f"  on-disk, unlisted, unclassified: {sorted(x.name for x in unaccounted)}")
        print("  Refusing the verdict rather than printing a count that does not add up.")
        return 2
    # A TU may legitimately sit in parse_blind AND another bucket only if the classes leak;
    # a count that is a sum of overlaps is not a set, so this is checked, not assumed.
    if len(orphans) != len(set(orphans)):
        print(f"FATAL: orphan classes overlap ({len(orphans)} rows, {len(set(orphans))}"
              " distinct) - the count would be a sum, not a set. Refusing the verdict.")
        return 2

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
    if not provided:
        print("FATAL: no definitions parsed from any compiled TU - the definition regex is")
        print("       broken; a latent-LNK2019 PASS from here would be vacuous.")
        return 2
    if len(orphans) == len(list(tus)):
        print("FATAL: every TU is an orphan - that is a parser failure, not a finding.")
        return 2

    rel = lambda p: str(p.relative_to(ROOT))
    print("=" * 72)
    print("build-orphan-check  (item 76: a gate claim must name what the gate compiled)")
    print(f"  build lists      : {', '.join(BUILD_FILES)}")
    print(f"  sources listed   : {len(compiled_names)} basenames")
    print(f"  TUs on disk      : {len(tus)}  (src/** + modules/openje/src/**)")
    print(f"  orphans          : {len(orphans)}")
    print(f"  identity         : listed {len(compiled_tus)} + orphans {len(orphans)}"
          f" == on-disk {len(disk)} ->"
          f" {'OK' if len(compiled_tus) + len(orphans) == len(disk) else 'BROKEN'}")
    if listed_unparsed:
        print(f"  listed but 0 defs parsed (PARSER BLIND SPOT, NOT orphans):"
              f" {len(listed_unparsed)}")
        for t in listed_unparsed:
            print(f"    {rel(t)}")
    if dup_basenames:
        print(f"  WARNING: basename join-key collisions: {sorted(dup_basenames)}")
    print()
    print(f"  LATENT LNK2019 (unique def referenced by a COMPILED TU) : {len(latent)}")
    for t, uniq, n, refs in latent:
        print(f"    {rel(t)}  defines {uniq}{' ...' if n > 4 else ''}")
        for u, c in refs:
            print(f"        referenced by built {c} via {u}(")
    print(f"  OVERLOAD TWIN (same name, different signature -> NOT a collision): {len(twins)}")
    for t, ex in twins:
        print(f"    {rel(t)}  {ex}")
    print(f"  PARSE-BLIND (no definitions matched; UNKNOWN, not evidence)   : {len(parse_blind)}")
    for t in parse_blind:
        print(f"    {rel(t)}   <- may be genuinely definition-free (registry tables, main())")
        print("              or a parser blind spot; NEVER treat as 'no symbols here'")
    # IN-FLIGHT SIGNAL. "UNREFERENCED + UNLISTED" is also the exact fingerprint of a shard
    # another seat is mid-writing: GameWeapon.cpp showed up that way while its consumer was
    # still unwritten, and a reader acting on "delete-or-absorb" would have deleted an active
    # shard mid-wave. git status gives the distinction for free, so the row is annotated rather
    # than left for a human to go and check.
    untracked = set()
    git_ok = False
    try:
        out = subprocess.run(["git", "status", "--porcelain", "-uall"], cwd=str(ROOT),
                             capture_output=True, text=True, timeout=30).stdout
        untracked = {ln.split(" ", 1)[1].strip() for ln in out.splitlines()
                     if ln.startswith("??") and len(ln) > 3}
        git_ok = True
    except Exception as exc:                      # a missing git must not fake a clean tree
        print(f"  WARNING: git status unavailable ({exc}); IN-FLIGHT column is UNKNOWN, not 'no'"
              " - do not read the delete-or-absorb list as safe to act on.")
    # UNKNOWN must render distinctly: a git failure means the in-flight column
    # was never computed, and "no" would read as "safe to delete".
    state = None if git_ok else "UNKNOWN (git status unavailable)"
    print(f"  DEAD WEIGHT / unreferenced (delete-or-absorb PARK list)  : {len(dead)}")
    print("     (in-flight = untracked in git: do NOT delete, its consumer may be mid-write)")
    for row in dead:
        t = row[0]
        flight = (f"in-flight={state}" if state else
                  f"in-flight={'YES' if rel(t) in untracked else 'no'}")
        print(f"    {rel(t)}  ({row[2]} defs) {row[3]}  [{flight}]")
        for ex in row[1]:
            print(f"        {ex}")
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
