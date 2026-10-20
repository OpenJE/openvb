---
status: open
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

## Acceptance
- Output makes it impossible to quote a gate number without also quoting its revision.
- Breaking a suite's case count below its floor turns the suite red while a real run stays green.
- `docs/notes/harness-items.md` gains the rule: a gate claim carries a timestamp **and** a git-status
   caveat, and the tooling now enforces the first half instead of relying on me remembering.

## Refs
- `docs/notes/harness-items.md` 8b, 16 (my stale-number failure, in my own words)
- UltraBear's B37/B43 "a pass that asserts nothing" reports
