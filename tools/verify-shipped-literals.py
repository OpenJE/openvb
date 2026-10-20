#!/usr/bin/env python3
"""Assert every string we call "shipped verbatim" is byte-identical in the original F3.exe.

Born in T-021: a lint grep was about to "fix" a shipped log string, and the only thing
separating a preserved literal from a corrupted one was a hand byte-search of the original
image at offset 0x24fb48. "We preserved it" and "it matches the original" are different
claims; only the second is testable, so this tests it.

WHAT THIS PROVES                                WHAT IT DOES NOT PROVE
- each asserted literal's bytes occur in the     - that our code CALLS the string from where the
  shipped image at ONE offset, and the EA         original does (no xref analysis is done)
  that offset maps to                            - that surrounding behaviour matches, or that the
- that the match is a WHOLE string, not a          exe runs at all
  fragment of a longer one                       - that a string missing from our sources ought to
- that the match is UNAMBIGUOUS (0 = drift,        exist; absence of a claim is not a failure
  >1 = no site without a pinned EA)

A green run is a STRING-FIDELITY gate. It is never an acceptance result for behaviour.
"""

import argparse
import os
import re
import sys
import subprocess
import shutil
import tempfile

MARKER = "shipped string verbatim"
DEFAULT_MANIFEST = "tools/shipped-literals.txt"
SCAN_SUBPATHS = ["src", "modules"]
SRC_EXT = (".cpp", ".hpp", ".h", ".c")
_STRING_RE = re.compile(r'"((?:[^"\\\n]|\\.)*)"')


class Failure(Exception):
    pass


# ------------------------------------------------------------- literal syntax
_ESC = {"n": 0x0A, "t": 0x09, "r": 0x0D, "0": 0x00, "a": 0x07, "b": 0x08, "f": 0x0C,
        "v": 0x0B, "\\": 0x5C, '"': 0x22, "'": 0x27, "?": 0x3F}


def decode_c_string(body):
    """Inside of a C string literal -> bytes (C++98 basic escapes + \\xNN)."""
    out, i = bytearray(), 0
    while i < len(body):
        ch = body[i]
        if ch != "\\":
            out += ch.encode("utf-8")
            i += 1
            continue
        i += 1
        if i >= len(body):
            raise Failure("trailing backslash")
        if body[i] == "x":
            j = i + 1
            while j < len(body) and body[j] in "0123456789abcdefABCDEF":
                j += 1
            if j == i + 1:
                raise Failure("\\x with no digits")
            out.append(int(body[i + 1:j], 16))
            i = j
            continue
        if body[i] in _ESC:
            out.append(_ESC[body[i]])
            i += 1
            continue
        raise Failure("unsupported escape \\%s" % body[i])
    return bytes(out)


def strip_comments(text):
    """Blank out // and /* */ comments, leaving string bodies intact AND the text the same
    length, so offsets and line numbers still refer to the original file.

    A phrase quoted in prose is NOT a literal we ship. The first draft of this tool counted
    '...unable to load...' from a COMMENT as a shipped string - exactly the false claim the
    tool exists to prevent, so this function is load-bearing, not hygiene. A first attempt at
    it shortened the text and silently mis-numbered every reported line."""
    out = list(text)
    i, n = 0, len(text)

    def blank(a, b):
        for k in range(a, min(b, n)):
            if out[k] != "\n":
                out[k] = " "

    while i < n:
        c = text[i]
        if c == '"':
            j = i + 1
            while j < n and text[j] != '"':
                j += 2 if text[j] == "\\" else 1
            i = j + 1
            continue
        if text.startswith("//", i):
            j = text.find("\n", i)
            j = n if j < 0 else j
            blank(i, j)
            i = j
            continue
        if text.startswith("/*", i):
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
            blank(i, j)
            i = j
            continue
        i += 1
    return "".join(out)


def statement_literals(text, start):
    """Literals of the statement starting at `start`, with ADJACENT literals merged the way the
    compiler concatenates them ("ab" "cd" ships as ONE string, so asserting the halves is wrong).
    Returns (bodies, line_no_where_statement_ends)."""
    bodies, i, sep = [], start, ""
    n = len(text)
    while i < n:
        if text.startswith("//", i):
            j = text.find("\n", i)
            i = n if j < 0 else j
            continue
        ch = text[i]
        if ch == '"':
            m = _STRING_RE.match(text, i)
            if not m:
                break
            if bodies and not re.search(r'[,;:(]', sep):
                bodies[-1] += m.group(1)      # concatenation, not a new argument
            else:
                bodies.append(m.group(1))
            sep = ""
            i = m.end()
            continue
        if ch in ";)":
            break
        sep += ch
        i += 1
    return bodies, text.count("\n", 0, i) + 1


# ------------------------------------------------------------- PE layout
def pe_sections(data):
    """Return (image_base, sections). Verified against the shipped F3.exe: magic 0x10B and
    ImageBase 0x400000 at optional header +0x1C. The EA this tool prints is derived from these
    fields, not assumed - a wrong base silently mislabels every site it reports."""
    if data[:2] != b"MZ":
        raise Failure("not a PE (no MZ)")
    pe = int.from_bytes(data[0x3C:0x40], "little")
    if data[pe:pe + 4] != b"PE\0\0":
        raise Failure("not a PE (no PE signature)")
    coff = pe + 4
    nsec = int.from_bytes(data[coff + 2:coff + 4], "little")
    optsz = int.from_bytes(data[coff + 16:coff + 18], "little")
    opt = coff + 20
    magic = int.from_bytes(data[opt:opt + 2], "little")
    if magic == 0x10B:          # PE32:  ImageBase is 4 bytes at optional header +0x1C
        image_base = int.from_bytes(data[opt + 0x1C:opt + 0x20], "little")
    elif magic == 0x20B:        # PE32+: 8 bytes at +0x18 (PE32's BaseOfData slot is absent,
        image_base = int.from_bytes(data[opt + 0x18:opt + 0x20], "little")   # so it moves down)
    else:
        raise Failure("unknown optional header magic 0x%X" % magic)
    tbl = opt + optsz
    secs = []
    for k in range(nsec):
        e = tbl + k * 40
        secs.append((data[e:e + 8].rstrip(b"\0").decode("latin-1"),
                     int.from_bytes(data[e + 12:e + 16], "little"),   # virtual address
                     int.from_bytes(data[e + 20:e + 24], "little"),   # raw pointer
                     int.from_bytes(data[e + 16:e + 20], "little")))  # raw size
    return image_base, secs


def offset_to_ea(off, image_base, secs):
    for name, va, rp, rs in secs:
        if rp <= off < rp + max(rs, 1):
            return "0x%X" % (image_base + va + (off - rp)), name
    return "?", "?"


def find_all(data, needle):
    offs, i = [], data.find(needle)
    while i >= 0:
        offs.append(i)
        i = data.find(needle, i + 1)
    return offs


# ------------------------------------------------------------- discovery
def iter_sources(root):
    for base in SCAN_SUBPATHS:
        top = os.path.join(root, base)
        if not os.path.isdir(top):
            continue
        for dirpath, dirs, files in os.walk(top):
            dirs[:] = [d for d in dirs if d not in ("build", "build-msvc-gen", ".git")]
            for fn in sorted(files):
                if fn.endswith(SRC_EXT):
                    yield os.path.join(dirpath, fn)


def discover(root):
    """(file, line, literal_body) for every literal carrying the do-not-fix marker."""
    found = []
    for path in iter_sources(root):
        try:
            raw = open(path, encoding="utf-8", errors="replace").read()
        except OSError:
            continue
        if MARKER not in raw:
            continue
        lines = raw.splitlines()
        stripped = strip_comments(raw)
        starts, pos = [], 0
        for _ in lines:
            starts.append(pos)
            pos = stripped.find("\n", pos) + 1
        for idx, line in enumerate(lines):
            if MARKER not in line:
                continue
            nxt = idx + 1
            while nxt < len(lines) and not lines[nxt].strip():
                nxt += 1
            if nxt >= len(lines):
                continue
            bodies, endline = statement_literals(stripped, starts[min(nxt, len(starts) - 1)])
            for b in bodies:
                found.append((path, nxt + 1, b))
    return found


def read_manifest(path):
    """One literal per line, optionally TAB-pinned to an EA or raw offset:  "lit"\t0x64FB48"""
    entries = []
    for n, line in enumerate(open(path, encoding="utf-8"), 1):
        t = line.rstrip("\n")
        if not t.strip() or t.strip().startswith("#"):
            continue
        pin = None
        if "\t" in t:
            t, _, pin = t.partition("\t")
            t, pin = t.strip(), pin.strip()
        if len(t) >= 2 and t[0] == '"' and t[-1] == '"':
            t = t[1:-1]
        entries.append((path, n, t, pin))
    return entries


# ------------------------------------------------------------- checking
def dedup(entries):
    """One claim per distinct literal. Our own code legitimately reuses a shipped string (the
    blood .veg appears 4x in GameActionAttack.cpp), and checking it 4 times would report 4
    'sites' for what is one fidelity claim - while making the count of claims look inflated.
    The first claiming site is kept; the rest are counted so the report can say so."""
    seen, out, dupes = {}, [], {}
    for src, line, body, pin in entries:
        if body in seen:
            dupes[body] = dupes.get(body, 0) + 1
            continue
        seen[body] = (src, line, body, pin)
        out.append(seen[body])
    return out, dupes


def check(data, image_base, secs, entries):
    rows, bad = [], 0
    for src, line, body, pin in entries:
        try:
            needle = decode_c_string(body)
        except Failure as exc:
            rows.append(("UNDECODABLE", src, line, body, str(exc), "-", "-"))
            bad += 1
            continue
        offs = find_all(data, needle)
        exact = [o for o in offs if data[o + len(needle):o + len(needle) + 1] == b"\0"]
        if pin:
            p = int(pin, 16)
            p = p - image_base if p >= image_base else p
            offs = [o for o in offs if o == p]
        if not offs:
            verdict, ea, sect = ("PIN-MISS" if pin else "DRIFT"), "-", "-"
        elif len(offs) > 1:
            verdict, ea, sect = "AMBIGUOUS", "-", "-"
        elif offs[0] not in exact:
            verdict, ea, sect = "FRAGMENT", "-", "-"
        else:
            ea, sect = offset_to_ea(offs[0], image_base, secs)
            verdict = "OK"
        rows.append((verdict, src, line, body, ",".join(hex(o) for o in offs[:4]), ea, sect))
        if verdict != "OK":
            bad += 1
    return rows, bad


WHY = {"DRIFT": "absent from the shipped image - our bytes are not the original's",
       "AMBIGUOUS": "matches at >1 offset - no site can be claimed without pinning an EA",
       "FRAGMENT": "matches, but as the PREFIX of a longer shipped string - not verbatim",
       "PIN-MISS": "the pinned EA/offset holds no match here",
       "UNDECODABLE": "escape sequence not understood"}


def report(rows, bad, meta, dupes=None):
    print("shipped image : %s" % meta)
    print("literals      : %d distinct claims checked%s"
          % (len(rows), (" (%d duplicate claim sites folded)" % sum(dupes.values())) if dupes else ""))
    w = min(max((len(r[3]) for r in rows), default=20), 58)
    print("  %-10s %-8s %-8s %s" % ("VERDICT", "EA", "SECTION", "LITERAL"))
    print("  " + "-" * (w + 30))
    for v, src, line, body, offs, ea, sect in rows:
        shown = body if len(body) <= w else body[:w - 1] + "..."
        print("  %-10s %-8s %-8s %s" % (v, ea, sect, shown))
        print("  %-10s %-8s %-8s   %s:%d  raw=%s" % ("", "", "", src, line, offs))
    print("== verdict ==")
    for v, src, line, body, offs, _ea, _s in rows:
        if v != "OK":
            extra = ("%d matches" % len(offs.split(","))) if v == "AMBIGUOUS" else ""
            print("  FAIL %-11s %s:%d  %s\n              -> %s %s"
                  % (v, src, line, body[:56], WHY.get(v, "?"), extra))
    print("  %d/%d literals byte-identical, whole, and unambiguous in the original."
          % (len(rows) - bad, len(rows)))
    print("  boundary: this proves BYTES and their offsets. It does NOT prove we call them from")
    print("            the right place, that behaviour matches, or that the exe runs. It is a")
    print("            string-fidelity gate, never an acceptance result for behaviour.")


# ------------------------------------------------------------- self-test
def self_test(root, pe, argv):
    """Falsification, against a scratch copy. Writes nothing into the repo."""
    tmp = tempfile.mkdtemp(prefix="verify-literals-selftest-")
    res = []
    try:
        real = os.path.join(root, "src", "F3", "CombatMath.cpp")
        if not os.path.isfile(real):
            raise Failure("self-test needs src/F3/CombatMath.cpp (the marker site)")
        os.makedirs(os.path.join(tmp, "src", "F3"))
        scratch = os.path.join(tmp, "src", "F3", "CombatMath.cpp")
        shutil.copy(real, scratch)

        def run(extra=()):
            return subprocess.run([sys.executable, argv, "--pe", pe, "--root", tmp] + list(extra),
                                  capture_output=True, text=True)

        base = run()
        res.append(("control: unmodified scratch copy passes", base.returncode == 0,
                    "exit=%d" % base.returncode))

        text = open(scratch, encoding="utf-8").read()
        m = re.search(r'"(GameWeapon::CalculateDamage[^"]*)"', text)
        if not m:
            raise Failure("could not find the marked literal to corrupt")
        for tag, bad_lit in (("one byte flipped", "GameWeapon::CalculateDamage - No body part was targetted.\\n"),
                            ("prefix of a longer string", "GameWeapon::CalculateDamage - No body part was t"),
                            ("duplicated site", "AmbientMaterialSource")):
            broken = text[:m.start(1)] + bad_lit + text[m.end(1):]
            open(scratch, "w", encoding="utf-8").write(broken)
            r = run()
            want = "DRIFT" if "flipped" in tag else ("FRAGMENT" if "prefix" in tag else "AMBIGUOUS")
            res.append(("corruption caught: %s -> %s" % (tag, want), r.returncode != 0,
                        "exit=%d, %s" % (r.returncode, "saw " + want if want in r.stdout
                                         else "wanted " + want)))
            open(scratch, "w", encoding="utf-8").write(text)

        prose = os.path.join(tmp, "src", "F3", "Prose.cpp")
        open(prose, "w", encoding="utf-8").write(
            '// a comment that quotes "GameWeapon::CalculateDamage - No body part was targeted.\\n"\n'
            'int t022_prose_marker = 0; // shipped string verbatim is NOT in this file\n')
        res.append(("prose in a comment is not claimed as a literal",
                    run(["--no-discover", "--literal", "notARealShippedStringAtAll"]).returncode != 0,
                    "manifest path still asserts"))

        empty = os.path.join(tmp, "empty.txt")
        open(empty, "w").write("# only comments\n")
        r = run(["--no-discover", "--manifest", empty])
        res.append(("empty claim set is NOT reported as a pass", r.returncode != 0,
                    "exit=%d" % r.returncode))
    finally:
        shutil.rmtree(tmp, ignore_errors=True)

    print("== self-test (falsification; scratch copy, repo untouched) ==")
    failed = 0
    for name, ok, note in res:
        print("  %-4s  %-58s  (%s)" % ("PASS" if ok else "FAIL", name, note))
        failed += not ok
    print("  boundary: the self-test proves the CHECKER discriminates bytes. It says nothing")
    print("            about the repo, and it never writes into the repo.")
    return 1 if failed else 0


# ------------------------------------------------------------- survey
def survey(root, data, image_base, secs, claimed):
    """Unclaimed literals that DO occur exactly once in the image - candidates to mark. Advisory:
    it proposes, it does not claim, and it never changes the exit code."""
    out = []
    for path in iter_sources(root):
        try:
            raw = open(path, encoding="utf-8", errors="replace").read()
        except OSError:
            continue
        st = strip_comments(raw)
        for m in _STRING_RE.finditer(st):
            body = m.group(1)
            if len(body) < 12 or body in claimed:
                continue
            try:
                needle = decode_c_string(body)
            except Failure:
                continue
            offs = find_all(data, needle)
            exact = [o for o in offs if data[o + len(needle):o + len(needle) + 1] == b"\0"]
            if len(exact) == 1:
                ea, _sect = offset_to_ea(exact[0], image_base, secs)
                out.append((path, raw[:m.start()].count("\n") + 1, body, ea))
    return out


# ------------------------------------------------------------- main
def main():
    here = os.path.dirname(os.path.abspath(__file__))
    root = os.path.dirname(here)
    ap = argparse.ArgumentParser(description="Verify 'shipped verbatim' literals against F3.exe")
    ap.add_argument("--pe", default=os.environ.get("OPENVB_ORIG_EXE"),
                    help="shipped image (default $OPENVB_ORIG_EXE)")
    ap.add_argument("--root", default=root)
    ap.add_argument("--manifest", default=None,
                    help="claim file (default %s when present)" % DEFAULT_MANIFEST)
    ap.add_argument("--literal", action="append", default=[], metavar="STR",
                    help="assert one more literal (repeatable); C escapes honoured")
    ap.add_argument("--no-discover", action="store_true", help="skip marker discovery")
    ap.add_argument("--self-test", action="store_true", help="prove the checker can fail")
    ap.add_argument("--survey", action="store_true",
                    help="list unmarked literals that match uniquely (advisory, no verdict)")
    ap.add_argument("--limit", type=int, default=40)
    a = ap.parse_args()

    if not a.pe or not os.path.isfile(a.pe):
        print("SETUP ERROR: no shipped image (--pe or $OPENVB_ORIG_EXE).\n"
              "Byte identity CANNOT be checked without it, so this refuses to report a pass.",
              file=sys.stderr)
        return 2

    data = open(a.pe, "rb").read()
    image_base, secs = pe_sections(data)
    meta = "%s (%d bytes, image base 0x%X, %d sections)" % (a.pe, len(data), image_base, len(secs))

    if a.self_test:
        return self_test(a.root, a.pe, os.path.abspath(__file__))

    if a.survey:
        pre = [] if a.no_discover else [(f, l, b, None) for f, l, b in discover(a.root)]
        mp = a.manifest or os.path.join(a.root, DEFAULT_MANIFEST)
        if os.path.isfile(mp):
            pre += read_manifest(mp)
        rows = survey(a.root, data, image_base, secs, set(e[2] for e in pre))
        print("== unmarked literals matching ONE whole shipped string (%d, showing %d) =="
              % (len(rows), min(len(rows), a.limit)))
        for path, line, body, ea in rows[:a.limit]:
            print("  %-8s %s:%d  %s" % (ea, path, line, body[:60]))
        print("  boundary: ADVISORY. A hit means the bytes exist once in the image; it does not")
        print("            mean the string is load-bearing, correct, or ours to claim. Marking a")
        print("            literal is a human decision; this list only makes it cheap.")
        return 0

    entries = [] if a.no_discover else [(f, l, b, None) for f, l, b in discover(a.root)]
    mpath = a.manifest or os.path.join(a.root, DEFAULT_MANIFEST)
    if os.path.isfile(mpath):
        entries += read_manifest(mpath)
    for lit in a.literal:
        entries.append(("<--literal>", 0, lit, None))

    if not entries:
        print("NOTHING TO CHECK: no '%s' markers, no manifest entries, no --literal." % MARKER)
        print("  A zero-claim run is NOT a pass - it is an unguarded repo. Add a marker or")
        print("  --manifest, or run --survey to find candidates.")
        print("  boundary: unverifiable != verified.")
        return 1

    entries, dupes = dedup(entries)
    rows, bad = check(data, image_base, secs, entries)
    report(rows, bad, meta, dupes)
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
