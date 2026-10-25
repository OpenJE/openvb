#!/usr/bin/env python3
# Copyright 2024 OpenJE
"""Find `mbr_0xNN` accesses whose declared offset is NOT NN.

WHY THIS EXISTS
    The port's generated headers list class members in IDA's EA order, not in offset
    order: `class connection` declares mbr_0x10, mbr_0x14, ..., mbr_0x20a4, mbr_0x21,
    mbr_0x22, ..., mbr_0x4. C++ lays members out in DECLARATION order, so `this->mbr_0x4`
    is not at +0x4. Every access that goes through a member NAME instead of an explicit
    `(byte*)this + 0xNN` offset is therefore silently reading the wrong word.

    Three shipped defects of exactly this shape were found and fixed by hand
    (net::base::meth_0x59d530's +0x48 release, net::critical_section::dtor_0x5b92a0's
    +0x4, net::socket::meth_0x5b8d40's +0x4 descriptor - which left every real socket
    open), each only because a probe happened to drive that path. This tool finds the
    REST of them by static arithmetic instead of waiting for luck.

WHAT IT COMPUTES
    For each class/struct in a header: the byte offset C++ gives each member, from
    declaration order, base-class size, an implicit vftptr when the class declares a
    virtual method, and member sizes from the declared type. Classes with any type it
    cannot size are reported as UNKNOWN and never flagged (no guessing).

WHAT IT CHECKS
    `this->mbr_0xNN` inside a `Class::method` definition, and `((Type*)x)->mbr_0xNN`
    filtered to Type, across modules/openje/src/**.cpp. A hit is a real addressing bug
    unless the class is one of the documented exceptions in ALLOWED.

USAGE
    tools/check-member-offsets.py            # census: report, exit 0
    tools/check-member-offsets.py --gate     # report, exit 1 when unreviewed hits exist
    tools/check-member-offsets.py --md FILE  # write the markdown census
"""

import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SRC = os.path.join(ROOT, "modules", "openje", "src")

# Types whose C++ size is fixed and known.
FIXED = {
    "byte": 1, "bool": 1, "char": 1, "signed": 1, "unsigned": 1,
    "word": 2, "short": 2, "wchar": 2,
    "dword": 4, "undefined4": 4, "int": 4, "long": 4, "float": 4,
    "double": 8, "undefined8": 8,
}

MEMBER_RE = re.compile(
    r"^\s*(?:/\*.*?\*/\s*)?([A-Za-z_][\w:]*(?:\s*[*&])?)\s+([A-Za-z_]\w*)\s*"
    r"(?:\[\s*(0[xX][0-9a-fA-F]+|\d+)\s*\])?\s*;"
)

# MSVC default packing on win32: a member starts at the next multiple of its alignment.
def align_of(type_name, size):
    t = short(type_name)
    if t in ("byte", "bool", "char"):
        return 1
    if t in ("word", "short", "wchar"):
        return 2
    if t in ("double", "undefined8"):
        return 8 if size % 8 == 0 else 4
    return 4
CLASS_RE = re.compile(r"^\s*(?:class|struct)\s+([A-Za-z_]\w*)\s*(?::\s*([^{]+))?\{")
FUNC_RE = re.compile(r"^\s*[\w:<>,*&\s]*?\b([A-Za-z_]\w*)::([A-Za-z_]\w*)\s*\(")

ALLOWED = {
    # (class, method-regex, reason) - empty until a case is proven benign.
}


COMMENT_RE = re.compile(r"/\*.*?\*/|//[^\n]*", re.S)


def strip_comments(text):
    """Blank out comments, preserving line structure so reported lines stay accurate.

    Both sides need this: a comment that merely MENTIONS `this->mbr_0x48` is not an access
    (net/base.cpp's own explanatory comment was the tool's first false positive), and a
    comment containing the word `virtual` must not make a class look polymorphic - which is
    exactly how net::socket was mis-sized at +4 (it declares no virtuals, so its member is
    at +0, which is the defect the socket fix measured).
    """
    return COMMENT_RE.sub(lambda m: re.sub(r"[^\n]", " ", m.group(0)), text)


def base_name(bases):
    """`public Foo, private Bar` -> Foo (the first base's class name)."""
    if not bases.strip():
        return None
    first = bases.split(",")[0]
    words = [w for w in re.findall(r"[A-Za-z_]\w*", first)
             if w not in ("public", "protected", "private", "virtual")]
    return short(words[-1]) if words else None


def short(name):
    name = name.strip()
    name = re.sub(r"^(?:::)?(?:[A-Za-z_]\w*::)*", "", name)
    return name.strip(" *&")


def header_classes(path):
    """[{name, base, size, members:[(name, off)], unknown}] for one header."""
    try:
        text = open(path, encoding="utf-8", errors="replace").read()
    except OSError:
        return []
    lines = text.splitlines()
    out = []
    i = 0
    while i < len(lines):
        m = CLASS_RE.match(lines[i])
        if not m:
            i += 1
            continue
        name, bases = m.group(1), m.group(2) or ""
        depth = lines[i].count("{") - lines[i].count("}")
        body, i = [], i + 1
        while i < len(lines) and depth > 0:
            depth += lines[i].count("{") - lines[i].count("}")
            if depth > 0:
                body.append(lines[i])
            i += 1
        # members at body depth 0 only (nested classes have their own "{" lines)
        members, unknown, level, has_virtual = [], False, 0, False
        packed = False
        for line in body:
            code = re.sub(r"//.*$", "", line)      # a trailing comment is not a declaration
            code = re.sub(r"/\*.*?\*/", "", code)
            if re.search(r"#pragma\s+pack\s*\(\s*push\s*,\s*1\s*\)", code):
                packed = True
            elif re.search(r"#pragma\s+pack\s*\(\s*pop\s*\)", code):
                packed = False
            if level == 0 and re.search(r"\bvirtual\b", code):
                has_virtual = True
            if level == 0:
                mm = MEMBER_RE.match(line)
                if mm:
                    members.append((mm.group(2), mm.group(1), mm.group(3)))
            level += line.count("{") - line.count("}")
            if level < 0:
                level = 0
        out.append({
            "name": name,
            "base": base_name(bases),
            "virtual": has_virtual,
            "packed": packed,
            "members": members,
            "unknown": not members,
        })
    return out


def sizes_of(classes):
    """Resolve each class's size + member offsets; None where a type is unknown."""
    computed = {}
    for _ in range(6):  # fixed-point: class sizes depend on member class sizes
        progress = False
        for cls in classes.values():
            base, add_vft = 0, 0
            if cls["base"]:
                if cls["base"] not in computed or computed[cls["base"]] is None:
                    # base declared but not sized (or not parsed at all): do NOT guess a size -
                    # an invented base would produce invented offsets, i.e. false positives.
                    continue
                base = computed[cls["base"]][0]     # a base with a vftable already carries it
            elif not any(m[0] == "vftptr_0x0" for m in cls["members"]):
                add_vft = 4 if cls["virtual"] else 0
            off = base + add_vft
            table, ok = [], True
            for mname, mtype, count in cls["members"]:
                t = short(mtype)
                n = (int(count, 0) if count else 0) or 1
                if t in FIXED:
                    elem = FIXED[t]
                    sz = elem * n
                elif t in computed and computed[t] is not None:
                    elem = computed[t][0]
                    sz = elem * n
                else:
                    ok = False
                    break
                a = 1 if cls.get("packed") else align_of(mtype, elem)
                if a > 1:
                    off = (off + a - 1) // a * a
                table.append((mname, off))
                off += sz
            if ok:
                biggest = 8 if any(short(t) in ("double", "undefined8")
                                   for _n, t, _c in cls["members"]) else 4
                a = 1 if cls.get("packed") else (biggest if cls["virtual"] or off > 4 else 1)
                off = (off + a - 1) // a * a
                val = (off, table)
                if computed.get(cls["name"]) != val:
                    computed[cls["name"]] = val
                    progress = True
            else:
                computed.setdefault(cls["name"], None)
        if not progress:
            break
    return computed


# Data-member names carry the shipped offset as text (mbr_0x14, vftptr_0x0). Comparing that text with
# the offset the compiler would give the member is a check that needs NO access site: it catches the
# rotation defect class (a byte declared before the dwords pushes every later field up one slot) in
# headers no ported .cpp touches yet. Gfx_CamCtrl was exactly that - EA-ordered, byte mbr_0x4 first,
# and invisible to the access census because nothing accessed it.
# any member whose NAME encodes its shipped offset: mbr_0x14, vftptr_0x0, gap_0x04,
# reserved_0x30. The name is the claim; the declaration order is what must honour it.
NAME_OFFSET_RE = re.compile(r"^[A-Za-z_]\w*_0x([0-9a-fA-F]+)$")


def static_layout_rows(sized, ambiguous, where):
    """(class, member, declared, named-offset, file) for every member whose name contradicts layout."""
    rows = []
    for cname in sorted(sized):
        if cname in ambiguous:
            continue
        for mname, off in sized[cname][1]:
            m = NAME_OFFSET_RE.match(mname)
            if not m:
                continue
            named = int(m.group(1), 16)
            if named != off:
                rows.append((cname, mname, off, named, where.get(cname, "?")))
    return rows


def main(argv):
    gate = "--gate" in argv
    md_path = None
    if "--md" in argv:
        md_path = argv[argv.index("--md") + 1]

    headers = {}
    where = {}
    # A class NAME is not unique across the tree: `exception` exists as JE::net::exception and as
    # the nested JE::net::connection::exception (and again as JE::net::buffer::overflow's base
    # spelling), all collapsing to the short name. Resolution then answers for the wrong class and
    # reports a bogus offset - a FALSE POSITIVE, the one failure mode this tool must not have,
    # because a permanently wrong row hides real regressions in the same file. Names are counted
    # here so that any resolution path touching an ambiguous name is skipped instead of guessed.
    name_count = {}
    for dirpath, _dirs, files in os.walk(SRC):
        for f in files:
            if f.endswith(".hpp") or f.endswith(".h"):
                for cls in header_classes(os.path.join(dirpath, f)):
                    name_count[cls["name"]] = name_count.get(cls["name"], 0) + 1
                    where.setdefault(cls["name"], os.path.relpath(os.path.join(dirpath, f), ROOT))
                    if cls["name"] not in headers:
                        headers[cls["name"]] = cls
    ambiguous = {n for n, c in name_count.items() if c > 1}
    computed = sizes_of(headers)

    def lookup_offset(cls_name, member):
        """The offset of `member` in `cls_name`, following base classes."""
        seen = set()
        while cls_name and cls_name not in seen:
            if cls_name in ambiguous:
                return None        # the name resolves to more than one class: no evidence
            seen.add(cls_name)
            target = sized.get(cls_name)
            if target:
                for mname, off in target[1]:
                    if mname == member:
                        return off
            cls_name = headers.get(cls_name, {}).get("base")
        return None

    sized = {k: v for k, v in computed.items() if v}
    hits, unknown_hits = [], 0
    this_re = re.compile(r"this\s*->\s*(mbr_0x[0-9a-fA-F]+)")
    cast_re = re.compile(r"\(\s*\(\s*(?:JE::)?(?:[\w:]+::)?(\w+)\s*\*\s*\)[^)\n]*\)\s*->\s*(mbr_0x[0-9a-fA-F]+)")

    for dirpath, _dirs, files in os.walk(SRC):
        for f in files:
            if not f.endswith(".cpp"):
                continue
            path = os.path.join(dirpath, f)
            text = strip_comments(open(path, encoding="utf-8", errors="replace").read())
            owner, offset, line_no = None, 0, 0
            for line in text.splitlines():
                line_no += 1
                fm = FUNC_RE.match(line)
                if fm:
                    owner = fm.group(1)
                for m in this_re.finditer(line):
                    target = sized.get(owner)
                    if not target:
                        if owner in computed:
                            unknown_hits += 1
                        continue
                    want = int(m.group(1)[6:], 16)
                    got = lookup_offset(owner, m.group(1))
                    if got is not None and got != want:
                        hits.append((os.path.relpath(path, ROOT), line_no, owner, m.group(1), got))
                for m in cast_re.finditer(line):
                    cls = m.group(1)
                    target = sized.get(cls)
                    if not target:
                        continue
                    want = int(m.group(2)[6:], 16)
                    got = lookup_offset(cls, m.group(2))
                    if got is not None and got != want:
                        hits.append((os.path.relpath(path, ROOT), line_no, cls, m.group(2), got))

    by_file = {}
    for path, ln, owner, member, got in hits:
        by_file.setdefault(path, []).append((ln, owner, member, got))

    print("classes parsed: %d   sized: %d   unsized: %d"
          % (len(headers), len(sized), len(headers) - len(sized)))
    print("member-name accesses with a wrong declared offset: %d in %d files"
          % (len(hits), len(by_file)))
    for path in sorted(by_file):
        rows = by_file[path]
        print("  %-58s %d" % (path, len(rows)))
        for ln, owner, member, got in rows[:6]:
            print("      :%d  %s::%s -> declared +0x%X" % (ln, owner, member, got))
        if len(rows) > 6:
            print("      ... %d more" % (len(rows) - 6))

    static_rows = static_layout_rows(sized, ambiguous, where)
    if "--static" in argv or static_rows:
        static_files = {r[4] for r in static_rows}
        print("members whose declared offset contradicts the offset in their own name: %d in %d files"
              % (len(static_rows), len(static_files)))
        shown = {}
        for cname, mname, off, named, path in static_rows:
            shown.setdefault(path, []).append((cname, mname, off, named))
        for path in sorted(shown):
            rows = shown[path]
            print("  %-58s %d" % (path, len(rows)))
            for cname, mname, off, named in rows[:6]:
                print("      %s::%s named +0x%X but laid out at +0x%X" % (cname, mname, named, off))
            if len(rows) > 6:
                print("      ... %d more" % (len(rows) - 6))

    if md_path:
        with open(md_path, "w", encoding="utf-8") as fh:
            fh.write("# Member-offset census (tools/check-member-offsets.py)\n\n")
            fh.write("Generated by `tools/check-member-offsets.py --md docs/je/MEMBER-OFFSET-AUDIT.md`.\n\n")
            fh.write("Every row is an access that uses a member NAME while the generated header's\n"
                     "declaration order puts that member at a different offset - the exact shape of the\n"
                     "three defects fixed in net::base, net::critical_section and net::socket.\n\n")
            fh.write("classes parsed %d | sized %d | wrong-offset accesses %d in %d files\n\n"
                     % (len(headers), len(sized), len(hits), len(by_file)))
            fh.write("| file | line | class | member | declared offset |\n|---|---|---|---|---|\n")
            for path in sorted(by_file):
                for ln, owner, member, got in sorted(by_file[path]):
                    fh.write("| `%s` | %d | `%s` | `%s` | +0x%X |\n" % (path, ln, owner, member, got))
        print("wrote %s" % os.path.relpath(md_path, ROOT))

    # RATCHET: the tree is mid-migration from IDA's EA order to shipped offset order. A hard gate
    # ("zero rows") would fail for the 144 files still waiting, so the contract enforced here is that
    # a change may REDUCE the count but never raise it: a new member declared at an offset its own
    # name contradicts is a regression the moment it lands, whatever the backlog is.
    if "--ratchet" in argv:
        ceiling = int(argv[argv.index("--ratchet") + 1])
        if len(static_rows) > ceiling:
            print("GATE FAIL: %d members contradict their own named offset, ceiling %d (%+d)"
                  % (len(static_rows), ceiling, len(static_rows) - ceiling))
            for cname, mname, off, named, path in static_rows[:10]:
                print("   %s::%s named +0x%X declared +0x%X" % (cname, mname, named, off))
            return 1
        print("layout-ratchet: %d rows <= ceiling %d%s"
              % (len(static_rows), ceiling,
                 " (lower the ceiling to %d)" % len(static_rows)
                 if len(static_rows) < ceiling else ""))

    if "--static-gate" in argv and static_rows:
        print("GATE FAIL: %d members are declared at an offset their name contradicts" % len(static_rows))
        return 1
    if gate and hits:
        print("GATE FAIL: %d member-name accesses read the wrong offset" % len(hits))
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
