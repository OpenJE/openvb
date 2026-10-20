#!/usr/bin/env bash
# tools/gate-integrity.sh — external staleness + provenance check for the host gate (T-024).
#
# WHY THIS EXISTS AS A SEPARATE FILE
# run-all-tests lives inside the very payload whose staleness it tries to detect, so a stale payload
# simply has no guard: on 2026-10-20 the installed script contained ZERO occurrences of the STALE
# warning that the tree file has, and it reported CombatMath as a RED suite (undefined reference to
# JE::LogDebugString) purely because ITS table was older than the tree. A self-checking tool cannot
# report on its own version. This file checks from outside, costs one grep, and needs no execution.
#
# usage: tools/gate-integrity.sh
# exit:  0 payload matches the tree's table and carries the integrity stamp
#        2 payload table DIFFERS from the tree (results are measuring the installed rows: devenv-reload)
#        3 payload predates the T-024 stamp block (its numbers are unattributable)
#        4 cannot locate the gate payload or the tree file
#
# BOUNDARY (rule 3): this compares the SUITE TABLE and checks for the stamp block. It does NOT verify
# that the rest of the payload body matches the tree, that the compiler is the one you expect, or that
# any suite actually passed. It answers exactly one question - "is the gate I am about to quote from
# the same program as the gate in the tree?" - which is the question that produced three false results
# today (harness items 8b, 14, 17).
set -uo pipefail

root="$PWD"
src="$root/devenv/scripts/run-all-tests.nix"
row_re='"[A-Za-z0-9_]+\|[^"]*\.cpp[^"]*"'

launcher=$(command -v run-all-tests 2>/dev/null)
[ -n "$launcher" ] || { echo 'gate-integrity: no run-all-tests on PATH (are you in the devenv shell?)'; exit 4; }
[ -f "$src" ] || { echo "gate-integrity: tree file missing: $src"; exit 4; }

# devenv scripts are a 3-line launcher that execs the real payload; follow it.
payload=$(grep -oE '/nix/store/[a-z0-9-]+-run-all-tests-script' "$launcher" | head -1)
[ -n "$payload" ] && [ -f "$payload" ] || payload="$launcher"

head_sha=$( (cd "$root" && git rev-parse --short HEAD 2>/dev/null) || echo unknown)
openje_sha=$( (cd "$root/modules/openje" && git rev-parse --short HEAD 2>/dev/null) || echo unknown)
dirty=$( (cd "$root" && git status --porcelain -- cmake.toml CMakeLists.txt src devenv modules 2>/dev/null) | grep -c . || true)

printf 'payload  : %s\n' "$payload"
printf 'HEAD     : %s   openje: %s   build-affecting dirty: %s\n' "$head_sha" "$openje_sha" "${dirty:-0}"

want=$(grep -oE "$row_re" "$src")
held=$(grep -oE "$row_re" "$payload")

rc=0
if [ "$held" != "$want" ]; then
  printf 'STALE    : installed table differs from %s\n' "$src"
  printf '           installed rows: %s, tree rows: %s\n' \
    "$(printf '%s\n' "$held" | grep -c .)" "$(printf '%s\n' "$want" | grep -c .)"
  diff <(printf '%s\n' "$held") <(printf '%s\n' "$want") | grep -E '^[<>]' | cut -c1-72 | sed 's/^/    /' | head -8
  printf '           => results from the installed gate measure the INSTALLED rows. devenv-reload.\n'
  rc=2
else
  printf 'table    : OK (%s rows identical to the tree)\n' "$(printf '%s\n' "$want" | grep -c .)"
fi

if ! grep -q 'GATE STAMP' "$payload"; then
  printf 'stamp    : ABSENT - this payload predates the T-024 integrity stamp.\n'
  printf '           Any number it prints carries no revision, payload path or table hash, so it is\n'
  printf '           not quotable as evidence even when it says 9/9.\n'
  [ $rc -eq 0 ] && rc=3
else
  printf 'stamp    : present (run-all-tests prints HEAD/openje/payload/table-hash before any result)\n'
fi

[ $rc -eq 0 ] && printf 'verdict  : gate payload is current and attributable\n'
exit $rc
