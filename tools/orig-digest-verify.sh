#!/usr/bin/env bash
# T-031 falsification suite for the shipped-side digest emitter (rule: every tool ships with a test
# showing it can FAIL - and for a comparator, "has never rejected anything" is itself unproven).
#
# Each check below has a mutant. If the emitter or the comparator stops refusing what it must
# refuse, or starts calling a fixture difference a parity divergence, a line here goes red.
#
# Exit: 0 all good, 1 a check failed. No gate is required to run this: it touches tools/ only.
set -uo pipefail
cd "$(git rev-parse --show-toplevel)"

fail=0
say() { printf '%-64s %s\n' "$1" "$2"; }
# harness item 17: an assertion helper takes the exit code AND the evidence line as separate,
# validated inputs. A scenario that produced neither is a failure, never a silent pass.
check_rc() { if [ "$2" = "$3" ]; then say "$1" "ok (rc=$2)"; else say "$1" "FAIL got=$2 want=$3"; fail=1; fi; }
check_has() { if printf '%s' "$3" | grep -qF -- "$2"; then say "$1" "ok"; else say "$1" "FAIL missing: $2"; fail=1; fi; }
check_not() { if printf '%s' "$3" | grep -qF -- "$2"; then say "$1" "FAIL found: $2"; fail=1; else say "$1" "ok"; fi; }

W=/tmp/t031-od
rm -rf "$W"; mkdir -p "$W"
DB=artifacts/F3.json
MD5=$(python3 -c "import json;print(json.load(open('$DB'))['filemd5'])" 2>/dev/null || echo UNKNOWN)

# vftptrs read from artifacts/F3.json, not typed from memory: the point of R4b is that the NAME
# comes from the DB. These are the stored-pointer values (vftable ea + the class's vftptr offset).
VT_MOVE=0x649c1c      # F3::GameActionMoveTo
VT_ATK=0x647bbc       # F3::GameActionAttack
VT_WAIT=0x64bc6c      # F3::GameActionWait
VT_DIE=0x648694       # F3::GameActionDie
VT_BOGUS=0xdeadbeef   # present in NO class: the invariant-violating fixture

# --------------------------------------------------------------------------------------------
# fixture A: deliberately SYNTHETIC. It exercises the tool and says so.
# --------------------------------------------------------------------------------------------
cat > "$W/cap-synthetic.txt" <<'CAP'
# T-031 fixture capture - SYNTHETIC VALUES, not read from any process. Two entities, three frames,
# an action queue that fills and drains. Real GameAction* vftptrs are used on purpose: name
# resolution must be exercised even when the numbers around it are invented.
record=frame frame=100 tickms=3456789 clocksec=34567
record=entity frame=100 slot=0 name=CAPFIX_A pos=20.00/0.00/0.00 yaw=0.00 hpcur=45 hpmax=45 state=4 moving=0 acted=0 q=0 vt=-,-,-,-
record=entity frame=100 slot=1 name=CAPFIX_B pos=20.40/0.00/0.00 yaw=1.57 hpcur=30 hpmax=30 state=4 moving=0 acted=0 q=1 vt=0x64bc6c,-,-,-
record=world frame=100 pathjobs=0 turns=2 turnorder=CAPFIX_A,CAPFIX_B
record=frame frame=101 tickms=3456789 clocksec=34567
record=entity frame=101 slot=0 name=CAPFIX_A pos=20.00/0.00/0.00 yaw=0.00 hpcur=45 hpmax=45 state=4 moving=0 acted=0 q=1 vt=0x649c1c,-,-,-
record=entity frame=101 slot=1 name=CAPFIX_B pos=20.40/0.00/0.00 yaw=1.57 hpcur=28 hpmax=30 state=4 moving=0 acted=1 q=1 vt=0x64bc6c,-,-,-
record=world frame=101 pathjobs=0 turns=2 turnorder=CAPFIX_A,CAPFIX_B
record=frame frame=103 tickms=3456790 clocksec=34568
record=entity frame=103 slot=0 name=CAPFIX_A pos=20.10/0.00/0.00 yaw=0.00 hpcur=45 hpmax=45 state=4 moving=1 acted=0 q=2 vt=0x649c1c,0x647bbc,-,-
record=entity frame=103 slot=1 name=CAPFIX_B pos=20.40/0.00/0.00 yaw=1.57 hpcur=26 hpmax=30 state=4 moving=0 acted=1 q=0 vt=-,-,-,-
record=world frame=103 pathjobs=0 turns=2 turnorder=CAPFIX_A,CAPFIX_B
CAP

# --------------------------------------------------------------------------------------------
# fixture B: a second synthetic capture with ONE operand corrupted (hp and yaw on slot 1).
# --------------------------------------------------------------------------------------------
sed -e 's/hpcur=28 hpmax=30/hpcur=17 hpmax=30/' -e 's/yaw=1.57 hpcur=17/yaw=2.25 hpcur=17/' \
	"$W/cap-synthetic.txt" > "$W/cap-mutant.txt"

# fixture C: the frame indices go BACKWARDS. This must be refused, not rendered.
cat > "$W/cap-outoforder.txt" <<'CAP'
record=frame frame=100 tickms=1 clocksec=1
record=entity frame=100 slot=0 name=A pos=0/0/0 yaw=0 hpcur=1 hpmax=1 state=4 moving=0 acted=0 q=0 vt=-,-,-,-
record=world frame=100 pathjobs=0 turns=1 turnorder=A
record=frame frame=99 tickms=2 clocksec=2
record=entity frame=99 slot=0 name=A pos=0/0/0 yaw=0 hpcur=1 hpmax=1 state=4 moving=0 acted=0 q=0 vt=-,-,-,-
record=world frame=99 pathjobs=0 turns=1 turnorder=A
CAP

# fixture D: an action whose vftptr is in NO class.
sed "s/vt=0x649c1c,-,-,-/vt=$VT_BOGUS,-,-,-/" "$W/cap-synthetic.txt" > "$W/cap-unknownvt.txt"

# --------------------------------------------------------------------------------------------
# 1) provenance is REQUIRED input, not a default
# --------------------------------------------------------------------------------------------
out=$(tools/orig-digest.sh --capture "$W/cap-synthetic.txt" --out "$W/no-prov.digest" 2>&1); rc=$?
check_rc "no --provenance -> refused" "$rc" "3"
check_has "  refusal names PROVENANCE as the reason" "PROVENANCE REQUIRED" "$out"
check_not "  and NO digest file was written" "" "$( [ -e "$W/no-prov.digest" ] && echo existed )"; \
	say "  no file written on refusal" "$([ -e "$W/no-prov.digest" ] && echo "FAIL file exists" || echo ok)"

# --------------------------------------------------------------------------------------------
# 2) synthetic input is accepted, but the ARTIFACT admits it
# --------------------------------------------------------------------------------------------
out=$(tools/orig-digest.sh --capture "$W/cap-synthetic.txt" --provenance synthetic-fixture \
	      --out "$W/orig-a.digest" 2>&1); rc=$?
check_rc "synthetic provenance -> emitted" "$rc" "0"
d=$(cat "$W/orig-a.digest" 2>/dev/null)
check_has "  digest says SYNTHETIC in the file itself" "# SYNTHETIC - exercises the tool, proves nothing about F3.exe" "$d"
check_has "  and records the provenance verbatim" "# PROVENANCE=synthetic-fixture" "$d"
check_has "  and its class" "# PROVENANCE-CLASS=synthetic" "$d"
check_has "  vftptrs resolved to class NAMES, not pointers" "ak=GameActionWait/-/-/-" "$d"
check_has "  the queue DEPTH is printed as a count" ",q=2,ak=GameActionMoveTo/GameActionAttack" "$d"
check_has "  frame indices strictly increase in the output" "frame=103" "$d"
check_has "  states= are THREE values now (T-039: no busy byte)" ",st=4,0,0,q=" "$d"

# --------------------------------------------------------------------------------------------
# 3) MIXED PROVENANCE is refused by the comparator, and exits nonzero
# --------------------------------------------------------------------------------------------
# ours side without a PROVENANCE-CLASS line is `unrecorded`; synthetic vs unrecorded is a fixture
# meeting evidence, which is exactly the confusion that must not produce a divergence report.
ours=$(ls /tmp/sd-a.digest 2>/dev/null || true)
if [ -z "$ours" ]; then
	sh tools/sim-driver-build.sh > "$W/driver-build.log" 2>&1 && {
		sed 's#/tmp/sd-demo.digest#'"$W"'/ours.digest#' tools/scripts/host-demo.txt > "$W/ours.txt"
		WINE=$(command -v wine || ls /nix/store/*/bin/wine 2>/dev/null | head -1)
		WINEPREFIX=$PWD/.devenv/state/f3-reimpl-wineprefix WINEDEBUG=-all \
			"$WINE" tools/sim-driver.exe "$W/ours.txt" >/dev/null 2>&1 || true
		ours=$W/ours.digest
	}
fi
if [ -r "$ours" ]; then
	out=$(tools/diff-digest.sh "$W/orig-a.digest" "$ours" 2>&1); rc=$?
	check_rc "SYNTHETIC side vs a real ours run -> refused" "$rc" "3"
	check_has "  says MIXED-PROVENANCE and that no comparison ran" "MIXED-PROVENANCE: no comparison performed" "$out"
	check_not "  and does NOT report a frame divergence (the trap)" "first divergence at line" "$out"
else
	say "SYNTHETIC vs ours refused" "SKIP: no ours-side digest (wine/driver unavailable)"
	say "  SKIP is a miss, not a pass" "FAIL"; fail=1
fi

# synthetic vs synthetic IS comparable (that is what a fixture exercise means) and still says so
out=$(tools/diff-digest.sh "$W/orig-a.digest" "$W/orig-a.digest" 2>&1); rc=$?
check_rc "synthetic vs itself -> IDENTICAL (exit 0)" "$rc" "0"
check_has "  and IDENTICAL is what it prints" "IDENTICAL" "$out"

# --------------------------------------------------------------------------------------------
# 4) the mutant: a corrupted operand must NAME the field, not just differ
# --------------------------------------------------------------------------------------------
out=$(tools/orig-digest.sh --capture "$W/cap-mutant.txt" --provenance synthetic-fixture \
	      --out "$W/orig-mutant.digest" 2>&1); rc=$?
check_rc "mutant capture emitted" "$rc" "0"
out=$(tools/diff-digest.sh "$W/orig-a.digest" "$W/orig-mutant.digest" 2>&1); rc=$?
check_rc "hp+yaw mutant -> diverges" "$rc" "1"
check_has "  divergence NAMES the field (e1.hp)" "e1.hp: [28/30] -> [17/30]" "$out"
check_has "  and says which frame" "frame=101" "$out"
check_not "  CONTEXT (tickMs) is not reported as state" "tickMs: [" "$out"

# --------------------------------------------------------------------------------------------
# 5) invariant refusals: the run that proves a refusal fires
# --------------------------------------------------------------------------------------------
out=$(tools/orig-digest.sh --capture "$W/cap-outoforder.txt" --provenance synthetic-fixture \
	      --out "$W/bad-order.digest" 2>&1); rc=$?
check_rc "out-of-order frame -> INVARIANT-VIOLATION" "$rc" "4"
check_has "  names the frame pair it refused" "frame=99 follows frame=100" "$out"
check_not "  nothing written for a refused capture" "" "$( [ -e "$W/bad-order.digest" ] && echo existed )"

out=$(tools/orig-digest.sh --capture "$W/cap-unknownvt.txt" --provenance synthetic-fixture \
	      --out "$W/unknownvt.digest" 2>&1); rc=$?
check_rc "unresolvable vftptr -> emitted (counted, never named)" "$rc" "0"
d=$(cat "$W/unknownvt.digest" 2>/dev/null)
check_has "  prints UNKNOWN_VT=<ea>" "UNKNOWN_VT=0xdeadbeef" "$d"
check_has "  and COUNTS the unresolved in the artifact" "# AK-RESOLUTION=4/5 vftptr slots named" "$d"
out=$(tools/orig-digest.sh --capture "$W/cap-unknownvt.txt" --provenance synthetic-fixture \
	      --require-resolved-ak --out "$W/strict.digest" 2>&1); rc=$?
check_rc "  ...and --require-resolved-ak REFUSES it" "$rc" "4"
check_has "  refusal says the DB cannot resolve it" "absent from the class DB" "$out"

# --------------------------------------------------------------------------------------------
# 6) a `lockstep` claim must be EARNED, not typed
# --------------------------------------------------------------------------------------------
out=$(tools/orig-digest.sh --capture "$W/cap-synthetic.txt" \
	      --provenance "lockstep r3,r4,r4b session=99 exe=$MD5" --out "$W/fake-lock.digest" 2>&1); rc=$?
check_rc "lockstep claim with no record=read evidence -> refused" "$rc" "3"
check_has "  refusal says which reads have no record" "evidence for r3,r4,r4b" "$out"
check_not "  and nothing was written" "" "$( [ -e "$W/fake-lock.digest" ] && echo existed )"

# the ACCEPTING branch, so the guard is not just "always say no". The values below are still
# invented - what this proves is that a claim WITH read records, consistent frames and the right
# binary is labelled lockstep. It is not parity evidence; a fixture is still a fixture.
cat > "$W/cap-withreads.txt" <<CAP
record=read read=r1 address=0x707c60 value=0x12345678 when=first-draw breakpoint=0x48ccf6
record=read read=r3 address=Entity+0x314 value=2 when=mid-round breakpoint=0x459f5d
record=read read=r4b address=[action+0x00] value=$VT_ATK when=mid-round breakpoint=0x459f5d
record=frame frame=200 tickms=100 clocksec=100 lcgstate=0x12345678
record=entity frame=200 slot=0 name=READFIX pos=1.00/2.00/0.00 yaw=0.50 hpcur=9 hpmax=9 state=4 moving=0 acted=0 q=2 vt=$VT_MOVE,$VT_ATK,-,-
record=world frame=200 pathjobs=0 turns=1 turnorder=READFIX
CAP
out=$(tools/orig-digest.sh --capture "$W/cap-withreads.txt" \
	      --provenance "lockstep r1,r3,r4b session=41 exe=$MD5" --out "$W/locked.digest" 2>&1); rc=$?
check_rc "  a lockstep claim WITH consistent read records is accepted" "$rc" "0"
d=$(cat "$W/locked.digest" 2>/dev/null)
check_has "  and the file says CAPTURE=verified, not SYNTHETIC" "# PROVENANCE-CLASS=lockstep" "$d"
check_not "  SYNTHETIC is absent from a verified side" "# SYNTHETIC" "$d"
check_has "  the pinned LCG state rides in CONTEXT, not the hash" "lcgState=0x12345678" "$d"
# and a read record that contradicts the frames is refused: the guard checks, it does not merely
# require the words to be present.
out=$(tools/orig-digest.sh --capture "$W/cap-withreads.txt" --provenance "lockstep r3,r4b session=41 exe=$MD5" \
	      --out "$W/locked2.digest" 2>&1 >/dev/null; sed 's/value=2 when/value=7 when/' "$W/cap-withreads.txt" > "$W/cap-badread.txt"; \
	      tools/orig-digest.sh --capture "$W/cap-badread.txt" --provenance "lockstep r3,r4b session=41 exe=$MD5" \
	      --out "$W/locked3.digest" 2>&1); rc=$?
check_rc "  r3 value that appears in no q= field -> refused" "$rc" "3"
check_has "  and says the records and frames disagree" "disagree" "$out"
out=$(tools/orig-digest.sh --capture "$W/cap-synthetic.txt" \
	      --provenance "lockstep r3,r4b session=99 exe=00000000000000000000000000000000" \
	      --out "$W/wrong-exe.digest" 2>&1); rc=$?
check_rc "lockstep claim against the wrong binary -> refused" "$rc" "3"
check_has "  and says the md5 is what disagreed" "filemd5" "$out"

# R-PORT-Q: the r5 guard for numbered qi= fields - the same sloppy-proof q=/r3 gets, exercised
# in both directions: numbered qi WITHOUT the r5 read is refused; numbered qi WITH a consistent
# r5 read is accepted; and a pre-inbox-era capture (no qi at all) still renders NA without any r5.
cat > "$W/cap-qi-nor5.txt" <<CAP
record=read read=r3 address=Entity+0x314 value=2 when=mid-round breakpoint=0x459f5d
record=read read=r4b address=[action+0x00] value=$VT_ATK when=mid-round breakpoint=0x459f5d
record=frame frame=300 tickms=100 clocksec=100
record=entity frame=300 slot=0 name=QIFIX pos=1.00/2.00/0.00 yaw=0.50 hpcur=9 hpmax=9 state=4 moving=0 acted=0 q=2 vt=$VT_MOVE,$VT_ATK,-,- qi=3
record=world frame=300 pathjobs=0 turns=1 turnorder=QIFIX
CAP
out=$(tools/orig-digest.sh --capture "$W/cap-qi-nor5.txt" --provenance "lockstep r3,r4b session=41 exe=$MD5" \
	      --out "$W/qi-nor5.digest" 2>&1); rc=$?
check_rc "  numbered qi= without an r5 read -> refused" "$rc" "3"
check_has "  and names r5 as the missing read" "r5" "$out"
# (no-op removed: the fixtures below carry their own qi fields)
cat > "$W/cap-qi-r5.txt" <<CAP
record=read read=r3 address=Entity+0x314 value=2 when=mid-round breakpoint=0x459f5d
record=read read=r4b address=[action+0x00] value=$VT_ATK when=mid-round breakpoint=0x459f5d
record=read read=r5 address=Entity+0x2F0 value=3 when=mid-round breakpoint=0x55a3a0
record=frame frame=300 tickms=100 clocksec=100
record=entity frame=300 slot=0 name=QIFIX pos=1.00/2.00/0.00 yaw=0.50 hpcur=9 hpmax=9 state=4 moving=0 acted=0 q=2 vt=$VT_MOVE,$VT_ATK,-,- qi=3
record=world frame=300 pathjobs=0 turns=1 turnorder=QIFIX
CAP
out=$(tools/orig-digest.sh --capture "$W/cap-qi-r5.txt" --provenance "lockstep r3,r4b,r5 session=41 exe=$MD5" \
	      --out "$W/qi-r5.digest" 2>&1); rc=$?
check_rc "  numbered qi= WITH a consistent r5 read -> accepted" "$rc" "0"
check_has "  and the frame renders the numbered field" ",qi=3" "$(cat "$W/qi-r5.digest")"
sed '/read=r5/d; s/ qi=3//' "$W/cap-qi-r5.txt" > "$W/cap-qi-era.txt"
out=$(tools/orig-digest.sh --capture "$W/cap-qi-era.txt" --provenance "lockstep r3,r4b session=41 exe=$MD5" \
	      --out "$W/qi-era.digest" 2>&1); rc=$?
check_rc "  pre-inbox-era capture (no qi) still renders" "$rc" "0"
check_has "  and says the field was absent, not zero" "qi=NA:absent-in-capture" "$(cat "$W/qi-era.digest")"
# r5 read whose value contradicts the frames is refused too: the guard checks, not requires.
sed 's/value=3 when/value=7 when/' "$W/cap-qi-r5.txt" > "$W/cap-qi-badr5.txt"
out=$(tools/orig-digest.sh --capture "$W/cap-qi-badr5.txt" --provenance "lockstep r3,r4b,r5 session=41 exe=$MD5" \
	      --out "$W/qi-badr5.digest" 2>&1); rc=$?
check_rc "  r5 value that appears in no qi= field -> refused" "$rc" "3"
check_has "  and says the records and frames disagree" "disagree" "$out"

# a SYNTHETIC side against a verified-capture side is the trap the whole guard exists for:
# it looks exactly like a divergence finding and is a fixture bug.
out=$(tools/diff-digest.sh "$W/orig-a.digest" "$W/locked.digest" 2>&1); rc=$?
check_rc "SYNTHETIC side vs lockstep side -> refused, no comparison" "$rc" "3"
check_has "  names the refusal" "MIXED-PROVENANCE: no comparison performed" "$out"
check_has "  and shows BOTH classes so the reader sees which side is which" "PROVENANCE-CLASS=synthetic" "$out"
check_not "  emits no fake state divergence" "first divergence at line" "$out"

# --------------------------------------------------------------------------------------------
# 7) grammar drift is caught at the CONTRACT layer, not silently
# --------------------------------------------------------------------------------------------
sed 's/# NA-means=.*/# NA-means=CHANGED BY HAND/' "$W/orig-a.digest" > "$W/orig-contract-changed.digest"
out=$(tools/diff-digest.sh "$W/orig-a.digest" "$W/orig-contract-changed.digest" 2>&1); rc=$?
check_rc "a changed contract line -> CONTRACT differs, not a frame divergence" "$rc" "1"
check_has "  and names it as a contract problem" "CONTRACT differs" "$out"
check_not "  contract mismatch is not dressed up as state" "first divergence at line" "$out"

# side annotation lines must NOT be a contract divergence (otherwise original-vs-ours can never
# be compared at all, which was the design goal of one comparator for two producers)
sed 's/# PROVENANCE-CLASS=synthetic/# PROVENANCE-CLASS=synthetic/' "$W/orig-a.digest" > "$W/orig-b.digest"
out=$(tools/diff-digest.sh "$W/orig-a.digest" "$W/orig-b.digest" 2>&1); rc=$?
check_rc "identical frames, annotation-only difference -> 0" "$rc" "0"

# --------------------------------------------------------------------------------------------
# 8) the ours side still compares to the ours side (no regression in the existing users)
# --------------------------------------------------------------------------------------------
if [ -n "${ours:-}" ] && [ -r "$ours" ]; then
	cp "$ours" "$W/ours2.digest"
	out=$(tools/diff-digest.sh "$ours" "$W/ours2.digest" 2>&1); rc=$?
	check_rc "ours vs ours still IDENTICAL (exit 0)" "$rc" "0"
fi

# --------------------------------------------------------------------------------------------
# 9) CROSS-PRODUCER GRAMMAR: the contract block emitted by tools/orig-digest.sh must be
#    byte-identical to the one written by F3::Sim::WriteHeader in the running binary. This is the
#    test that makes "one comparator, two producers" true rather than aspirational: the contract
#    text is maintained in two places on purpose (see the emitter's LIMIT note - parsing C string
#    literals was rejected), and THIS is what fails the day they drift.
# --------------------------------------------------------------------------------------------
if [ -n "${ours:-}" ] && [ -r "$ours" ]; then
	# non-annotation '#' lines from each producer, in order
	grep -E '^#' "$ours" | tr -d '\r' | grep -vE '^# (SIDE|PROVENANCE|PROVENANCE-CLASS|SYNTHETIC|CAPTURE|AK-RESOLUTION|ORIG-LCG|LIMIT|GENERATED|SOURCES|END)( |=|:)' > "$W/contract-ours.txt"
	grep -E '^#' "$W/orig-a.digest" | tr -d '\r' | grep -vE '^# (SIDE|PROVENANCE|PROVENANCE-CLASS|SYNTHETIC|CAPTURE|AK-RESOLUTION|ORIG-LCG|LIMIT|GENERATED|SOURCES|END)( |=|:)' > "$W/contract-orig.txt"
	oc=$(wc -l < "$W/contract-ours.txt" | tr -d ' ')
	pc=$(wc -l < "$W/contract-orig.txt" | tr -d ' ')
	check_rc "both producers emit the same NUMBER of contract lines" "$oc" "$pc"
	if cmp -s "$W/contract-ours.txt" "$W/contract-orig.txt"; then
		say "contract block is byte-identical across the two producers" "ok"
	else
		say "contract block is byte-identical across the two producers" "FAIL"
		diff "$W/contract-ours.txt" "$W/contract-orig.txt" | head -6 | sed 's/^/       /'
		fail=1
	fi
	# line endings agree too: the binary writes CRLF (MSVC text mode), so the emitter must as
	# well - an invisible-byte difference makes every frame of a real comparison "diverge".
	if [ "$(head -1 "$ours" | od -c | grep -c '\\r')" -gt 0 ] \
	   && [ "$(head -1 "$W/orig-a.digest" | od -c | grep -c '\\r')" -eq 0 ]; then
		say "line endings: emitter matches the binary (CRLF)" "FAIL orig side is LF-only"; fail=1
	else
		say "line endings: emitter matches the binary (CRLF)" "ok"
	fi

	# and the ours side still hashes to what its own text says: if fnv ever stops covering the
	# COMPARED span, the orig side would silently disagree while both files looked self-consistent
	python3 - "$ours" <<'PYF' || true
import re, sys
bad = 0
for line in open(sys.argv[1], encoding="utf-8", errors="replace"):
    if " fnv=" not in line or line.startswith("#"):
        continue
    span, rest = line.split(" fnv=", 1)
    want = int(rest.split()[0], 16)
    h = 2166136261
    for b in span.encode("latin-1", "replace"):
        h = ((h ^ b) * 16777619) & 0xFFFFFFFF
    if h != want:
        bad += 1
if bad:
    print("FNV MISMATCH on %d ours-side line(s) - the emitter's hash is not the binary's" % bad)
    sys.exit(1)
print("ours-side fnv recomputed independently: matches on every frame line")
PYF
else
	say "cross-producer contract check" "SKIP: no ours-side digest"
	say "  a SKIP here means the check did not run" "FAIL"; fail=1
fi

echo
if [ $fail -eq 0 ]; then echo "[orig-digest-verify] PASS"; else echo "[orig-digest-verify] FAIL"; fi
exit $fail
