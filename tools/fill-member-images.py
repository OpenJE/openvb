#!/usr/bin/env python3
"""Fill the GAPS in a recovered class's byte image, from F3's member map.

A declaration list silently closes holes: if F3 has no member at +0x180 and the previous member ends at
+0x17C, a header that just lists fields puts the next field at +0x180 and every successor lands 4 bytes
early. The same happens when F3 records a member the port never declared, or a subobject span the port
declared as a single dword. This tool inserts what is missing and touches nothing else:

  * a gap that F3 fills with members -> those members, at their recorded sizes (subobject spans become
    byte arrays - the struct is a memory image);
  * a gap F3 does not fill          -> `byte reserved_0xNNN[N]` to hold the place.

It never deletes or retypes an existing declaration, so port evidence (and every use site) survives.
Classes whose declared names are not in offset order are skipped: their layout problem is ordering, which
tools/sort-member-decls.py handles.

    python3 tools/fill-member-images.py --check  modules/openje/src/JE/*.hpp
    python3 tools/fill-member-images.py --apply  modules/openje/src/JE/*.hpp
"""
import importlib.util
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))


def load(name):
    spec = importlib.util.spec_from_file_location(name, os.path.join(HERE, name + ".py"))
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


cmo = load("check-member-offsets")
# reserved_0xNNN placeholders written by an earlier pass are part of the image too: matching only
# mbr_ names makes the tool try to re-insert fields those holes already cover.
NAME_RE = re.compile(r"(?:mbr|reserved)_0x([0-9a-fA-F]+)")


def image_inserts(cls, f3cls):
    """[(after-name, [decl lines])] to insert, or None when the class's names are out of order."""
    members = f3cls["members"]
    f3_offs = {}
    for o, m in members.items():
        f3_offs[int(o, 16)] = m
    decls = []
    for mname, mtype, count in cls["members"]:
        m = NAME_RE.match(mname)
        if not m:
            continue
        elem = cmo.FIXED.get(cmo.short(mtype))
        n = (int(count, 0) if count else 0) or 1
        decls.append((mname, int(m.group(1), 16), None if elem is None else elem * n))
    if not decls:
        return None
    offs = [o for _n, o, _s in decls]
    if offs != sorted(offs):
        return None                      # ordering problem, not an image problem
    out, prev_end, covered_to = [], None, -1
    for name, off, size_now in decls:
        if prev_end is not None and off > prev_end and prev_end > covered_to:
            lines = []
            cursor = max(prev_end, covered_to)
            for o in sorted(o for o in f3_offs if prev_end <= o < off):
                m = f3_offs[o]
                if o < cursor:
                    continue
                name2 = re.sub(r"[^A-Za-z0-9_]", "_", m["name"])
                size = m["size"]
                if m.get("type") == "struc" or size > 8:
                    lines.append("\t\t\tbyte %s[0x%X];   // +0x%X F3 %s, %d bytes (flattened image)"
                                 % (name2, size, o, m["name"], size))
                elif size == 8:
                    lines.append("\t\t\tbyte %s[8];      // +0x%X" % (name2, o))
                else:
                    lines.append("\t\t\t%s %s;        // +0x%X"
                                 % ({1: "byte", 2: "word", 4: "dword"}.get(size, "dword"), name2, o))
                cursor = o + size
            if cursor < off:
                lines.append("\t\t\tbyte reserved_0x%X[0x%X];   // +0x%X..+0x%X hole in the F3 map"
                             % (cursor, off - cursor, cursor, off))
            if lines:
                out.append((name, lines))
        prev_end = off + (size_now or 0)
        # A member larger than a dword is a SPAN - a flattened subobject or a byte array. F3's members
        # inside it are already represented by it, so inserting them again would double the image (that
        # is exactly what took this class from 0 rows back to 69 on the first attempt).
        if size_now is not None and size_now > 4:
            covered_to = max(covered_to, prev_end)
    return out


def fix(path, apply, f3):
    classes = cmo.header_classes(path)
    if not classes:
        return 0, []
    if len(classes) > 1:
        # The anchor is located by member name, and a file with several classes can repeat names.
        # Refuse rather than insert into the wrong class.
        return 0, ["%s: %d classes - skipped (anchor lookup is per-file)" % (path, len(classes))]
    text = open(path, encoding="utf-8", errors="replace").read()
    lines = text.splitlines()
    pairs = []
    for cls in classes:
        if cls["name"] not in f3 or cls["name"] in getattr(fix, "ambiguous", set()):
            continue
        if cls.get("base"):
            # A DERIVED class's leading "gap" is the base subobject, which the base already declares.
            # Filling it would insert a second copy and shift every member - measured: applying this to
            # derived classes took the census from 1822 to 5000. Those classes are the reorderer's job.
            continue
        ins = image_inserts(cls, f3[cls["name"]])
        if ins:
            pairs.append((cls["name"], ins))
    if not pairs:
        return 0, []
    notes, changed = [], 0
    for cname, ins in pairs:
        # locate each anchor declaration line and insert after it
        for after, new_lines in ins:
            idx = None
            for i, line in enumerate(lines):
                if re.search(r"\b%s\b" % re.escape(after), line) and not line.strip().startswith("//"):
                    idx = i
            if idx is None:
                continue
            lines[idx + 1:idx + 1] = new_lines
            notes.append("%s: inserted %d declaration(s) after %s" % (cname, len(new_lines), after))
            changed += 1
    if apply and changed:
        open(path, "w", encoding="utf-8").write("\n".join(lines) + "\n")
    return changed, notes


def main(argv):
    files = [a for a in argv[1:] if not a.startswith("-")]
    apply = "--apply" in argv
    if apply and len(files) > 1:
        # MEASURED FAILURE, do not remove this guard: applying this tool across the tree took the
        # static census from 1822 to 5000 rows in 394 files. Two causes, neither fixable from the
        # declarations alone: (1) a derived class's leading "gap" is its base subobject, which the base
        # already declares, so filling it inserts a second copy; (2) whether a gap is real depends on
        # spans the header expresses as one array, and a wrong guess doubles that span's fields.
        # Per-file application, reviewed by eye, is the only safe mode.
        print("refusing --apply on %d files: this tool is per-class hand work, not a bulk pass "
              "(measured: tree-wide apply took the census 1822 -> 5000)" % len(files))
        return 2
    f3 = load("sort-member-decls").load_f3()
    total = 0
    for path in files:
        n, notes = fix(path, apply, f3)
        total += n
        for note in notes[:4]:
            print("  %s: %s" % ("FILLED" if apply else "WOULD fill", note))
        if len(notes) > 4:
            print("  ... %d more in %s" % (len(notes) - 4, path))
    print("%s %d gap(s) in %d file(s)" % ("APPLIED" if apply else "WOULD apply", total, len(files)))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
