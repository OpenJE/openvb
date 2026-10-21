---
status: done
assignee: GoldYak `01a15f83` (claimed `4c256a5`, dispatched by LoudDragon after T-033 chain close; build lane only, no IDB entry, no devenv-process ops)
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

## Evidence (T-049 close, GoldYak 2025-10-21)

**CHANGE**: the T-046 pull-else-cached block ported verbatim (code shape reused, not re-designed) into
`devenv/scripts/msvc-build-debug.nix` and `devenv/scripts/msvc-build-tracing.nix`, replacing the
unconditional `"$docker" pull "${msvcImage}" >/dev/null`. Labels in printf lines are the per-script names;
comment cites T-046/harness-33 provenance. `nix-instantiate --parse` OK on both.

**ACTIVATION MECHANISM (advisory 2, stated explicitly)**: every run below is **direct invocation of the
script body extracted from the edited .nix** (nix interpolation resolved: pkgs paths, `${msvcImage}`,
`${cmakeVersion}`; `''${` unescaped) executed from the repo root. The session-PATH installed scripts are
STALE for ALL THREE (verified: `grep -c 'toolchain from local cache' $(command -v msvc-build{,,-debug,-tracing})` = 0/0/0 —
PATH still carries the pre-T-046 build; activation needs a coordinator re-up, not requested here, none
performed by me). No `devenv up`/`processes restart` was run; no IDB touched — `OPENVB_REIMPL_IDB` was
pointed at a scratch file in every leg (live 8746-held `build/F3.exe.i64` never entered).

1. **Bogus-registry proof (full script, both variants)** — alias `ghcr.io/openje/msvc2002-t049proof:latest`
tagged from the cached image (`docker tag`), upstream nonexistent:
   - debug:  first line `msvc-build-debug: toolchain from local cache (...) [sha256:f5cbf4de05ccc19db44346f54303b379a7dda4bb478391665a9fc205e8383f01 size=6614263241]; registry path failed: Error response from daemon: manifest unknown` → full build → **rc=0**, `Z:\work\build\F3-dbg.exe` linked 100% (log `/tmp/b4/t049_build_debug.log`)
   - tracing: identical shape, `msvc-build-tracing:` label → **rc=0**, `F3-trace.exe` linked (log `t049_build_tracing.log`)
   - refresh-leg spot-check with the REAL ref on both extracted bodies: `toolchain refreshed from registry` line, rc=0.
2. **Negative control (image nowhere)**: ref `ghcr.io/openje/msvc2002-t049absent:latest` (no alias) →
   both variants: stderr `msvc-build-{debug,tracing}: cannot pull ghcr.io/openje/msvc2002-t049absent:latest and it is not cached: Error response from daemon: manifest unknown`, **RC=1** — naming the image.
3. **Grep gate**: `grep -n '"\$docker" pull' devenv/scripts/*.nix | grep -v 'pull_out='` → **no output**
   (PASS); guard count `if pull_out=` = 1 in each of msvc-build.nix / msvc-build-debug.nix / msvc-build-tracing.nix.

**GATE RUN, legs distinguished (advisory 2)**:
   - canonical Release leg = direct-invocation of the extracted `msvc-build.nix` body (file UNCHANGED in
     this wave — T-046 landed shape): refreshed-from-registry line, **rc=0**, fresh `Z:\work\build\F3.exe` linked.
   - `run-all-tests`: **10 pass, 0 fail** against the fresh payload (`exe: 2026-10-21 14:34:27 -0400 57344`,
     app-boot exit 124 alive-past-6s; GUILoading 187≥116, ResourceSystem 7044 entries, PathManager 117≥117).
   - the debug/tracing legs touched build-msvc (Debug reconfigure) and build-msvc-tracing BEFORE the
     canonical leg restored Release; alias tag cleaned up (`docker rmi`) after.

**Boundary honored**: `docker-rootless.nix` NOT touched (T-046 activation note stands); scope exactly the
two reserved scripts.

## Boundary
`devenv/**` grant: same two-lane rule as T-046 (explicit coordinator grant required, scope exactly
these two files). Do NOT also edit `docker-rootless.nix` — its T-046 activation boundary (new
process script lands at next full session re-up; `devenv processes restart` reuses loaded config) is
recorded and is not a defect.
