---
status: open
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
