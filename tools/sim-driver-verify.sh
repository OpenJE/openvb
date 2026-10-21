#!/usr/bin/env bash
# T-028 falsification suite for the scripted-input driver (wave rule 2: a tool that cannot
# be shown to fail is decoration). Every check below has a MUTANT: if the driver silently
# stopped working, one of these lines turns red.
set -uo pipefail
cd "$(git rev-parse --show-toplevel)"

fail=0
say() { printf '%-58s %s\n' "$1" "$2"; }
check() { if [ "$2" = "$3" ]; then say "$1" "ok ($2)"; else say "$1" "FAIL got=$2 want=$3"; fail=1; fi; }

sh tools/sim-driver-build.sh >/tmp/sd-build.log 2>&1 || { echo "BUILD-FAIL (see /tmp/sd-build.log)"; tail -20 /tmp/sd-build.log; exit 1; }

# WINE resolution identical to the suite runner
WINE=$(command -v wine || ls /nix/store/*/bin/wine 2>/dev/null | head -1)
[ -n "$WINE" ] || { echo "verify FAIL: no wine"; exit 1; }
run() { "$WINE" tools/sim-driver.exe "$1" >/dev/null 2>&1; }

# 1) determinism: same script twice -> byte-identical digests (diff-digest.sh exit 0)
sed 's#/tmp/sd-demo.digest#/tmp/sd-a.digest#' tools/scripts/host-demo.txt > /tmp/sd-a.txt
sed 's#/tmp/sd-demo.digest#/tmp/sd-b.digest#' tools/scripts/host-demo.txt > /tmp/sd-b.txt
run /tmp/sd-a.txt
run /tmp/sd-b.txt
./tools/diff-digest.sh /tmp/sd-a.digest /tmp/sd-b.digest >/dev/null 2>&1
check "same script twice -> identical digests (want 0)" "$?" "0"

# 2) mutation reaches the digest: bump flatDamageBonus -> divergence that NAMES a field
sed 's/attack actor victim 2 2 0 0/attack actor victim 2 2 0 9/; s#/tmp/sd-demo.digest#/tmp/sd-c.digest#' tools/scripts/host-demo.txt > /tmp/sd-c.txt
run /tmp/sd-c.txt
./tools/diff-digest.sh /tmp/sd-a.digest /tmp/sd-c.digest >/tmp/sd-diff.txt 2>&1
rc=$?
[ $rc -eq 1 ] || true
check "damage-bonus mutant -> diverges (want 1)" "$rc" "1"
grep -Eq "e[0-9]+\.[a-z]+|hp" /tmp/sd-diff.txt
check "divergence names a field (not just hash)" "$?" "0"

# 3) grammar honesty: unknown verb FAILS the run instead of skipping quietly
printf 'frobnicate 1\n' > /tmp/sd-bad.txt
run /tmp/sd-bad.txt
check "unknown verb -> nonzero exit" "$?" "1"

# 4) duplicate spawn name is refused (script integrity before sim integrity)
printf 'spawn a 0 0 0 0 45\nspawn a 1 1 0 0 45\n' > /tmp/sd-dup.txt
run /tmp/sd-dup.txt
check "duplicate entity name -> nonzero exit" "$?" "1"

# 5) boundary is PRINTED, not just in the README (wave rule 3)
("$WINE" tools/sim-driver.exe /tmp/sd-a.txt 2>&1 >/dev/null | head -1) | grep -q "not the 8-slot GameStateLoop"
check "boundary statement printed on stderr" "$?" "0"

# 6) draws never lies: the demo digest must NOT contain draws=0 while it can contain draws=NA
grep -c 'draws=0' /tmp/sd-a.digest | { read n; check "draws=0 absent (NA is the honest value)" "${n:-0}" "0"; }
grep -q 'draws=NA' /tmp/sd-a.digest
check "draws=NA present" "$?" "0"

# 6b) D-3 lifecycle boundary PRINTED, not claimed as coverage: target is a raw pointer in
# our port; the shipped record stores a ref KEY re-resolved every use (0x54BE10), so
# "target despawned mid-attack" is UNREPRESENTABLE here, not merely untested.
"$WINE" tools/sim-driver.exe /tmp/sd-a.txt 2>&1 >/dev/null | grep -q "GameStateLoop"
d3=$(grep -c "unrepresentable\|ref-key\|despawn" tools/sim-driver.cpp)
[ "$d3" -ge 1 ] && echo "D-3 boundary documented in driver source" || echo "D-3 MISSING"

# 7) reach the round pump: some frame shows victim hp below max (a single-shive melee lands once)
grep -Eq 'hp=[0-9]+/30' /tmp/sd-a.digest || true
hit=$(awk '{ if (match($0,/victim,pos=[^ ]+,yaw=[^ ]+,hp=[0-9]+\/30/)) { s=substr($0,RSTART,RLENGTH); if (s ~ /hp=(2[0-9]|1[0-9]|0|[1-9])\//) { print "yes"; exit } } }' /tmp/sd-a.digest)
[ "$hit" = "yes" ] && r=0 || r=1
check "combat round reached: victim hp below max after orders" "$r" "0"

# 8) T-051: the driver must NEVER arm the dead walk branch - the audit PRINTS its counts, and
#    the branch trace reports the manager's live mode (free-run expected; round-walk = bug).
arming=$(grep -c "SetRoundMode" tools/sim-driver.cpp || true)
check "sim-driver SetRoundMode call sites (want 0: no invented arming, T-051)" "${arming:-0}" "0"
tr=$("$WINE" tools/sim-driver.exe /tmp/sd-a.txt 2>&1 >/dev/null | grep -o "combat branch: [a-z-]*" | head -1)
check "branch trace line present and names free-run" "$tr" "combat branch: free-run"
if printf '%s' "$tr" | grep -q "round-walk"; then
	say "T-051 walk-arming guard" "FAIL (driver armed the shipped-dead branch)"; fail=1
else
	say "T-051 walk-arming guard" "ok (no round-walk trace; free-run only)"
fi


# 9) T-047 attack-pos (packet-91 place-attack model): enqueues + dispatches with NO victim
#    entity; the actor walks to the coordinates; a bystander victim is untouched; and the
#    NEGATIVE pairing: bare `attack` naming a missing target still FAILS the run.
printf 'spawn walker 0 0 0 0 45\nspawn bystand 1 9 0 0 45\ndigest /tmp/sd-pos.digest\nattack-pos walker 3 0 0\nrun 40 16\nend\n' > /tmp/sd-pos.txt
rm -f /tmp/sd-pos.digest   # never grade a stale digest if the previous wineserver lags
"$WINE" tools/sim-driver.exe /tmp/sd-pos.txt >/dev/null 2>&1
check "attack-pos run exits 0" "$?" "0"
[ -s /tmp/sd-pos.digest ]
check "pos digest exists and is non-empty" "$?" "0"
grep -q 'walker.*ak=GameActionAttack' /tmp/sd-pos.digest
check "digest shows the place-attack ON the actor (ak=GameActionAttack)" "$?" "0"
walked=$(grep -o 'walker,pos=[0-9.]*' /tmp/sd-pos.digest | cut -d= -f2 | sort -g | tail -1 | awk '{print ($1>1.0)?"yes":"no"}')
check "actor walked toward the coordinates (max x past 1.0)" "${walked:-no}" "yes"
hurt=$(grep 'bystand' /tmp/sd-pos.digest | grep -vc 'hp=45/45' || true)
check "bystander untouched (no invented victim)" "${hurt:-0}" "0"
printf 'spawn solo 0 0 0 0 45\nattack solo nosuch\nrun 5 16\nend\n' > /tmp/sd-neg.txt
"$WINE" tools/sim-driver.exe /tmp/sd-neg.txt >/dev/null 2>&1
check "negative pairing: bare attack with missing target still refuses (want 1)" "$?" "1"

# 10) T-047 grammar: incomplete attack-pos FAILS (not a quiet skip)
printf 'spawn w 0 0 0 0 45\nattack-pos w 1 1\nend\n' > /tmp/sd-pos-bad.txt
"$WINE" tools/sim-driver.exe /tmp/sd-pos-bad.txt >/dev/null 2>&1
check "attack-pos missing coordinate -> nonzero exit" "$?" "1"

echo "[sim-driver-verify] $([ $fail -eq 0 ] && echo PASS || echo FAIL)"
exit $fail
