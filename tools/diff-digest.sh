#!/bin/sh
# T-026 - tools/diff-digest.sh: compare two F3 simulation digests and NAME the first field that
# diverges, because "the hashes differ" is not an answer anyone can act on.
#
# Reads frame lines produced by F3::Sim::WriteFrame (src/F3/SimDigest.hpp). The leading '#' lines
# carry the contract (allowlist, exclusions, NA policy) and are compared too: a digest whose
# contract changed is a different measurement even when the frames match.
#
# Fields after " ctx:" on a frame line are CONTEXT: printed, captured, deliberately NOT compared
# (tickMs is machine uptime and differs across runs of the same session; a CONTEXT field can later be
# promoted to COMPARED by editing this comparator, never by re-running a debugger session). A line that
# differs only in CONTEXT is reported as a note and is NOT a divergence.
#
# PROVENANCE IS PART OF THE CONTRACT, and comparing across it is not a finding. `tools/orig-digest.sh`
# writes `# PROVENANCE-CLASS=synthetic|lockstep`; a SYNTHETIC side compared against a real capture
# (or against a run that carries no class at all) produces divergences that LOOK like parity results
# and are actually fixture bugs, so the comparator refuses: it prints MIXED-PROVENANCE, compares
# NOTHING, and exits 3. A file that declares no class is treated as its own class (`unrecorded`),
# which is what our own digests are; `synthetic` is only ever comparable to `synthetic`, and
# `lockstep` to `lockstep` or to an unrecorded ours-side run.
#
# The `#` header lines are split the same way: CONTRACT lines (the field allowlist, COMPARED list,
# exclusions, NA policy, PROVES line) must be byte-equal or the two files are different measurements;
# SIDE/PROVENANCE/ANNOTATION lines identify WHICH side a file is and are read, not diffed. The list
# of annotation prefixes is in the awk below and it is deliberately short - an unrecognised `#` line
# still compares, so a new header line cannot quietly become side-local.
#
# Exit codes: 0 identical, 1 divergence, 2 usage/IO error, 3 MIXED-PROVENANCE (no comparison run).
#
# LIMIT, stated here because a tool that hides its own limits is how stale numbers get quoted: it
# compares digest FILES. Whether a file is evidence depends on its own PROVENANCE line, which this
# comparator checks but cannot invent - a `# SYNTHETIC` side is still synthetic when compared
# against another synthetic side, and the note says so.

if [ $# -ne 2 ]; then
	echo "usage: $0 <digest-a> <digest-b>" >&2
	exit 2
fi
for f in "$1" "$2"; do
	if [ ! -r "$f" ]; then echo "cannot read $f" >&2; exit 2; fi
done

# --- provenance gate: refuse a mixed-provenance comparison BEFORE comparing anything ----------
# Read with grep, not awk, so this cannot be skipped by a parse error further down, and so the
# exit code is the shell's own (harness item 17: never read a status through a subshell).
# `tr -d '\r'` is NOT cosmetic: the ours side is written by MSVC in text mode, so the value ends
# in CRLF, and a trailing \r silently breaks the `case` patterns below (it did - the refusal
# looked like it was not firing when in fact "synthetic\r/unrecorded" matched nothing).
prov_a=$(grep -m1 '^# PROVENANCE-CLASS=' "$1" 2>/dev/null | sed 's/^# PROVENANCE-CLASS=//' | tr -d '\r\n' || true)
prov_b=$(grep -m1 '^# PROVENANCE-CLASS=' "$2" 2>/dev/null | sed 's/^# PROVENANCE-CLASS=//' | tr -d '\r\n' || true)
[ -n "$prov_a" ] || prov_a=unrecorded
[ -n "$prov_b" ] || prov_b=unrecorded
mixed=0
case "$prov_a/$prov_b" in
	synthetic/synthetic) mixed=0 ;;          # fixture vs fixture: allowed, and still a fixture
	synthetic/*|*/synthetic) mixed=1 ;;      # a fixture meeting real evidence: refuse
	*) mixed=0 ;;
esac
if [ $mixed -eq 1 ]; then
	echo "MIXED-PROVENANCE: no comparison performed"
	echo "  $1: PROVENANCE-CLASS=$prov_a"
	echo "  $2: PROVENANCE-CLASS=$prov_b"
	echo "  A SYNTHETIC side and a real capture differ in everything except state, and the diff"
	echo "  would report that as divergence. Re-run with both sides the same class, or fix the"
	echo "  capture's provenance line so the artifact says what it is."
	exit 3
fi

awk -v fa="$1" -v fb="$2" '
# split one line into key -> value pairs, expanding comma-chained entity tokens so that
# "e0=DIGEST_A,pos=1.0/2.0/0.0,hp=43/45" yields e0 / e0.pos / e0.hp
function parse(line, out,   np, parts, i, eq, key, val, np2, p2, j, head, cp) {
	cp = index(line, " ctx:")
	if (cp > 0) line = substr(line, 1, cp - 1)   # COMPARED span only
	np = split(line, parts, " ")
	for (i = 1; i <= np; i++) {
		eq = index(parts[i], "=")
		if (eq == 0) continue
		key = substr(parts[i], 1, eq - 1)
		val = substr(parts[i], eq + 1)
		if (key ~ /^e[0-9]+/) {
			head = key
			sub(/,.*/, "", head)
			np2 = split(val, p2, ",")
			out[head] = p2[1]
			for (j = 2; j <= np2; j++) {
				eq = index(p2[j], "=")
				if (eq > 0) out[head "." substr(p2[j], 1, eq - 1)] = substr(p2[j], eq + 1)
			}
		} else {
			out[key] = val
		}
	}
}
# CONTEXT span: everything after " ctx:" on the line
function parsectx(line, out,   p2, seg, np, parts, i, eq) {
	p2 = index(line, " ctx:")
	if (p2 == 0) return
	seg = substr(line, p2 + 6)
	np = split(seg, parts, " ")
	for (i = 1; i <= np; i++) {
		eq = index(parts[i], "=")
		if (eq > 0) out[substr(parts[i], 1, eq - 1)] = substr(parts[i], eq + 1)
	}
}
# side identity / provenance / run metadata: which side a file is, not what it measures. The list
# is deliberately short and anchored, so a NEW header line still compares as contract rather than
# dodging the check by being unrecognised.
function isannot(line) {
	return (line ~ /^# (SIDE|PROVENANCE|PROVENANCE-CLASS|SYNTHETIC|CAPTURE|AK-RESOLUTION|ORIG-LCG|LIMIT|GENERATED|SOURCES|END)( |=|:)/)
}
# NA means "this side could not measure it", never "it was unchanged" (the digest prints that rule
# in its own header). So NA-vs-NA, and NA-vs-a-value, are a COVERAGE difference: shown, labelled,
# and NOT a state divergence. Silently treating them as equal would hide which fields were never
# compared at all; treating them as divergences would make every ours-vs-original run report the
# same fake finding.
function isna(v) { return (substr(v, 1, 3) == "NA:") }
function any_printed(x, y,   k) {
	for (k in x) { if (!(k in y) || x[k] != y[k]) return 1 }
	for (k in y) { if (!(k in x)) return 1 }
	return 0
}
BEGIN {
	la = 0; lb = 0            # CONTRACT lines
	fa_n = 0; fb_n = 0         # frame (payload) lines
	while ((getline line < fa) > 0) {
		sub(/\r$/, "", line)		# CRLF from MSVC text mode vs LF from a unix editor is not a finding
		if (substr(line, 1, 1) == "#") { if (!isannot(line)) AC[++la] = line; continue }
		if (line == "") continue
		AF[++fa_n] = line
	}
	close(fa)
	while ((getline line < fb) > 0) {
		sub(/\r$/, "", line)
		if (substr(line, 1, 1) == "#") { if (!isannot(line)) BC[++lb] = line; continue }
		if (line == "") continue
		BF[++fb_n] = line
	}
	close(fb)

	# --- contract first: a different contract is a different measurement, whatever the frames say
	n = (la < lb ? la : lb)
	for (i = 1; i <= n; i++) {
		if (AC[i] != BC[i]) {
			printf("CONTRACT differs at contract line %d\n  A: %s\n  B: %s\n", i, AC[i], BC[i])
			exit 1
		}
	}
	if (la != lb) {
		printf("CONTRACT differs: %s has %d contract line(s), %s has %d\n", fa, la, fb, lb)
		if (la > lb) printf("  extra in %s: %s\n", fa, AC[lb + 1])
		else printf("  extra in %s: %s\n", fb, BC[la + 1])
		exit 1
	}

	if (fa_n != fb_n)
		printf("FRAME COUNT differs: %s=%d %s=%d (a short file is usually a run that was killed, not a run that diverged)\n", fa, fa_n, fb, fb_n)

	n = (fa_n < fb_n ? fa_n : fb_n)
	cov = 0
	for (i = 1; i <= n; i++) {
		sa = AF[i]; sb = BF[i]
		pa = index(sa, " ctx:"); if (pa > 0) sa = substr(sa, 1, pa - 1)
		pb = index(sb, " ctx:"); if (pb > 0) sb = substr(sb, 1, pb - 1)
		if (sa == sb) {
			if (AF[i] != BF[i]) printf("note: frame line %d differs only in CONTEXT fields (not a divergence)\n", i)
			continue
		}
		split("", x); split("", y)
		parse(AF[i], x); parse(BF[i], y)
		frame = (x["frame"] != "" ? x["frame"] : "?")
		printf("first divergence at line %d, frame=%s\n", i, frame)
		split("", c); split("", d); parsectx(AF[i], c); parsectx(BF[i], d)
		ndiff = 0
		for (k in x) {
			if (!(k in y))        { printf("  %s=REMOVED\n", k); ndiff++ }
			else if (x[k] != y[k]) {
				if (isna(x[k]) || isna(y[k])) {
					printf("  %s: [%s] -> [%s]  (COVERAGE: one side could not measure it, not a state divergence)\n", k, x[k], y[k])
					cov++
				} else { printf("  %s: [%s] -> [%s]\n", k, x[k], y[k]); ndiff++ }
			}
		}
		for (k in y) if (!(k in x)) { printf("  %s=ADDED\n", k); ndiff++ }
		for (k in c) if (k in d && c[k] != d[k]) printf("  %s: [%s] -> [%s]  (CONTEXT, not a divergence)\n", k, c[k], d[k])
		if (!ndiff && !any_printed(x, y)) {
			printf("  (only coverage differences - no state key moved)\n")
		} else {
			if (cov) printf("  (%d field(s) also differ on COVERAGE only)\n", cov)
			exit 1
		}
	}
	if (fa_n != fb_n) exit 1          # counts differed above: never report IDENTICAL
	if (cov) { print "NO STATE DIVERGENCE (coverage-only differences were reported above)"; exit 0 }
	print "IDENTICAL"; exit 0
}
' "$1" "$2"
