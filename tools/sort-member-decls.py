#!/usr/bin/env python3
"""Rewrite a generated header's member block into shipped OFFSET order.

Why this exists: the generated headers list data members in IDA's EA (alphabetical) order, not in
offset order. That is harmless for names like mbr_0x10 (a dword sorts where it belongs) but not when
a smaller member sits in the middle: `byte mbr_0x4` sorted before the dwords makes the compiler put
the byte first and push EVERY following field one slot up. The member-offset census only sees
classes that ported code touches, so most of the tree stayed silently wrong - ActionDie declares
byte 0x10, dword 0x14, byte 0x18, byte 0x4 and lays out 0x4 at +0x11.

This tool sorts each class's members by the offset encoded in their names - the F3 offset the name
already carries - and inserts explicit padding members where the sorted declaration order would
otherwise leave a hole. It refuses to touch a class it cannot settle unambiguously:
  * a base class whose size is not resolvable here (no invented bases),
  * two members carrying the same offset name (a shipped overlap needs a union, not an order),
  * packed classes (padding rules differ),
  * a member whose type is not sized by the checker.

Usage:
  tools/sort-member-decls.py --check FILE...     report what would change, write nothing
  tools/sort-member-decls.py --apply FILE...     rewrite the member blocks

The authoritative verifier is tools/check-member-offsets.py --static: after --apply, the files named
must contribute zero static rows.
"""
import importlib.util
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
_spec = importlib.util.spec_from_file_location("cmo", os.path.join(ROOT, "tools", "check-member-offsets.py"))
cmo = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(cmo)

DECL_RE = re.compile(r"^(\s*)([A-Za-z_][\w:<>,\s]*?)\s+(mbr_0x[0-9a-fA-F]+|vftptr_0x0)(\s*\[[^\]]*\])?\s*;(.*)$")
NAME_OFFSET_RE = cmo.NAME_OFFSET_RE


def classes_of(path):
    """header_classes() plus the member block's line span, so the rewrite is in-place."""
    text = open(path, encoding="utf-8", errors="replace").read()
    lines = text.splitlines()
    classes = cmo.header_classes(path)
    spans = []
    start = None
    depth = 0
    for i, line in enumerate(lines):
        if line.strip() == "//: Members":
            start = i
        elif start is not None and line.strip().startswith("//: Methods"):
            spans.append((start, i))
            start = None
    if len(spans) != len(classes):
        return lines, classes, []
    return lines, classes, spans


def member_entries(lines, lo, hi):
    """[(offset, decl_text, attached_comment_lines, trailing)] for the member block [lo, hi)."""
    out = []
    lead = []
    for i in range(lo + 1, hi):
        raw = lines[i]
        stripped = raw.strip()
        if not stripped:
            lead = []
            continue
        m = DECL_RE.match(raw)
        if not m:
            # a comment (or a continuation of one) documents the NEXT member
            lead.append(raw)
            continue
        indent, _type, name, _arr, trailing = m.groups()
        nm = NAME_OFFSET_RE.match(name)
        out.append((int(nm.group(1), 16), raw, lead, trailing, indent, name))
        lead = []
    return out


def plan(cls, sized):
    """(sorted_entries, pad_inserts) or None when the class cannot be settled from names alone."""
    base, add_vft = 0, 0
    if cls["base"]:
        if cls["base"] not in sized:
            return None
        base = sized[cls["base"]][0]
    elif not any(m[0] == "vftptr_0x0" for m in cls["members"]):
        add_vft = 4 if cls["virtual"] else 0
    off = base + add_vft
    return off


SIZE_TYPE = {1: "byte", 2: "word", 4: "dword", 8: "undefined8"}


def load_f3():
    """demangled class name -> structure, only where the name is unambiguous."""
    import json
    path = os.path.join(ROOT, "artifacts", "F3.json")
    data = json.load(open(path))
    seen, out = {}, {}
    for key, st in data["structures"].items():
        name = st.get("demangled_name") or key
        seen[name] = seen.get(name, 0) + 1
        out[name] = st
    return {n: out[n] for n, c in seen.items() if c == 1}


def f3_own_members(st):
    """[(offset, name, size)] for the fields F3 says are the class's OWN (base=False)."""
    rows = []
    for off, m in st["members"].items():
        if m.get("base"):
            continue
        name = m.get("name", "")
        if not NAME_OFFSET_RE.match(name):
            continue            # the base subobject row, or an unnamed gap
        rows.append((int(off, 16), name, m.get("size")))
    return sorted(rows)


def tree_sized():
    """Every class in the tree, so a base declared in another header still resolves."""
    classes = {}
    for dirpath, _dirs, files in os.walk(cmo.SRC):
        for f in files:
            if f.endswith((".hpp", ".h")):
                for cls in cmo.header_classes(os.path.join(dirpath, f)):
                    classes.setdefault(cls["name"], cls)
    return {k: v for k, v in cmo.sizes_of(classes).items() if v}


def fix(path, apply, sized, f3=None):
    lines, classes, spans = classes_of(path)
    if not spans:
        return 0, ["%s: class/member-block mismatch - skipped whole file" % path]
    notes, changed = [], 0
    for cls, (lo, hi) in zip(classes, spans):
        if cls.get("packed"):
            notes.append("%s: packed - skipped" % cls["name"])
            continue
        if f3 is not None:
            own = f3_own_members(f3[cls["name"]])
            start = plan(cls, sized)
            if start is None:
                notes.append("%s: base size unresolved - skipped" % cls["name"])
                continue
            have = {}
            for offset, raw, lead, _tr, _ind, name in member_entries(lines, lo, hi):
                have[name] = (raw, lead)
            indent = "\t\t\t"
            body, off, bad = [], start, False
            for offset, name, size in own:
                if size not in SIZE_TYPE:
                    notes.append("%s: %s has size %s, no C type for it - skipped class"
                                 % (cls["name"], name, size))
                    bad = True
                    break
                if offset < off:
                    notes.append("%s: %s overlaps a shipped field - skipped class"
                                 % (cls["name"], name))
                    bad = True
                    break
                if offset > off:
                    body.append("%sbyte pad_0x%x[%d];   // alignment filler: nothing is stored here"
                                % (indent, off, offset - off))
                raw, lead = have.get(name, (None, []))
                if raw is None:
                    raw = "%s%s %s;   // F3: %d byte(s), not declared in the port before"
                    raw = raw % (indent, SIZE_TYPE[size], name, size)
                body.extend(lead)
                body.append(raw)
                off = offset + size
            if bad or not body:
                continue
            if body != lines[lo + 1:hi]:
                changed += 1
                if apply:
                    lines[lo + 1:hi] = body
            continue

        entries = member_entries(lines, lo, hi)
        if not entries:
            continue
        offsets = [e[0] for e in entries]
        if len(set(offsets)) != len(offsets):
            notes.append("%s: duplicate offset names (shipped overlap needs a union) - skipped"
                         % cls["name"])
            continue
        if offsets == sorted(offsets) and all(
                cmo.NAME_OFFSET_RE.match(e[5]) for e in entries):
            # already in offset order; the checker decides whether it actually lands there
            pass
        start = plan(cls, sized)
        if start is None:
            notes.append("%s: base size unresolved - skipped" % cls["name"])
            continue
        body, off = [], start
        for offset, raw, lead, trailing, indent, name in sorted(entries, key=lambda e: e[0]):
            decl = DECL_RE.match(raw)
            if offset < off:
                notes.append("%s: %s overlaps the previous member - skipped class" % (cls["name"], name))
                body = None
                break
            if offset > off:
                body.append("%sbyte pad_0x%x[%d];   // alignment filler: nothing is stored here" %
                            (indent, off, offset - off))
            body.extend(lead)
            body.append(raw)
            if decl.group(4):
                notes.append("%s: %s is an array - skipped class" % (cls["name"], name))
                body = None
                break
            t = cmo.short(decl.group(2))
            size = cmo.FIXED.get(t)
            if size is None:
                size = sized[t][0] if t in sized else None
            if size is None:
                notes.append("%s: %s has a type this tool cannot size - skipped class"
                             % (cls["name"], name))
                body = None
                break
            off = offset + size
        if body is None:
            continue
        new = body
        if new != lines[lo + 1:hi]:
            changed += 1
            if apply:
                lines[lo + 1:hi] = new
    if apply and changed:
        open(path, "w", encoding="utf-8").write("\n".join(lines) + "\n")
    return changed, notes


def main(argv):
    apply = "--apply" in argv
    files = [a for a in argv if not a.startswith("--")]
    sized = tree_sized()
    f3 = load_f3() if "--f3" in argv else None
    total = 0
    for f in files:
        n, notes = fix(f, apply, sized, f3)
        total += n
        if n:
            print("%-58s %d class(es) reordered" % (os.path.relpath(f, ROOT), n))
        for note in notes:
            print("  ! %s" % note)
    print("%s %d class(es)." % ("APPLIED to" if apply else "WOULD reorder", total))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
