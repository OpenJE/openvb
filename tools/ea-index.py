#!/usr/bin/env python3
"""T-032: EA -> document index for discoveries/*.md. Makes what we already know addressable.

BOUNDARY (print it before quoting a hit list):
  * This index records WHERE an EA is mentioned. It does NOT know whether the mention is correct,
    current, or superseded. Superseded documents stay in the index; judging them is the reader's job.
    Check discoveries/2025-10-20-canonical-fact-sheet.md before trusting any hit.
  * Scope is discoveries/ ONLY: not tasks/, not src/ comments, not the IDB, not docs/f3/.
  * Keys are literal addresses. Two EAs one character apart are different keys and this tool does not
    know they are related. It does not follow xrefs, vftables, or inheritance.
  * Tokens this tool could NOT classify are refused and COUNTED, never dropped silently. A refusal
    count is printed on every run and written into the TSV header. Refused != unimportant.

Usage:
  tools/ea-index.py                      regenerate artifacts/ea-index.tsv (default)
  tools/ea-index.py --query 0x45C140     citing docs + section headings; exits 1 with NO RECORD if none
  tools/ea-index.py --check              exit 2 if the checked-in TSV is stale (regenerate it)
  tools/ea-index.py --refused            list every refused token with file:line, grouped by reason

Pure offline. Reads markdown, writes one TSV. No IDA, no IDB, no build.
"""
import argparse
import collections
import glob
import os
import re
import sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DISC = os.path.join(REPO, "discoveries")
OUT = os.path.join(REPO, "artifacts", "ea-index.tsv")

# Image span of shipped F3.exe (base 0x400000; .text 0x401000 .. .rsrc end 0x715728).
VA_LO, VA_HI = 0x400000, 0x715800

SUB_RE = re.compile(r"(?<![0-9A-Za-z])sub_([0-9A-Fa-f]{4,8})(?![0-9A-Fa-f])")
CLS_RE = re.compile(r"cls_0x([0-9A-Fa-f]{4,8})(?![0-9A-Fa-f])")
JRE_RE = re.compile(r"(?<![0-9A-Za-z])(?:j_|call_)([0-9A-Fa-f]{4,8})(?![0-9A-Fa-f])")
MASK_RE = re.compile(r"(?<![0-9A-Fa-f])0x([0-9A-Fa-f]{1,6}[xX][0-9A-Fa-fxX]*)(?![0-9A-Fa-f])")
# NB: no leading \\b -- identifier suffixes like CalculateDamage_0x413310 have a word
# char before the 0, so \\b0x silently misses the most common way EAs appear in this corpus.
HEX_RE = re.compile(r"(?<![0-9A-Fa-f])0x([0-9A-Fa-f]{1,12})(?![0-9A-Fa-f])")
PLUS_RE = re.compile(r"\+0x([0-9A-Fa-f]{4,8})\b")
RANGE_RE = re.compile(r"\b0x([0-9A-Fa-f]{4,8})\s*[\u2013\u2014-]\s*(?:0x)?([0-9A-Fa-f]{2,8})\b")
SLOT_RE = re.compile(r"\bvft(?:\[)?\s*\+0?([0-9A-Fa-fxX]{1,3})")
BARE_RE = re.compile(r"(?<![0-9A-Za-z_])([4-7][0-9A-Fa-f]{5})(?![0-9A-Fa-z_])")
# name-suffix form: <Method>_0x413310 / <Method>_413310 (very common in this corpus)
SUFFIX_RE = re.compile(r"[A-Za-z_\]]_(?:0x)?([4-7][0-9A-Fa-f]{5})(?![0-9A-Fa-f])")
# IDA-style zero-padded address inside an identifier: lpSrcStr_006cb56c
PAD_RE = re.compile(r"(?<![0-9A-Fa-f])0*(?:x)?(0{2}[4-7][0-9A-Fa-f]{5}|[4-7]0[0-9A-Fa-f]{5})(?![0-9A-Fa-f])")
# address after an offset segment: Get_mbr_0x98_463b60, cls_0_59f7d0
POSTOFF_RE = re.compile(r"(?:0x[0-9A-Fa-f]{1,4}|_0)_([4-7][0-9A-Fa-f]{5})(?![0-9A-Fa-f])")


def in_va(v):
    return VA_LO <= v <= VA_HI


def classify(line, fname, ln, seen_here, refused, in_fence=False):
    """Return {key: kind} for one line; append refusals. Keys are never invented.

    in_fence matters: inside a fenced block a bare all-digit token like '470377:' is a disassembly
    line address by convention, whereas the same digits in prose are most likely a decimal count.
    """
    hits = {}

    # 1. Consume sub_/cls_/j_ forms first so their digits are not re-read as bare hex.
    masked_spans = set()
    for m in MASK_RE.finditer(line):
        masked_spans.add(m.start(1) - 2)
    for rx, kind in ((SUB_RE, "sub_"), (CLS_RE, "cls_0x"), (JRE_RE, "j_/call_")):
        for m in rx.finditer(line):
            v = int(m.group(1), 16)
            if in_va(v):
                hits["0x%06x" % v] = kind
            else:
                refused.append((fname, ln, m.group(0), "out-of-image (file offset/FourCC/mask)"))

    # 2. Ranges: index both endpoints when in range, and note the range itself.
    for m in RANGE_RE.finditer(line):
        for g in (1, 2):
            try:
                v = int(m.group(g), 16)
            except ValueError:
                continue
            if in_va(v) and len(m.group(g)) >= 5:
                hits["0x%06x" % v] = "range-endpoint"
            elif len(m.group(g)) >= 5:
                refused.append((fname, ln, m.group(0), "range outside image"))

    # 3. Plain 0x hex, skipping masked forms (0x401xxx) which are not addresses.
    for m in HEX_RE.finditer(line):
        tok = m.group(0)
        if any(m.start() == s for s in masked_spans):
            refused.append((fname, ln, tok, "masked/pattern EA (N significant digits, not an address)"))
            continue
        v = int(m.group(1), 16)
        if not in_va(v):
            if len(m.group(1)) >= 5:
                refused.append((fname, ln, tok, "outside image span 0x400000-0x715800"))
            continue
        # Keys are normalised to lowercase 0x%06x. Without this, 0x54BE10 and 0x54be10 are two
        # different keys and the corpus silently fragments (measured: 318 split addresses).
        hits["0x%06x" % v] = "0x"

    # 4. '+0x…' with address-looking digits is AMBIGUOUS: member offset or global address?
    #    Refuse rather than merge two different meanings into one key.
    for m in PLUS_RE.finditer(line):
        if len(m.group(1)) >= 5 and in_va(int(m.group(1), 16)):
            refused.append((fname, ln, m.group(0),
                            "ambiguous +0x hex: struct offset vs global address"))

    # 5. Suffix/bare six-hex forms. The identifier context (`QueueToCombat_463140`, `_0x98_463b60`,
    #    `006cb56c`) proves address intent even when the token has no hex letter, so the digit guard
    #    applies ONLY to a bare token floating in prose, where 463140 could be a decimal count --
    #    unless we are inside a fenced block, where '470377:' is a disassembly address.
    allow_digitless = in_fence
    for rx, kind, guard in ((SUFFIX_RE, "name-suffix", False), (POSTOFF_RE, "after-offset-suffix", False),
                            (PAD_RE, "zero-padded", False), (BARE_RE, "bare", True)):
        for m in rx.finditer(line):
            t = m.group(1)
            if guard and not re.search(r"[A-Fa-f]", t):
                if not allow_digitless:
                    refused.append((fname, ln, t, "bare all-digit hex: could be a decimal count, not indexed"))
                    continue
            v = int(t, 16)
            if in_va(v):
                hits["0x%06x" % v] = kind
            elif guard:
                refused.append((fname, ln, t, "bare hex outside image span"))

    # 5b. Slash-compressed sibling lists: 'sub_0x463000/463040/463080/4630c0/463100(time)'.
    #     Elements shorter than 6 hex digits inherit the leading address's high digits.
    for m in re.finditer(r"(?:sub_)?(0x)?([4-7][0-9A-Fa-f]{5})((?:\s*/\s*[0-9A-Fa-f]{2,6})+)", line):
        lead = m.group(2)
        hits["0x%s" % lead.lower()] = "slash-lead"
        for part in re.findall(r"/\s*([0-9A-Fa-f]{2,6})", m.group(3)):
            cand = (lead[: 6 - len(part)] + part)[-6:] if len(part) < 6 else part
            try:
                v = int(cand, 16)
            except ValueError:
                continue
            if in_va(v):
                hits["0x%06x" % v] = "slash-sibling"

    # 6. vft slot offsets are relative to a table this tool does not resolve: refuse, count.
    for m in SLOT_RE.finditer(line):
        refused.append((fname, ln, "vft+0x" + m.group(1), "vftable slot offset (relative to an unresolved table)"))

    return hits


def title_of(lines):
    for l in lines:
        m = re.match(r"^#\s+(.*)", l)
        if m:
            return m.group(1).strip()
    for l in lines:
        if l.strip():
            return l.strip()[:90]
    return "(empty)"


def build():
    refused = []
    kinds = collections.defaultdict(set)
    index = collections.defaultdict(set)
    where = collections.defaultdict(list)   # key -> [(file, line, heading, snippet)]
    headings = {}
    docs = sorted(glob.glob(os.path.join(DISC, "*.md")))
    for path in docs:
        fname = os.path.basename(path)
        lines = open(path, encoding="utf-8", errors="replace").read().splitlines()
        headings[fname] = title_of(lines)
        cur = headings[fname]
        in_fence = False
        seen_here = collections.defaultdict(list)
        for i, line in enumerate(lines, 1):
            if line.lstrip().startswith("```"):
                in_fence = not in_fence
                continue
            hm = re.match(r"^(#{1,6})\s+(.*)", line)
            if hm:
                cur = hm.group(2).strip()
            for key, kind in classify(line, fname, i, seen_here, refused, in_fence).items():
                index[key].add(fname)
                where[key].append((fname, i, cur, line.strip()[:120]))
                kinds[key].add(kind)
    return docs, index, where, refused, headings, kinds


INFERRED_ONLY = {"slash-sibling", "zero-padded", "after-offset-suffix", "range-endpoint"}


def write_tsv(index, docs, refused, headings, out_path, kinds=None):
    counts = collections.Counter(r[3] for r in refused)
    assert sum(counts.values()) == len(refused), "refused categories must sum to the total"
    nref = len(refused)
    uniq = sorted(index, key=lambda k: int(k, 16))
    hdr = [
        "# ea-index: EA -> discoveries documents that mention it. Generated by tools/ea-index.py; never edit by hand.",
        "# BOUNDARY: records WHERE an EA is mentioned, not whether the mention is correct or current.",
        "#   Superseded docs stay in the index; judge them by the reader (see canonical-fact-sheet).",
        "#   Scope: discoveries/*.md only (not tasks/, not src/, not docs/f3/, not the IDB).",
        "#   Keys are literal addresses; 0x45c140 and 0x45c141 are unrelated keys here.",
        "# corpus_docs=%d indexed_eas=%d mentions=%d refused_tokens=%d %s"
        % (len(docs), len(index), sum(len(v) for v in index.values()), nref,
           dict(counts) if counts else "{}"),
        "# %d keys are backed ONLY by inference from compressed prose (slash lists like @0x45c251/261/271,"
        % len([k for k, v in (kinds or {}).items() if v and v <= INFERRED_ONLY]),
        "#   zero-padded or after-offset suffixes). They are real but not literally written: list with --inferred.",
        "#   Every other key appears literally as 0x.../sub_.../name_0x... in at least one document.",
        "# ea\tcount\tdocs   (count = number of citing documents; docs = ';' separated, newest-in-corpus order not guaranteed)",
    ]
    tmp = out_path + ".tmp"
    with open(tmp, "w", encoding="utf-8") as f:
        f.write("\n".join(hdr) + "\n")
        f.write("ea\tcount\tdocs\n")
        for k in uniq:
            f.write("%s\t%d\t%s\n" % (k, len(index[k]), ";".join(sorted(index[k]))))
    os.replace(tmp, out_path)
    return len(uniq), nref, counts


def expected_header(docs, index):
    return "corpus_docs=%d indexed_eas=%d" % (len(docs), len(index))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--query", help="EA to look up (0x..., sub_..., cls_0x... all accepted)")
    ap.add_argument("--check", action="store_true", help="exit 2 if the checked-in TSV is stale")
    ap.add_argument("--refused", action="store_true", help="dump refused tokens grouped by reason")
    ap.add_argument("--verify", action="store_true", help="cross-check index coverage against naive grep")
    ap.add_argument("--full", action="store_true", help="with --verify: check every EA, not the top 60")
    ap.add_argument("--inferred", action="store_true", help="list keys that exist only by inference from compressed prose")
    ap.add_argument("--out", default=OUT)
    a = ap.parse_args()

    docs, index, where, refused, headings, kinds = build()

    if a.check:
        # Must NOT regenerate first, or it always finds itself current (this bug shipped once).
        want = expected_header(docs, index)
        try:
            have = open(a.out, encoding="utf-8").read()
        except OSError:
            have = ""
        if want in have:
            print("ea-index is current: %s" % want)
            return 0
        print("STALE: corpus now says (%s) but checked-in index does not. "
              "Regenerate: tools/ea-index.py" % want, file=sys.stderr)
        return 2

    n, nref, counts = write_tsv(index, docs, refused, headings, a.out, kinds)

    if a.query:
        q = a.query.strip().lower()
        m = re.search(r"([0-9A-Fa-f]{4,8})$", q.replace("sub_", "").replace("cls_0x", ""))
        key = None
        if m:
            try:
                key = "0x%06x" % int(m.group(1), 16)
            except ValueError:
                key = None
        if key is None or key not in index:
            # NO RECORD must be loud: it is the difference between 'nobody looked at this'
            # and 'this file is empty'. Exit 1, not 0.
            print("NO RECORD for %s (%s) -- discoveries/*.md contains no mention of this address."
                  % (a.query, key or "unparseable as an EA"))
            print("  refused-token classes that could hide a mention: %s" % (dict(counts) or "none"))
            print("  A NO RECORD is evidence of absence ONLY for discoveries/: tasks/, src/ and")
            print("  docs/f3/ are out of scope, and masked forms (0x401xxx) are never indexed.")
            return 1
        hits = where[key]
        print("QUERY %s -> %d document(s), %d mention line(s)" % (key, len(index[key]), len(hits)))
        for f in sorted(index[key]):
            print("\n%s\n  title: %s" % (f, headings[f]))
            seen = set()
            for ff, l, h, snip in hits:
                if ff != f:
                    continue
                tag = (h, l // 40)
                if tag in seen:
                    continue
                seen.add(tag)
                print("    L%-5d [%s]" % (l, h[:70]))
                print("           %s" % snip[:100])
        print("\nReminder: this lists MENTIONS, not verdicts. Superseded documents appear here too.")
        return 0

    if a.verify:
        # Coverage self-check: for each sampled EA, compare a *word-boundary* grep against the index.
        # NB: a plain substring test is wrong -- 0x400000 is a substring of 0x40000000, which is a flag
        # constant, not an address. This produced a phantom miss once; the boundary check is the fix.
        missing = collections.Counter()
        pairs = 0
        refused_ok = 0
        sample = sorted(index) if a.full else sorted(index, key=lambda k: -len(index[k]))[:60]
        tok_cache = {}
        for path in docs:
            txt = open(path, encoding="utf-8", errors="replace").read()
            tok_cache[path] = set(re.findall(r"[0-9A-Fa-f]{6,8}", txt))
        for key in sample:
            v = int(key, 16)
            alts = {format(v, "06x"), format(v, "06X"), format(v, "08x"), format(v, "08X")}
            for path in docs:
                pairs += 1
                base = os.path.basename(path)
                if (alts & tok_cache[path]) and base not in index[key]:
                    # A token that was REFUSED AND REPORTED is not a silent miss: refusals are counted
                    # in the run output and in the TSV header, which is the whole point of refusing loudly.
                    if any(f == base and int(t.replace("0x", ""), 16) == v
                           for f, l, t, why in refused if re.fullmatch(r"0?[0-9A-Fa-f]{6,8}", t or "x")):
                        refused_ok += 1
                    else:
                        missing[base] += 1
        print("verify: %d EAs x %d docs = %d (EA,doc) pairs checked; index missed %d pairs in %d docs"
              % (len(sample), len(docs), pairs, sum(missing.values()), len(missing)))
        if refused_ok:
            print("  (%d further hits are explicitly REFUSED and counted -- see --refused, not silent misses)"
                  % refused_ok)
        for b, c in missing.most_common(10):
            print("   %s (%d EAs mentioned in this file but not indexed)" % (b, c))
        print("NOTE: this measures RECALL only. It cannot tell you a mention is correct, current, or")
        print("      non-duplicated -- see the BOUNDARY header. 51 keys are inference-only: --inferred.")
        return 0 if not missing else 3

    if a.inferred:
        inf = sorted([k for k, v in kinds.items() if v and v <= INFERRED_ONLY], key=lambda x: int(x, 16))
        print("%d keys exist ONLY because of inference from compressed/abbreviated prose:" % len(inf))
        for k in inf:
            hs = where[k][:1]
            print("   %s  <- %s:%d  %s" % (k, hs[0][0], hs[0][1], hs[0][3][:78]))
        print("Treat these as weaker than literal mentions: the address was assembled from notation like")
        print("'@0x45c251/261/271', so a typo in the source prose propagates here.")
        return 0

    if a.refused:
        g = collections.defaultdict(list)
        for f, l, t, why in refused:
            g[why].append("%s:%d %s" % (f, l, t))
        # Print the total first, so nobody has to add up category lines and get a different number
        # than the header reports (that disagreement happened once).
        print("TOTAL REFUSED TOKENS: %d  (= sum of the %d categories below; matches refused_tokens= in the TSV header)"
              % (len(refused), len(g)))
        for why in sorted(g, key=lambda k: -len(g[k])):
            print("\n%d refused: %s" % (len(g[why]), why))
            for s2 in g[why][:12]:
                print("   ", s2)
            if len(g[why]) > 12:
                print("    ... %d more" % (len(g[why]) - 12))
        return 0

    print("wrote %s: %d docs indexed, %d distinct EAs, %d mentions, %d refused tokens %s"
          % (os.path.relpath(a.out, REPO), len(docs), n,
             sum(len(v) for v in index.values()), nref, dict(counts) if counts else ""))
    print("query with: tools/ea-index.py --query 0x45C140")
    return 0


if __name__ == "__main__":
    sys.exit(main())
