#!/usr/bin/env python3
# T-031 - tools/orig-digest.py : emit the SHIPPED side of the simulation digest in the SAME line
# grammar as src/F3/SimDigest.hpp, so tools/diff-digest.sh stays the only comparator in the project
# (one comparator, two producers - docs/f3/ParityOracleDesign.md §4).
#
# WHAT THIS IS NOT, printed in the artifact it writes: it does not read a running process. It renders
# a CAPTURE FILE - the dwords a lockstep session wrote down - into digest grammar. If the capture is
# not a real lockstep capture, the emitted file says `# SYNTHETIC`, and a synthetic side may not be
# compared against a real one (the comparator refuses; see MIXED-PROVENANCE below).
#
# PROVENANCE IS REQUIRED INPUT, not a default. `--provenance` (or a `record=provenance` line in the
# capture) must be present or nothing is written. A provenance may only be recorded as a real
# capture when it names the reads it used and identifies the binary:
#
#   lockstep <reads> session=<id> exe=<md5>      e.g.  lockstep r1,r3,r4,r4b session=41 exe=A68C...
#
# Anything else (including the literal word `synthetic`) marks the output SYNTHETIC. `lockstep` is
# NOT accepted on assertion: it is verified against the capture's own records (R1 -> lcgstate=,
# R3 -> q=, R4b -> vt=) and against artifacts/F3.json's filemd5, and refused otherwise. That guard
# is SLOPPY-PROOF, NOT FORGERY-PROOF: the md5 is public. What it prevents is the failure mode that
# actually happened here - a fixture quietly compared as if it were evidence.
#
# Exit codes: 0 wrote a digest | 2 usage/IO | 3 PROVENANCE missing or unverifiable |
#             4 INVARIANT violated by the capture (nothing is written).
#
# LIMITS, stated here because a tool that hides its own limits is how a stale claim gets quoted:
#  * It renders what a capture says. A memory read sees what the shipped code STORED, not what it
#    intended; presentation-side fields will differ harmlessly and identical values on a path we
#    never reach prove nothing (the row's own boundary).
#  * The contract (the `# ...` header block) is a COPY of the ours-side text, maintained in two
#    places on purpose. Drift is not silently possible: diff-digest.sh compares `#` lines and
#    reports `CONTRACT differs`, and tools/orig-digest-verify.sh asserts byte-equality against a
#    digest produced by the real binary. Auto-extracting the C string literals was rejected: one
#    reformat of WriteHeader would turn the emitter into a silent garbage producer.
#  * `%.2f` is formatted by Python here and by MSVC's printf on our side. Both round the same way
#    for values a capture can contain, but a divergence in `pos`/`yaw` alone should be read as a
#    formatting question first, not a state question.

import json
import os
import sys

USAGE = (
    "usage: tools/orig-digest.sh --capture FILE --out FILE [--provenance TEXT]\n"
    "                          [--db artifacts/F3.json] [--require-resolved-ak]\n"
    "                          [--print-classes]\n"
)

EXIT_IO = 2
EXIT_PROVENANCE = 3
EXIT_INVARIANT = 4


def die(code, *lines):
    sys.stderr.write("\n".join(lines) + "\n")
    sys.exit(code)


# --------------------------------------------------------------------------------------------
# the contract block. Kept as data, in the ours-side order, so `head -1` answers "what is in
# here" from either file. Text mirrors src/F3/SimDigest.hpp WriteHeader field-for-field.
# --------------------------------------------------------------------------------------------
CONTRACT = (
    "# SIM-DIGEST v1 include=frame clockDelta ents <eN=name,pos,yaw,hp,st,ak,qi> "
    "pathJobs turns turnOrder draws\n"
    "# entity-fields=name pos[3] yaw hp=cur/max st=entityState,moving,acted q=QUEUED ACTION COUNT "
    "(shipped Entity+0x314, the action deque's ELEMENT COUNT - ++ at 0x54ea71, -- at "
    "0x54b637/0x54b672, never a 0/1 store; our side derives it from actions[] so it can be at most "
    "4, and a shipped q above 4 beside our q=4 is deviation D-2, i.e. a DROPPED ORDER, not a "
    "digest bug - T-039 verdict, harness item 22; it is printed as a COUNT and was never a busy "
    "flag, which is why it left the st= triple) "
    "qi=EFFECT-INBOX COUNT (shipped Entity+0x2F0, the effect deque's live count - the "
    "field virt_meth_0x54ab00's busy predicate reads as != 0, T-074 §4e; our side derives it "
    "from inbox.fx[] - inbox-era state since P-CMBT steps 1-2 route damage through the inbox; "
    "the shipped side renders NA:absent-in-capture when its capture predates the inbox era, "
    "and a lockstep capture carrying numbered qi must name an r5 read) "
    "ak=ACTION CLASS NAMES: our side maps port kind->class in code, the original side maps "
    "[action+0x00] vftptr->class via artifacts/F3.json, UNKNOWN_VT=<ea> when unresolved and those "
    "are counted, never named | cap 12 entities, excess -> entsMore=1\n"
    "# COMPARED=frame clockDelta ents eN.name/pos/yaw/hp/st/q/ak/qi pathJobs turns turnOrder draws | "
    "CONTEXT(after ' ctx:')=tickMs clock (ABSOLUTE) - printed, captured, NOT hashed: uptime and the "
    "shipped first-Update leap make both incomparable across processes, so the digest carries "
    "clockDelta (seconds elapsed since frame 0) as the comparable axis instead; promote a CONTEXT "
    "field in the comparator, never by re-running the session | caller-supplied=clock tickMs - a "
    "headless harness owns simulated time; the app path passes Host::TickMilliseconds (timeGetTime, "
    "machine uptime) and the world clock Gregorian::TotalSeconds, which STARTS AT 0 and grows by "
    "Update(ms)-last (mbr_0x48 initialises to 0 against an ABSOLUTE tick, so the first pump adds "
    "the whole uptime - see docs/f3/SimDigest.md); cross-context comparisons must pin both\n"
    "# EXCLUDED=reaction-anim-id gfx-texture-slots gui-fragments voice-roll effect-rolls "
    "(presentation-side)\n"
    "# WHY-draws-IS-IN: the reaction id stays out, but one global sequential LCG feeds both - 8 of "
    "the 18 shipped draw sites sit in sub_45C140 (docs/f3/CombatRngSeam.md) - so damage-draw COUNT "
    "shifts the reaction outcome. Counting draws is what keeps that coupling named instead of "
    "implicit; excluding the reaction id alone would be an unnamed coupling, not a separation.\n"
    "# NA-means=the seam is not ported, NEVER =unchanged\n"
    "# SEED-LIMIT: a parity run pins ONE captured LCG state, so it proves SAME-STATE parity, NOT "
    "same-seed-from-clock parity: identical under a pinned state and divergent in real time is a "
    "possible and meaningful outcome\n"
    "# PROVES=self-consistency of our sim. DOES NOT PROVE=parity with F3.exe (needs T-028 + a "
    "bootable exe).\n"
)

# Side identity and provenance are ANNOTATION, not contract: the comparator must be able to tell
# "different measurement" (contract) from "different side of the same measurement" (annotation).
ANNOTATION_PREFIXES = (
    "# SIDE", "# PROVENANCE", "# PROVENANCE-CLASS", "# SYNTHETIC", "# CAPTURE",
    "# AK-RESOLUTION", "# ORIG-LCG", "# LIMIT", "# GENERATED", "# SOURCES",
)

OUR_SIDE_CONTRACT_TAIL = "# SOURCES combat=%s paths=%s\n"


def fnv1a(s):
    """FNV-1a/32 over the COMPARED span, matching F3::Sim::Fnv1a (h ^= byte, then h *= 16777619,
    both mod 2**32). Written with explicit masking because the ours side hashes a byte string in
    C and this has to produce the same eight hex digits."""
    h = 2166136261
    for b in s.encode("latin-1", "replace"):
        h = ((h ^ b) * 16777619) & 0xFFFFFFFF
    return h


# --------------------------------------------------------------------------------------------
# vftptr -> class, through the F3 object database. This is R4b's offline half: the debugger
# session reads `[action+0x00]`; the NAME is decided here, from the DB, never invented.
# --------------------------------------------------------------------------------------------
class VftMap:
    def __init__(self, db_path):
        try:
            with open(db_path, "r", encoding="utf-8", errors="replace") as fh:
                data = json.load(fh)
        except (OSError, ValueError) as exc:
            die(EXIT_IO, "orig-digest: cannot read the class DB %s: %s" % (db_path, exc))
        self.md5 = (data.get("filemd5") or "").upper()
        self.by_ea = {}
        for key, struct in (data.get("structures") or {}).items():
            name = struct.get("demangled_name") or key
            for vt_ea, vt in (struct.get("vftables") or {}).items():
                base = _num(vt_ea)
                # A vftptr is the address of the FIRST entry. The DB records the vftable's own ea
                # plus a `vftptr` offset, so the stored pointer for THIS class is base + offset.
                off = _num(vt.get("vftptr")) or 0
                addr = base + off
                prev = self.by_ea.get(addr)
                if prev is None or (prev[0].startswith("cls_") and not key.startswith("cls_")):
                    self.by_ea[addr] = (name, key)
        self.size = len(self.by_ea)

    def lookup(self, ea):
        return self.by_ea.get(ea)


def _num(v):
    if v is None:
        return None
    if isinstance(v, int):
        return v
    try:
        t = str(v).strip()
        return int(t, 16) if t.lower().startswith("0x") else int(t, 10)
    except ValueError:
        return None


# --------------------------------------------------------------------------------------------
# capture parsing + the invariants a capture must satisfy before it may become a digest.
# --------------------------------------------------------------------------------------------
RECORD_KEYS = {
    "provenance": ("provenance",),
    # A lockstep session records address / value / instant / breakpoint - the same four columns
    # T-034's acceptance demands. Making them a RECORD TYPE is what lets a `lockstep` claim be
    # checked instead of believed: the named reads must each have one, and its value must show up
    # in the frames the capture renders. Sloppy-proof, not forgery-proof (the md5 is public) - but
    # "I assert this was captured" stops being sufficient, which is the whole point.
    "read": ("read", "address", "value", "when", "breakpoint"),
    "frame": ("frame", "tickms", "clocksec", "lcgstate"),
    "entity": ("frame", "slot", "name", "pos", "yaw", "hpcur", "hpmax",
               "state", "moving", "acted", "q", "vt"),
    # R-PORT-Q: qi (the effect-inbox count, Entity+0x2F0) is OPTIONAL in captures - older
    # ones predate the inbox era; a capture that carries it as a number must name the r5
    # read (the lockstep guard enforces that in classify_provenance).
    "OPTIONAL": {"entity": ("qi",)},
    "world": ("frame", "pathjobs", "turns", "turnorder"),
    "end": ("frames",),
}


def parse_capture(path):
    try:
        with open(path, "r", encoding="utf-8", errors="replace") as fh:
            raw = fh.readlines()
    except OSError as exc:
        die(EXIT_IO, "orig-digest: cannot read capture %s: %s" % (path, exc))
    recs = []
    for ln, line in enumerate(raw, 1):
        text = line.strip()
        if not text or text.startswith("#"):
            continue
        fields = {}
        for tok in text.replace("\t", " ").split(" "):
            if not tok:
                continue
            if "=" not in tok:
                die(EXIT_INVARIANT,
                    "INVARIANT-VIOLATION: capture line %d is not key=value: %r" % (ln, text))
            k, v = tok.split("=", 1)
            fields[k] = v
        kind = fields.pop("record", None)
        if kind is None:
            die(EXIT_INVARIANT,
                "INVARIANT-VIOLATION: capture line %d has no record= : %r" % (ln, text))
        if kind not in RECORD_KEYS:
            # Grammar drift in a capture is a fixture bug, and a silently-ignored unknown record
            # type is how a capture full of typos would produce a clean-looking digest.
            die(EXIT_INVARIANT,
                "INVARIANT-VIOLATION: capture line %d: unknown record=%s (known: %s)"
                % (ln, kind, ",".join(sorted(RECORD_KEYS))))
        allowed = set(RECORD_KEYS[kind]) | set(RECORD_KEYS.get("OPTIONAL", {}).get(kind, ()))
        unknown = sorted(set(fields) - allowed)
        if unknown:
            die(EXIT_INVARIANT,
                "INVARIANT-VIOLATION: capture line %d: record=%s has unknown field(s) %s"
                % (ln, kind, ",".join(unknown)))
        if kind == "entity":
            opt = set(RECORD_KEYS.get("OPTIONAL", {}).get(kind, ()))
            for req in RECORD_KEYS["entity"]:
                if req not in fields and req not in opt:
                    die(EXIT_INVARIANT,
                        "INVARIANT-VIOLATION: capture line %d: record=entity missing %s"
                        % (ln, req))
        fields["__line"] = ln
        recs.append((kind, fields))
    return recs


def _int(rec, key, what):
    v = rec.get(key)
    try:
        return int(v, 0) if str(v).lower().startswith("0x") else int(v)
    except (TypeError, ValueError):
        die(EXIT_INVARIANT, "INVARIANT-VIOLATION: %s: %s=%r is not a number" % (what, key, v))


def _pos(v):
    parts = str(v).split("/")
    if len(parts) != 3:
        die(EXIT_INVARIANT, "INVARIANT-VIOLATION: pos=%r is not x/y/z" % (v,))
    try:
        return [float(p) for p in parts]
    except ValueError:
        die(EXIT_INVARIANT, "INVARIANT-VIOLATION: pos=%r has a non-numeric component" % (v,))


def classify_provenance(text, recs, db):
    """Returns (class, reason). `lockstep` must EARN its label from the capture's own records."""
    if not text:
        return None, "no provenance supplied"
    lowered = " ".join(text.strip().lower().split())
    if not lowered.startswith("lockstep"):
        return "synthetic", "provenance does not begin with `lockstep`"

    named = set()
    for chunk in lowered.replace(",", " ").split():
        # r1 / r3 / r4 / r4b - the DB labels T-034 uses include the letter suffix, so "r4b" IS a
        # read name. Dropping it here made every honest r4b claim look unverifiable.
        if chunk and chunk[0] == "r" and chunk[1:].rstrip("abcdefghijklmnopqrstuvwxyz").isdigit() \
           and chunk[1:].rstrip("abcdefghijklmnopqrstuvwxyz") != "":
            named.add(chunk)
    # r3 (the deque count behind `q=`) and r4b (the action vftptrs behind `ak=`) are the two reads
    # THIS artifact depends on; a claim without them cannot produce a shipped-side digest at all.
    # r5 (the effect-inbox count behind `qi=`, R-PORT-Q) joins the requirement ONLY when the
    # capture actually carries numbered qi= fields - pre-inbox-era captures stay renderable.
    carry_qi = any(str(r.get("qi", "")).isdigit() for k, r in recs if k == "entity")
    need = ({"r3", "r4b"} | ({"r5"} if carry_qi else set())) - named
    if need:
        return None, ("claims lockstep but does not name %s - the digest's q=/ak= columns come "
                      "from r3/r4b, and numbered qi= fields come from r5"
                      % ",".join(sorted(need)))
    reads = {}
    for kind, r in recs:
        if kind == "read":
            rn = str(r.get("read", "")).strip().lower()
            if rn and rn not in reads:
                reads[rn] = r
    missing = sorted(named - set(reads))
    if missing:
        return None, ("claims lockstep with no `record=read` evidence for %s (address / value / "
                      "when / breakpoint are the columns T-034 records; without them the label is "
                      "an assertion)" % ",".join(missing))
    # cross-check each claimed read against what the capture actually renders
    lcg = set(str(r.get("lcgstate", "")).lower().lstrip("0x") for k, r in recs if k == "frame")
    qs = set(str(r.get("q", "")).lower() for k, r in recs if k == "entity")
    qis = set(str(r.get("qi", "")) for k, r in recs if k == "entity")
    vts = set()
    for k, r in recs:
        if k == "entity":
            for v in str(r.get("vt", "")).split(","):
                n = _num(v)
                if n is not None:
                    vts.add("0x%x" % n)
    disagree = []
    for rn, rec in reads.items():
        val = _num(rec.get("value"))
        if val is None:
            disagree.append("%s has value=%r, not an address/number" % (rn, rec.get("value")))
            continue
        hexval = "0x%x" % val
        if rn == "r3" and str(val) not in qs:
            disagree.append("r3 value %s appears in no q= field" % hexval)
        elif rn == "r5" and str(val) not in qis:
            disagree.append("r5 value %s appears in no qi= field" % hexval)
        elif rn == "r4b" and hexval not in vts:
            disagree.append("r4b value %s appears in no vt= field" % hexval)
        elif rn == "r1" and hexval.lstrip("0x") not in {x.lstrip("0x") for x in lcg if x}:
            disagree.append("r1 value %s appears in no frame's lcgstate=" % hexval)
    if disagree:
        return None, "claims lockstep but its read records and its frames disagree: " + "; ".join(disagree)
    exe = None
    for tok in lowered.split():
        if tok.startswith("exe=") or tok.startswith("md5="):
            exe = tok.split("=", 1)[1]
    if exe is None:
        return None, "claims lockstep but says no exe=<md5>, so the binary is unidentified"
    if db.md5 and exe.replace("_", "").upper() != db.md5:
        return None, ("exe=%s does not match the DB's filemd5 %s - the capture and the class "
                      "table are for different binaries" % (exe, db.md5))
    return "lockstep", "verified: read records present, consistent with the frames, exe matches the DB"



def build(recs, db, require_resolved_ak):
    """Validate ordering/density invariants and render the frame lines. Dies on violation."""
    frames = []          # (n, tickms, clocksec, lcgstate)
    entities = {}        # frame -> list of (slot, rec)
    worlds = {}
    for kind, r in recs:
        if kind == "frame":
            n = _int(r, "frame", "frame record")
            frames.append((n, _int(r, "tickms", "frame %d" % n),
                           _int(r, "clocksec", "frame %d" % n), r.get("lcgstate")))
        elif kind == "entity":
            fn = _int(r, "frame", "entity record")
            entities.setdefault(fn, []).append((_int(r, "slot", "entity record"), r))
        elif kind == "world":
            worlds[_int(r, "frame", "world record")] = r

    if not frames:
        die(EXIT_INVARIANT, "INVARIANT-VIOLATION: the capture has no record=frame - refusing to "
                            "emit an empty digest that would compare IDENTICAL to any other "
                            "empty digest")
    # INVARIANT: frame indices strictly increase. An out-of-order or duplicated frame number is
    # the single most damaging thing a capture can carry, because the comparator walks lines in
    # lockstep and would report the REORDERING as a state divergence at the first moved line.
    prev = None
    for n, _, _, _ in frames:
        if prev is not None and n <= prev:
            die(EXIT_INVARIANT,
                "INVARIANT-VIOLATION: frame=%d follows frame=%d - frame indices must STRICTLY "
                "INCREASE (out of order or duplicated)" % (n, prev))
        prev = n

    base_clock = frames[0][2]
    out_lines = []
    unresolved = 0
    total_slots = 0
    cap = 12
    for n, tick_ms, clock_sec, lcg in frames:
        ents = sorted(entities.get(n, []), key=lambda t: t[0])
        # INVARIANT: slots dense from 0. A gap would make e2 mean a different entity on the two
        # sides, and a digest whose entity indices disagree is not a comparison at all.
        for i, (slot, _) in enumerate(ents):
            if slot != i:
                die(EXIT_INVARIANT,
                    "INVARIANT-VIOLATION: frame=%d entity slots must be dense from 0, got %s"
                    % (n, ",".join(str(s) for s, _ in ents)))
        # INVARIANT: the declared count must equal the records. `ents` is hashed, so a mismatch is
        # a claim about the world that the payload contradicts.
        w = worlds.get(n, {})
        parts = ["frame=%d clockDelta=%d ents=%d" % (n, clock_sec - base_clock, len(ents))]
        shown = 0
        for slot, r in ents:
            if shown >= cap:
                break
            q = _int(r, "q", "entity slot %d" % slot)
            # INVARIANT: the queue count cannot exceed the pointers the capture named. q=5 with
            # four vt= entries is an arithmetic impossibility in EITHER representation, and it is
            # the difference between "D-2 fired" and "the capture is broken".
            vt_raw = str(r.get("vt", ""))
            vts = [v for v in vt_raw.split(",")]
            if len(vts) != 4:
                die(EXIT_INVARIANT,
                    "INVARIANT-VIOLATION: frame=%d slot=%d vt=%r must name exactly 4 slots "
                    "(use - for empty)" % (n, slot, vt_raw))
            live = len([v for v in vts if v not in ("", "-")])
            if q > live and q > 4:
                die(EXIT_INVARIANT,
                    "INVARIANT-VIOLATION: frame=%d slot=%d q=%d exceeds the %d action pointers "
                    "the capture supplies for it" % (n, slot, q, live))
            names = []
            for v in vts:
                if v in ("", "-"):
                    names.append("-")
                    continue
                ea = _num(v)
                if ea is None:
                    die(EXIT_INVARIANT,
                        "INVARIANT-VIOLATION: frame=%d slot=%d vt=%r is not an address"
                        % (n, slot, v))
                hit = db.lookup(ea)
                total_slots += 1
                if hit is None:
                    unresolved += 1
                    names.append("UNKNOWN_VT=%s" % ("0x%x" % ea))
                else:
                    names.append(hit[0])
            if require_resolved_ak and unresolved:
                die(EXIT_INVARIANT,
                    "INVARIANT-VIOLATION: --require-resolved-ak and %d action vftptr(s) are "
                    "absent from the class DB - a class name we cannot read is not a value to "
                    "guess, and an ak column of UNKNOWN_VT cannot be compared as state"
                    % unresolved)
            ak = "/".join(names)
            # qi: the effect-inbox count (shipped Entity+0x2F0). A capture that predates the
            # inbox era has no read for it - that renders NA (absent, not zero), the same
            # NA discipline as pathJobs: a silent 0 would claim an observation nobody made.
            qi = r.get("qi")
            if qi is None:
                qi = "NA:absent-in-capture"
            else:
                qi = str(_int(r, "qi", "entity slot %d" % slot))
            parts.append(" e%d=%s,pos=%.2f/%.2f/%.2f,yaw=%.2f,hp=%d/%d,st=%d,%d,%d,q=%d,ak=%s,qi=%s"
                         % (shown, r.get("name") or "-",
                            _pos(r.get("pos"))[0], _pos(r.get("pos"))[1], _pos(r.get("pos"))[2],
                            float(r.get("yaw") or 0.0),
                            _int(r, "hpcur", "entity slot %d" % slot),
                            _int(r, "hpmax", "entity slot %d" % slot),
                            _int(r, "state", "entity slot %d" % slot),
                            _int(r, "moving", "entity slot %d" % slot),
                            _int(r, "acted", "entity slot %d" % slot),
                            q, ak, qi))
            shown += 1
        if len(ents) > shown:
            parts.append(" entsMore=1")
        # NA means "the seam is not ported / not observable here", never "unchanged" - the same
        # rule the ours side follows. A shipped PathManager count of 0 would be a claim we have
        # not earned unless the capture supplied it.
        pj = w.get("pathjobs")
        parts.append(" pathJobs=%s" % (pj if pj not in (None, "") else
                                       "NA:no-pathmanager-read-in-capture"))
        turns = w.get("turns")
        order = w.get("turnorder")
        if turns in (None, ""):
            parts.append(" turns=NA:no-initiative-manager-read-in-capture")
            parts.append(" turnOrder=NA:no-initiative-manager-read-in-capture")
        else:
            parts.append(" turns=%s" % turns)
            parts.append(" turnOrder=%s" % (order if order not in (None, "", "-") else "-"))
        # draws: the shipped generator exposes a STATE dword, not a counter. Counting draws would
        # be our seam, not theirs, so the shipped side prints NA and carries the state as CONTEXT.
        parts.append(" draws=NA:no-counter-on-shipped-generator")
        compared = "".join(parts)
        out_lines.append("%s fnv=%08X ctx: tickMs=%d clock=%d%s"
                         % (compared, fnv1a(compared), tick_ms, clock_sec,
                            (" lcgState=%s" % lcg) if lcg else ""))
    return out_lines, unresolved, total_slots


def main(argv):
    args = argv[:]
    opts = {"db": None, "capture": None, "out": None, "provenance": None,
            "require_resolved_ak": False, "print_classes": False}
    while args:
        a = args.pop(0)
        if a == "--capture":
            opts["capture"] = args.pop(0) if args else None
        elif a == "--out":
            opts["out"] = args.pop(0) if args else None
        elif a == "--provenance":
            opts["provenance"] = args.pop(0) if args else None
        elif a == "--db":
            opts["db"] = args.pop(0) if args else None
        elif a == "--require-resolved-ak":
            opts["require_resolved_ak"] = True
        elif a == "--print-classes":
            opts["print_classes"] = True
        elif a in ("-h", "--help"):
            sys.stdout.write(USAGE)
            return 0
        else:
            die(EXIT_IO, "orig-digest: unknown argument %r\n%s" % (a, USAGE))
    here = os.path.dirname(os.path.abspath(__file__))
    root = os.path.dirname(here)
    if opts["db"] is None:
        opts["db"] = os.path.join(root, "artifacts", "F3.json")
    if not opts["capture"]:
        die(EXIT_IO, "orig-digest: --capture FILE is required\n" + USAGE)
    if not opts["out"]:
        die(EXIT_IO, "orig-digest: --out FILE is required\n" + USAGE)
    db = VftMap(opts["db"])

    recs = parse_capture(opts["capture"])
    # PROVENANCE IS REQUIRED INPUT. Refuse before any invariant work: a digest file with no
    # provenance is unreadable evidence, and `head`ing it later must never leave that in doubt.
    cap_prov = None
    for kind, r in recs:
        if kind == "provenance":
            cap_prov = r.get("provenance")
    provenance = opts["provenance"] or cap_prov
    if provenance is None:
        die(EXIT_PROVENANCE,
            "PROVENANCE REQUIRED: pass --provenance TEXT or a `record=provenance provenance=...` "
            "line. Nothing is written. The point of the field is that a reader of the DIGEST FILE "
            "can tell a captured state from a made-up one - a default would destroy that.")
    klass, reason = classify_provenance(provenance, recs, db)
    if klass is None:
        die(EXIT_PROVENANCE,
            "PROVENANCE-UNVERIFIED: %s\n"
            "  provenance was: %r\n"
            "  a `lockstep` label must name the reads it used (r1/r3/r4b), carry exe=<filemd5 of "
            "%s>, and match the records in the capture. Say `synthetic` instead and the artifact "
            "will admit it." % (reason, provenance.strip(), opts["db"]))

    lines, unresolved, total_slots = build(recs, db, opts["require_resolved_ak"])

    header = []
    header.append(CONTRACT)
    header.append(OUR_SIDE_CONTRACT_TAIL % ("NA:shipped-side-does-not-run-our-combat-manager",
                                            "NA:shipped-side-does-not-run-our-path-manager"))
    header.append("# SIDE=original (rendered from a capture file; the ours side is SIDE=ours)\n")
    header.append("# PROVENANCE=%s\n" % provenance.strip())
    header.append("# PROVENANCE-CLASS=%s\n" % klass)
    if klass == "synthetic":
        header.append("# SYNTHETIC - exercises the tool, proves nothing about F3.exe\n")
    else:
        header.append("# CAPTURE=verified (%s)\n" % reason)
    header.append("# AK-RESOLUTION=%d/%d vftptr slots named through %s (filemd5 %s); unresolved "
                  "print UNKNOWN_VT=<ea> and are COUNTED, never named\n"
                  % (total_slots - unresolved, total_slots, os.path.basename(opts["db"]),
                     db.md5))
    header.append("# LIMIT=this file is a RENDER of a capture, not a read of a process. A mixed "
                  "provenance comparison (SYNTHETIC against a real capture) is a fixture bug, not "
                  "a divergence, and the comparator refuses it.\n")
    n_frames = len(lines)
    body = "".join(header) + "".join(l + "\n" for l in lines)
    body += "# END frames=%d\n" % n_frames

    if opts["print_classes"]:
        for l in lines:
            sys.stdout.write(l + "\n")
        return 0
    try:
        # newline="" + explicit "\r\n": the ours side is written by MSVC's text mode, which emits
        # CRLF. A digest whose lines differ only in an invisible byte would report every frame as
        # a divergence, so the shipped side matches it byte for byte.
        with open(opts["out"], "w", encoding="utf-8", newline="\r\n") as fh:
            fh.write(body)
    except OSError as exc:
        die(EXIT_IO, "orig-digest: cannot write %s: %s" % (opts["out"], exc))
    sys.stderr.write("orig-digest: wrote %s (%d frame line(s), SIDE=original, PROVENANCE-CLASS=%s"
                     "%s)\n" % (opts["out"], n_frames, klass,
                                " - SYNTHETIC, proves nothing about F3.exe"
                                if klass == "synthetic" else ""))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
