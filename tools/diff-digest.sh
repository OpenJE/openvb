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
# Exit codes: 0 identical, 1 divergence, 2 usage/IO error.
#
# LIMIT, stated here because a tool that hides its own limits is how stale numbers get quoted:
# this compares OUR digest against OUR digest. It says nothing about the original - parity needs
# the shipped side, which needs a bootable F3.exe plus a digest from it (T-028).

if [ $# -ne 2 ]; then
	echo "usage: $0 <digest-a> <digest-b>" >&2
	exit 2
fi
for f in "$1" "$2"; do
	if [ ! -r "$f" ]; then echo "cannot read $f" >&2; exit 2; fi
done

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
BEGIN {
	la = 0; lb = 0
	while ((getline line < fa) > 0) { la++; A[la] = line }
	close(fa)
	while ((getline line < fb) > 0) { lb++; B[lb] = line }
	close(fb)

	if (la != lb) printf("LINE COUNT differs: %s=%d %s=%d\n", fa, la, fb, lb)

	n = (la < lb ? la : lb)
	for (i = 1; i <= n; i++) {
		# compare the COMPARED span only: everything before " ctx:" is hashed and load-bearing,
	# everything after it is captured-not-compared (tickMs, and any promoted CONTEXT field later)
	{
		sa = A[i]; sb = B[i]
		pa = index(sa, " ctx:"); if (pa > 0) sa = substr(sa, 1, pa - 1)
		pb = index(sb, " ctx:"); if (pb > 0) sb = substr(sb, 1, pb - 1)
		if (sa == sb) {
			if (A[i] != B[i]) printf("note: line %d differs only in CONTEXT fields (not a divergence)\n", i)
			continue
		}
	}
		if (substr(A[i], 1, 1) == "#" || substr(B[i], 1, 1) == "#") {
			printf("CONTRACT differs at comment line %d\n  A: %s\n  B: %s\n", i, A[i], B[i])
			exit 1
		}
		split("", x); split("", y)
		parse(A[i], x); parse(B[i], y)
		frame = (x["frame"] != "" ? x["frame"] : "?")
		printf("first divergence at line %d, frame=%s\n", i, frame)
		# CONTEXT keys are printed after " ctx:" and are deliberately NOT part of the comparison;
		# if one differs we still show it, but labelled, so nobody reads an uptime difference as a
		# state divergence (which is how the first version of this comparator misled me).
		split("", c); split("", d); parsectx(A[i], c); parsectx(B[i], d)
		for (k in x) {
			if (!(k in y))        printf("  %s=REMOVED\n", k)
			else if (x[k] != y[k]) printf("  %s: [%s] -> [%s]\n", k, x[k], y[k])
		}
		for (k in c) if (k in d && c[k] != d[k]) printf("  %s: [%s] -> [%s]  (CONTEXT, not a divergence)\n", k, c[k], d[k])
		for (k in y) if (!(k in x)) printf("  %s=ADDED\n", k)
		if (!any_printed(x, y)) printf("  (no key-level change found - compare the raw lines)\n")
		exit 1
	}
	if (la == lb) { print "IDENTICAL"; exit 0 }
	exit 1
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
function ctxval(line, key,   p2, seg, np, parts, i, eq) {
	p2 = index(line, " fnv=")
	if (p2 == 0) return ""
	seg = substr(line, p2 + 1)
	np = split(seg, parts, " ")
	return parts[1]
}
function any_printed(x, y,   k) {
	for (k in x) { if (!(k in y) || x[k] != y[k]) return 1 }
	for (k in y) { if (!(k in x)) return 1 }
	return 0
}
' "$1" "$2"
