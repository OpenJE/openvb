---
status: done
assignee: coordinator (NiceFalcon, 01a15a98) — shared infra, so one writer
depends: nothing
---

# T-024 — Make the gate trustworthy enough to quote

## Why
Two real failures this session. (1) The **installed** `run-all-tests` payload can be stale in an
un-reloaded shell: it reported 9 pass while `app-boot` did not exist in its payload, and a lane spent
ten minutes chasing my wrong number. (2) A suite can pass while **asserting nothing** — SpawnMove's
`grep -c` counted 2 where 6 were needed, then 0. Both mean a green number can be fiction.

## Scope
1. Stamp every gate run with: repo `git rev-parse HEAD`, submodule SHAs, the **absolute path of the
   payload actually executed**, and whether that payload matches the repo table.
2. Per-suite minimum case counts, asserted *by the suite*, so a pass with 0 assertions is a failure.
3. A `STALE-PAYLOAD` warning when the executed payload differs from `devenv/scripts/run-all-tests.nix`.
4. Keep the existing "9 pass, 0 fail" summary line shape — consumers parse it.

## Verdict (coordinator, `tools/gate-integrity.sh` + `devenv/scripts/run-all-tests.nix`)

Landed, and the row's own premise got falsified on the way in — which is the most valuable output:

1. **Stamp block** (`run-all-tests`): before any result it prints UTC time, `HEAD`, `openje` SHA,
   dirty totals (all files + build-affecting only), **payload path**, table row count + `cksum` hash,
   compiler version, and the `build/F3.exe` mtime+size. Every quoted number now travels with the
   identity of the program that produced it.
2. **Per-suite assertion floors**: rows gained a 4th field `min_checks` (11/74/111/20/13/6/116/80/43),
   each OK line prints `[N asserts >= M]`, and a suite below floor reports `ASSERT-FAIL` and counts as
   a failure. `OPENVB_TEST_FLOOR=Suite=N` is the built-in falsification switch.
   Boundary stated in the tool and in the row: the floor **counts `check()` calls in the source**, so
   it catches assertions being removed and suites going vacuous — it is **not** a runtime execution count.
3. **Diagnostics surface themselves** (implementer-B's ask): `BUILD-FAIL` now writes the full compiler
   log to `.devenv/state/tests/<Suite>.build.log` and prints the first error lines without `-v`; runtime
   `FAIL` does the same into `<Suite>.run.log`. No more re-running a row's `g++` line by hand to read why.

**The falsification the row demanded, and the discovery it produced.** Forced floor
(`OPENVB_TEST_FLOOR=CombatMath=99999`) → `ASSERT-FAIL`, exit 1. Injected broken extra source →
`BUILD-FAIL` with the actual `error: invalid use of 'this'...` printed, no `-v`. Control run → clean,
exit 0. And running the **installed** payload against my new table produced a **false RED**
(`CombatMath` undefined reference to `JE::LogDebugString`) **while printing no stale warning at all** —
because the guard I wrote lives inside the payload whose staleness it is supposed to detect, and the
running store copy predates it (zero occurrences of `STALE vs`). A self-checking gate cannot report on
its own version. So the real deliverable is **external**: `tools/gate-integrity.sh` checks table
identity + stamp presence from outside, one grep, no execution — exit 0 current, 2 stale table,
3 predates-stamp, 4 not found. Against today's payload it says exactly what happened:
`STALE ... stamp: ABSENT ... not quotable as evidence even when it says 9/9`.

**Consequence for every lane:** run `tools/gate-integrity.sh` before quoting a gate number, and treat a
gate run that prints no `GATE STAMP` as not-the-tree's-gate. My changes are in the tree, not in your
shells — **`devenv-reload` is required** to get the stamp, the floors, and the diagnostics.

## Acceptance
- Output makes it impossible to quote a gate number without also quoting its revision.
- Breaking a suite's case count below its floor turns the suite red while a real run stays green.
- `docs/notes/harness-items.md` gains the rule: a gate claim carries a timestamp **and** a git-status
   caveat, and the tooling now enforces the first half instead of relying on me remembering.

## Refs
- `docs/notes/harness-items.md` 8b, 16 (my stale-number failure, in my own words)
- UltraBear's B37/B43 "a pass that asserts nothing" reports
