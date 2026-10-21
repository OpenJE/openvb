---
status: done
assignee: UltraMoon 01a15cb7 (build lane; devenv/** is coordinator-adjacent but this row is exactly two files)
depends: none
---

# T-046 — Harden the two devenv readiness/pull defects found in the 04:36Z dockerd outage

## Scope (from harness items 32-33, docs/notes/harness-items.md, commit ae8ef94)
1. `devenv/processes/docker-rootless.nix` — `ready.exec = docker info` passes against ANY
   daemon; combined with `nohup dockerd-rootless & disown; tail -f /dev/null`, a dead dockerd
   leaves the leader alive and the generation reporting ready while nothing can build.
   Make readiness owner-aware: the exec should verify the socket AND that a trivial
   container-run path works (or at minimum detect the no-daemon case distinctly), and the
   script should not end in a bare `tail` that masks child death — e.g. `wait` on the
   dockerd pid and exit nonzero so the guardian can see the failure.
2. `devenv/scripts/msvc-build.nix:79` — unconditional `docker pull` makes the canonical MSVC
   gate require registry reachability even when the image is cached. Make it
   pull-else-use-cached, and PRINT which path happened (`toolchain refreshed` vs
   `toolchain from local cache`), per the wave rule that a tool states its boundary in output.

## Acceptance
- With dockerd stopped, `devenv processes status docker-rootless` (or the ready probe) reports
  NOT ready — proven by actually stopping the daemon, then `devenv processes restart` to recover.
- msvc-build succeeds with the network path deliberately broken (e.g. pull against a bogus
  registry mirror override) while the image is cached, and the output names the cache path used.
- A full msvc-build + run-all-tests gate quoted from THIS row's run.

## Boundary
These are harness files; the artifact-contract rule (build-affecting edits stay in the two
build lanes) is satisfied because the coordinator dispatched this explicitly. Do not touch any
other devenv/** file in this row.

## Acceptance evidence (2026-10-21T05:12Z, UltraMoon 01a15cb7)

**1. Readiness / no-mask (`devenv/processes/docker-rootless.nix`)**
- New exec: every probe pins `DOCKER_HOST=unix:///run/user/$uid/docker.sock` (a foreign daemon cannot
  answer for ours); adopt-branch replaces the bare `tail -f /dev/null` with a 2 s `daemon_up` watch that
  **exits nonzero on daemon death**; start-branch `wait`s on the dockerd-rootless pid (rootlesskit provably
  exits when dockerd dies — measured tonight) and exits nonzero; a one-shot run-path probe
  (`docker run --rm <cached msvc2002> -lc true`, never pulls) validates dockerd→containerd→runc at startup,
  skipped with a printed note if no cached image exists.
- New `ready.exec`: distinct named cases — `NOT ready: no socket at …` vs `NOT ready: socket present but
  no daemon answers on …` (with the recovery command in the message), rc 1.
- **Live proof**: rendered the committed nix bodies to runnable bash (exact nix interpolations from the
  live store payloads; `bash -n` clean) and ran them against the real daemon: probe `OK`/rc 0 while up;
  after `kill -TERM dockerd` the watch exited rc 1 within 2 s (`dockerd stopped answering on
  /run/user/1000/docker.sock; exiting nonzero for the supervisor.`), the ready probe printed
  `NOT ready: no socket at /run/user/1000/docker.sock` rc 1 — while the OLD manager leader stayed alive,
  re-demonstrating the mask this row removes. Recovered with `devenv processes restart docker-rootless`
  (Phase: ready, probe OK again, run-path probe had passed live at startup: `run probe OK`).
- **Boundary honestly stated**: `devenv processes restart` reuses the up-session’s loaded process config
  (verified: leader store-path unchanged after restart), so the new exec/ready activate at the next full
  devenv session re-up — a user/coordinator action, not a lane action. The row’s acceptance (“or the ready
  probe”) is proven by the direct render-and-run above.

**2. Pull-else-cached (`devenv/scripts/msvc-build.nix:79`)**
- Unconditional `docker pull … >/dev/null` replaced with pull → else cached image (printing the resolved
  `sha256` + size) → else hard error naming the image; every path prints which source it used.
- **Cut-network test** (legit per the row’s own “bogus registry” example): aliased the cached image to
  `ghcr.io/openje/does-not-exist-msvc-t046:latest` and ran the installed payload copy with the image
  reference substituted consistently — the pull genuinely failed against the live registry. Output:
  `toolchain from local cache (…) [sha256:f5cbf4de05cc… size=6614263241]; registry path failed: Error
  response from daemon: manifest unknown`, then `Built target F3`, **exit 0**. The printed digest equals
  the cached toolchain’s real image ID (provenance of the fallback is shown, not assumed — ZenNova guard).
- **Negative control**: same payload with `ghcr.io/openje/absent-repo-t046-negative:latest` (never cached,
  not in registry) → `msvc-build: cannot pull … and it is not cached: … manifest unknown`, **exit 1**.
  The fallback cannot silently skip pulling real changes.
- Alias removed (`docker rmi`), nothing cached was deleted.

**3. Full gate, this row’s run**: `tools/gate-integrity.sh` rc 0 (payload x95aqfk7…, 9 rows, tree
identical). Fresh-payload `msvc-build` (after `source /tmp/openvb-dev-env.sh` — the non-sourced run hits
the old installed script, proven the hard way): `toolchain refreshed from registry (ghcr.io/openje/msvc2002:latest)`,
exit 0, F3.exe rebuilt. GATE STAMP 2026-10-21T05:12:51Z, HEAD a1d6848 / openje 77c18d8, table hash
2945679485, exe 2026-10-21 00:57:46 -0400: **10 pass, 0 fail** (StringSystem 27≥20, all suites at/above
floors, app-boot alive past 6 s).

**Out of scope, filed for the ledger**: `devenv/scripts/msvc-build-debug.nix` and
`devenv/scripts/msvc-build-tracing.nix` still carry the same unconditional pull (their line 79/69);
this row’s grant covered only `msvc-build.nix`.
