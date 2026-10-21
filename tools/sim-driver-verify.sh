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

# 7) reach the round pump: some frame shows victim hp below max (a single-shive melee lands once)
grep -Eq 'hp=[0-9]+/30' /tmp/sd-a.digest || true
hit=$(awk '{ if (match($0,/victim,pos=[^ ]+,yaw=[^ ]+,hp=[0-9]+\/30/)) { s=substr($0,RSTART,RLENGTH); if (s ~ /hp=(2[0-9]|1[0-9]|0|[1-9])\//) { print "yes"; exit } } }' /tmp/sd-a.digest)
[ "$hit" = "yes" ] && r=0 || r=1
check "combat round reached: victim hp below max after orders" "$r" "0"

echo "[sim-driver-verify] $([ $fail -eq 0 ] && echo PASS || echo FAIL)"
exit $fail
