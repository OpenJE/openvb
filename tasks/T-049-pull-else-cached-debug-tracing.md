---
status: in-progress
assignee: GoldYak `01a15f83` (T-049 dispatched by LoudDragon after T-033 chain close; coordinator grant given in dispatch, reservations: tasks file + both scripts. Build-lane only, no IDB entry.)
depends: T-046 (pattern established at 46f8c87)
---

# T-049 — Port the T-046 pull-else-cached path to `msvc-build-debug` and `msvc-build-tracing`

## Known (T-046 report, UltraMoon 01a15cb7, commit message records it explicitly)
`devenv/scripts/msvc-build-debug.nix` and `devenv/scripts/msvc-build-tracing.nix` still carry the
UNCONDITIONAL `docker pull` that item 33 condemned for the canonical gate — same network
dependency, same false "build failure" when the daemon's resolver is unhappy and the image is
cached. Out of T-046's two-file grant; filed here so it does not evaporate.

## Task
Apply the exact T-046 pattern: pull-else-cached, print which source was used (registry refresh line
vs cache line with resolved sha256+size and the failed pull's last error), hard exit 1 naming the
image when neither path has it. Reuse the code shape from `msvc-build.nix` post-46f8c87 — do not
re-design it.

## Acceptance
- Both scripts pass a bogus-registry run (aliased cached image, `manifest unknown` from the real
  registry) that prints the cache line with the digest, and build green.
- Negative control (image nowhere) exits 1 naming the image.
- A short grep gate: no remaining unconditional `"$docker" pull` without a fallback branch in
  devenv/scripts/*.nix — cite the grep in the row evidence.

## Boundary
`devenv/**` grant: same two-lane rule as T-046 (explicit coordinator grant required, scope exactly
these two files). Do NOT also edit `docker-rootless.nix` — its T-046 activation boundary (new
process script lands at next full session re-up; `devenv processes restart` reuses loaded config) is
recorded and is not a defect.
