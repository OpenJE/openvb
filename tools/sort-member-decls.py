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

DECL_RE = re.compile(r"^(\s*)([A-Za-z_][\w:<>,\s]*\**)\s+([A-Za-z_]\w*_0x[0-9a-fA-F]+)(\s*\[[^\]]*\])?\s*;(.*)$")
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


def member_entries(lines, lo, hi, prologue=None):
    """[(offset, decl_text, attached_comment_lines, trailing)] for the member block [lo, hi).

    Access specifiers ("public:", "protected:") are NOT members and must not travel with one: sorted
    into the middle of the block they silently make the following fields private (C2248 at the use
    sites). Everything that is neither a member declaration nor a // comment is collected into
    `prologue`, which the caller emits at the top of the block in original order.
    """
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
            if stripped.startswith("//"):
                lead.append(raw)          # a comment documents the NEXT member
            elif prologue is not None:
                prologue.append(raw)      # public:/protected:/#pragma - block-level, stays on top
                lead = []
            else:
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


def multi_base_start(decl_line, sized):
    """Byte offset where a multiple-inheritance class's own fields begin, or None if unsized."""
    after = decl_line.split(":", 1)[1]
    after = after.split("{", 1)[0]
    off = 0
    for part in after.split(","):
        name = part.replace("public", " ").replace("protected", " ").replace("private", " ") \
                   .replace("virtual", " ").strip()
        name = name.split("::")[-1].strip()
        if not name or name not in sized:
            return None
        base_size = sized[name][0]
        align = 4 if base_size % 4 == 0 else 1
        off = (off + align - 1) // align * align
        off += base_size
    return off


def tree_sized():
    """Every class in the tree, so a base declared in another header still resolves."""
    classes = {}
    for dirpath, _dirs, files in os.walk(cmo.SRC):
        for f in files:
            if f.endswith((".hpp", ".h")):
                for cls in cmo.header_classes(os.path.join(dirpath, f)):
                    classes.setdefault(cls["name"], cls)
    return {k: v for k, v in cmo.sizes_of(classes).items() if v}


def fix(path, apply, sized, f3=None, headers_by_name=None):
    headers_by_name = headers_by_name or {}
    lines, classes, spans = classes_of(path)
    if not spans:
        return 0, ["%s: class/member-block mismatch - skipped whole file" % path]
    notes, changed = [], 0
    for cls, (lo, hi) in zip(classes, spans):
        if cls.get("packed"):
            notes.append("%s: packed - skipped" % cls["name"])
            continue
        # Reorder-and-pad only. Every member the port declares is KEPT (it may carry evidence F3
        # does not, e.g. a field only a ported body proves); the only removals are names F3 marks
        # base=True, i.e. the base class's own fields that this class must not re-declare. An
        # F3-driven "emit exactly F3's fields" rewrite was tried and is wrong: it drops port fields
        # (C2039 at the use sites) and it cannot type them (float vs dword).
        prologue = []
        entries = member_entries(lines, lo, hi, prologue)
        base_owned = set()
        if f3 is not None and cls["name"] in f3:
            base_owned = {name for _off, name, _sz in f3_own_members(f3[cls["name"]]) if False}
            for m in f3[cls["name"]]["members"].values():
                if m.get("base") and NAME_OFFSET_RE.match(m.get("name", "")):
                    base_owned.add(m["name"])
        def base_declares(name):
            """Does the PORT's base class actually declare this member? Dropping a name F3 marks
            base=True is only safe then - otherwise the field simply disappears and every use site
            fails with C2039."""
            seen, cur = set(), cls["base"]
            while cur and cur not in seen:
                seen.add(cur)
                if cur in sized and any(m == name for m, _o in sized[cur][1]):
                    return True
                cur = headers_by_name.get(cur, {}).get("base")
            return False

        kept = []
        for e in entries:
            if e[5].startswith("pad_"):
                # filler this tool generated on an earlier pass - the padding step below regenerates
                # it. Leaving it in makes the rewrite non-idempotent: every run would insert pads
                # around the old pads and churn the whole tree again.
                continue
            if e[5] in base_owned and base_declares(e[5]):
                notes.append("%s: %s is the BASE's field (F3 base=True) - the base declares it, so "
                             "it is not re-declared here (it would shift everything after it)"
                             % (cls["name"], e[5]))
                continue
            if e[5] in base_owned:
                notes.append("%s: %s is base=True in F3 but the port's base does not declare it - "
                             "kept (dropping it would break the use sites)" % (cls["name"], e[5]))
            kept.append(e)
        if not kept:
            continue
        # A class-typed member whose SPAN covers other declared fields is a subobject whose contents
        # the header already flattens: keeping both would put those fields after the whole subobject
        # (that is why mbr_0x38 "fell inside" cls_0x50db20 at +0x34). Drop the container, keep the
        # leaves - the struct is a memory image, and every leaf then lands on its shipped offset.
        spans = []
        for e in kept:
            d = DECL_RE.match(e[1])
            t = cmo.short(d.group(2))
            size = 4 if t.replace(" ", "") == "void*" else cmo.FIXED.get(t)
            if size is None:
                size = sized[t][0] if t in sized else None
            if size is not None and d.group(4):
                try:
                    size *= int(d.group(4).strip()[1:-1].strip(), 0)
                except ValueError:
                    size = None
            spans.append((e, t, size))
        # Only flatten a container whose span the OTHER declared members FULLY cover. Partial
        # coverage means the container carries state of its own (CameraControl's embedded orbit) and
        # dropping it deletes fields the code uses - cls_0x401d60 lost mbr_0x4/mbr_0x8 that way.
        def covered(lo, hi, ivs):
            reach = lo
            for a, b in sorted(ivs):
                if a > reach:
                    break
                reach = max(reach, b)
                if reach >= hi:
                    return True
            return reach >= hi

        flattened = []
        for e, t, size in spans:
            if t in sized and size:
                inside = [(oe[0], s2) for oe, _ot, s2 in spans
                          if oe[0] > e[0] and oe[0] < e[0] + size and s2]
                ivs = [(o, o + s2) for o, s2 in inside]
                if inside and covered(e[0], e[0] + size, ivs):
                    notes.append("%s: %s (%s at +0x%X, %d bytes) is fully covered by %d individually "
                                 "declared fields - replaced by padding"
                                 % (cls["name"], e[5], t, e[0], size, len(inside)))
                    continue
            flattened.append(e)
        kept = flattened
        offsets = [e[0] for e in kept]
        if len(set(offsets)) != len(offsets):
            notes.append("%s: duplicate offset names (shipped overlap needs a union) - skipped"
                         % cls["name"])
            continue
        decl_line = ""
        for i in range(lo, max(0, lo - 40), -1):
            if re.search(r"\b(class|struct)\s+%s\b" % re.escape(cls["name"]), lines[i]):
                decl_line = lines[i]
                break
        multi_start = None
        if cls["base"] and "," in decl_line.split(":", 1)[-1]:
            multi_start = multi_base_start(decl_line, sized)
            # MULTIPLE BASES: MSVC lays the bases out in declaration order, each at its own
            # alignment, so the class's own fields start after ALL of them. The shared parser models
            # one base, which is why GUI::Window : Base, HeapTracker was mangled - filler over a
            # layout a human had derived from the ctor. Either every base is sized here or the class
            # is refused; nothing is guessed.
            if multi_start is None:
                notes.append("%s: multiple bases, at least one unsized - skipped" % cls["name"])
                continue
        if False:
            # A base's size has to come from the same parser that reads this header; multiple
            # inheritance and hand-derived bases (GUI::Window : Base, HeapTracker) are exactly where
            # it was wrong, and it inserted filler over a layout a human had already verified.
            notes.append("%s: has a base - skipped (pass --with-bases to include)" % cls["name"])
            continue
        start_off = multi_start if multi_start is not None else plan(cls, sized)
        if start_off is None:
            notes.append("%s: base size unresolved - skipped" % cls["name"])
            continue
        body, off, bad = [], start_off, False
        indent = kept[0][4] if kept else "\t\t\t"
        for offset, raw, lead, _tr, _ind, name in sorted(kept, key=lambda e: e[0]):
            decl = DECL_RE.match(raw)
            t = cmo.short(decl.group(2))
            size = 4 if t.replace(" ", "") == "void*" else cmo.FIXED.get(t)
            if size is None:
                size = sized[t][0] if t in sized else None
            if size is not None and decl.group(4):
                # an ARRAY member: the count is part of its footprint. Ignoring it (as the first
                # version did) left "byte mbr_0x8[0xc]" counted as one byte and inserted eleven
                # bogus filler bytes, which the layout guards in the headers then rejected.
                count = decl.group(4).strip()[1:-1].strip()
                try:
                    n = int(count, 0)
                except ValueError:
                    notes.append("%s: %s has an unsizable array count '%s' - skipped class"
                                 % (cls["name"], name, count))
                    bad = True
                    break
                size *= n
            if size is None:
                notes.append("%s: %s has a type this tool cannot size - skipped class"
                             % (cls["name"], name))
                bad = True
                break
            if offset < off:
                notes.append("%s: %s (named +0x%X) falls inside the base or the previous field "
                             "(+0x%X) - skipped class" % (cls["name"], name, offset, off))
                bad = True
                break
            if offset > off:
                body.append("%sbyte pad_0x%x[%d];   // alignment filler: nothing is stored here"
                            % (indent, off, offset - off))
            body.extend(lead)
            body.append(raw)
            off = offset + size
        if bad or not body:
            continue
        body = prologue + body
        # No self-verification here: the padding step absorbs any gap, so re-simulating it would
        # only confirm the builder's own arithmetic. The real verifiers are the layout guards the
        # headers already carry (typedef char ...[ offsetof == N ? 1 : -1 ]) and the test suite -
        # a wrong base size shows up there as C2118, which is how GUI::Window's multi-base case was
        # caught before it landed.
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
            size = 4 if t.replace(" ", "") == "void*" else cmo.FIXED.get(t)
            if size is None:
                size = sized[t][0] if t in sized else None
            if size is not None and decl.group(4):
                # an ARRAY member: the count is part of its footprint. Ignoring it (as the first
                # version did) left "byte mbr_0x8[0xc]" counted as one byte and inserted eleven
                # bogus filler bytes, which the layout guards in the headers then rejected.
                count = decl.group(4).strip()[1:-1].strip()
                try:
                    n = int(count, 0)
                except ValueError:
                    notes.append("%s: %s has an unsizable array count '%s' - skipped class"
                                 % (cls["name"], name, count))
                    bad = True
                    break
                size *= n
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
    by_name = {}
    for dirpath, _dirs, _walk_files in os.walk(cmo.SRC):
        for wf in _walk_files:
            if wf.endswith((".hpp", ".h")):
                for cls in cmo.header_classes(os.path.join(dirpath, wf)):
                    by_name.setdefault(cls["name"], cls)
    f3 = load_f3() if "--f3" in argv else None
    total = 0
    for f in files:
        n, notes = fix(f, apply, sized, f3, by_name)
        total += n
        if n:
            print("%-58s %d class(es) reordered" % (os.path.relpath(f, ROOT), n))
        for note in notes:
            print("  ! %s" % note)
    print("%s %d class(es)." % ("APPLIED to" if apply else "WOULD reorder", total))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
