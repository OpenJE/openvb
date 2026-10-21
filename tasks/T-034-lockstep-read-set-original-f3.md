---
status: closed-as-miss (accepted alternative; retry re-filed as T-052)
assignee: NiceHawk `01a15cb7` (live half; prior RedXenon `01a157ec` handed over) — see LIVE HALF VERDICT section below
depends: nothing; required by T-031 (parity oracle) and by D-2's severity call
---

# T-034 — Lockstep read set against the original `F3.exe` (six reads, no code)

Self-contained experiment. **Prefer zero reads to guessed reads**: a value you cannot reach is a reported
miss, never an estimate — that is harness item 13's discipline and the reason this row is worth more than a
paragraph of agreement.

Target: shipped binary (`OPENVB_ORIG_EXE` = `${localPackages.f3demo}/share/f3demo/F3.exe`) under
`f3-lockstep-debug` / `devenv/scripts/debugIdaF3.nix` (gdb: **31337** original, 31338 reimpl). For each read
record: **address, value, the instant, the breakpoint**. Spec source: ZenRaven's
`docs/f3/ParityOracleDesign.md` §3; R0 and R4b added by the coordinator.

| # | read | address / field | when | why |
|---|---|---|---|---|
| **R0** | `[World vftptr + 24]` → target EA | break `0x552bb5` | first pump of a new game | `docs/f3/WorldTickPump.md` declares this its only assumption. DB check: `World` vftable `0x691a1c`, length 20, slots listed 0,1,2,4,5,7–17 — **3 and 6 are holes**, so statics cannot confirm the tick target is `virt_meth_0x544a90` |
| **R1** | `F3::start_time` dword | advanced `0x48ccde`, stored `0x48cce9` | first return of `sub_48CCC0` (break `0x48ccf6`) | the oracle's **pinned state**; design is capture-and-replay, so this becomes a measured constant |
| **R2** | `world+0x3C` → `Gregorian::totalSeconds` | | same instant as R1, then frame 1 | confirms the frame-0 clock leap is shipped behaviour, not ours |
| **R3** *(field list CORRECTED by HappyKnight, 633744d — old labels are wrong, do not use them)* | deque at `Entity+0x304`, five consecutive size-4 members: `+0x304` anchor / `+0x308` block-array ptr / `+0x30C` block capacity / `+0x310` head index / `+0x314` **LIVE COUNT** | mid combat round, after a script-native `F3::Attack` (`0x459f5d` → `call 0x550720`) | **D-2’s number is the COUNT at +0x314, not the head index.** Sampling +0x314 as an offset reports a depth as a position and gets the D-2 call wrong. shipped count>4 beside our 4 = a dropped order |
| **R4** *(reframed by WildYak's statics)* | enqueued action object `+0x14` **before** and **after** the funnel call | | immediately after R3's insert | must equal the entity pointer — proves the owner back-store fires on the **script-native path**, which our port reaches only from re-queue sites |
| **R4b** | `[action + 0x00]` = object **vftptr**, per deque slot | | same instant as R4 | lets the digest compare **class names** via `artifacts/F3.json` (97.9% of slots resolved) instead of pointers/kind tags. Print `UNKNOWN_VT=<ea>` for unresolved slots |

## R3 field list corrected (HappyKnight, T-039 at 633744d) — read this before attaching

My earlier table called `Entity+0x314` the element index. **It is the live COUNT.** Verified against the database:
`Entity` carries five consecutive size-4 members at `0x304/0x308/0x30C/0x310/0x314`, initialised **contiguously** in the
ctor (`0x5515d4`, `0x5515da`, `0x5515e0`, `0x5515e6`) — a 20-byte block deque embedded by value. Mapping: `+0x304`
anchor, `+0x308` block-array pointer, `+0x30C` block capacity, `+0x310` head element index, `+0x314` count. Readers
corroborate: `Entity::virt_meth_0x4084c0` is exactly `return Entity+0x314 == 0` (is my queue empty), and the pump bounds
its loop at `0x54c447`/`0x54c788` by `head != head + count`.

**Sample both halves**: count at `+0x314` (the D-2 quantity) and head at `+0x310`. R3 is now a *confirming* sample —
statics settled the semantics; a live value still settles depth (capacity 4 at this instant vs capacity 4).

## WildYak's statics change this read (01a15802, T-037)

The native **never** writes `+0x14`, and `ctor_0x403e40` doesn't either (its first store after `+0x10` is `+0x18`);
`0x550737 mov [ecx+14h],esi` inside the funnel is the **only** writer on that path. So R4 is no longer
"does anything set the owner back-link" — that is answered — it becomes: **is `+0x14` garbage or zero *before* the
call?** `new(0xC8)` may land on reused memory, so a defined pre-call value means either heap reuse or a writer nobody
has found, and both are findings worth reporting. Take the value on both sides of `0x550720`.

## UNBLOCKED without evicting ImHex (coordinator, 2026-10-21)

`31337` is still held by the live ImHex listener (pid 12337), and the restart is deferred until the user's in-flight work
finishes. **The port was only ever a convention** — `devenv/scripts/f3-lockstep-debug.nix` reads
`origPort=''${OPENVB_ORIG_GDB_PORT:-<default>}`, so an env override moves the stub. Verified free: **31437, 31438, 31441,
31556**. So the reads can proceed now with `OPENVB_ORIG_GDB_PORT=31437`, leaving someone else's session intact — which is
also the correct precedent: work around a shared resource rather than evict a peer from it.

Note the IDB is served by shared MCP processes (`.idalib-mcp-wrapper` on **8745** original / **8746** reimpl), so an
"IDB writer" is whoever is issuing writes through that one session — single-writer discipline applies to the MCP, not to
a person's local file.

## PARKED as a harness action (2026-10-21) — three honest attach failures, no reads claimed

RedXenon reported the attempt straight: port-corrected tool (`COUNT@+0x314`, `HEAD@+0x310`), override used
(`OPENVB_ORIG_GDB_PORT=31437`), fresh stubs started three times — first caught a running-free inferior and died
breaking while running (fix landed: interrupt-tolerant arming), second timed out connecting, third stub did not survive
to the attach. **Zero reads claimed; nothing sampled.** Diagnosis: the listener-dies-with-gdb lifecycle plus a demo
prefix that has cycled ~4 stub sessions today needs **a clean wineprefix reset**, not another lane guessing.

Reframing that lowers this row's priority — **its blocking value is gone.** D-2's severity is now measured
(`8b6c982` + `0160a9c`), so the live depth sample is confirmation rather than the only route. What genuinely remains
live-only: **R0** (World vftable slot 6, a hole in every reading — `x/1wx $ebx+0x18` at `0x552bb5`) and **R4's
asymmetry** (zero = non-event; defined non-zero = a second owner-writer, evidence against T-037's own row). So the row
stays open but no lane burns context on flaky attaches until the prefix is reset.

## Environment status (2026-10-21) — two independent bugs found, one still needs the user

> **CORRECTION, same day (see harness item 39):** item 1 below was **wrong** — ImHex cannot be moved. Its
> Network Interface port is hardcoded to 31337 in the binary; `ss` showed the ImHex pid on 31337 while the
> config said 31347, and the settings file's `port` key is never read. So the "ImHex → 31347" edit did not
> free 31337 for this stub: it only made `run-imhex-mcp` wait on a port that can never open, time out at
> 120 s, kill a healthy ImHex, and restart-loop (`devenv processes list`: `imhex restart_count 5`), while
> `ida-orig-gdb` still exited on the collision. Fixed properly now: **the original's stub moves to 31437**
> and `OPENVB_IMHEX_MCP_PORT` returns to the true 31337. Item 2's owner-verified probe is kept and extended
> to ImHex (`ready.exec` was `coreutils true`, i.e. always green).

NiceHawk's handover (`b77625e`) diagnosed more than a port collision. Both parts are now fixed in the tree:

1. **Port collision** — ImHex and the original's gdb stub were both configured for 31337. Fixed: ImHex → 31347 and
   the literal no longer exists in three places (`fd6a578`). **The live listener still owns 31337 until the user
   restarts ImHex** — deferred until their in-flight work finishes; nobody should evict a session they don't own.
2. **Readiness check could not detect it** — `ready.exec = nc -z 127.0.0.1 31337` passes against *any* listener, so
   devenv reported the dead stub "ready". Replaced with an owner-verified probe. Measured surprise along the way:
   **`ss` reports the gdb socket as `wineserver`, not `winedbg`**, so a `grep -q winedbg` matcher would have
   reported NOT READY on a healthy stub — a false red replacing the false green. Matcher is now `wineserver|winedbg`,
   proven in both directions (healthy stub rc=0; ImHex on 31337 rc=1), with its own limit recorded: a killed stub
   leaves wineserver holding the port and the probe still says ready, so it rules out squatters, not liveness.
3. **Input automation was the real R3/R4b blocker** — no `xdotool`/`wmctrl` in the shell, so the shipped demo could not
   be driven menu→combat at all. Both added (`pkgs.xdotool`, `pkgs.wmctrl`, attr names resolved against the pinned
   nixpkgs). If R3 still can't be reached after that, the row's fallback stands: settle D-2 from T-037's record map
   plus a stress test on our own seam, and record R3 as unreachable-by-tooling.
4. **False negative to expect**: `0x552bab` dereferences `[esi]` unconditionally and `esi` is null when `[0x70BD98]==0`
   — at the main menu the pump body never runs, so R0 "never fired" ≠ "attach failed". R1 hits regardless.

## Environment status (2026-10-21)

- **Port collision fixed in the tree** (`devenv/environment/openvb.nix` + both consumers now read the variable): ImHex
  moved 31337 → **31347**, so `31337` is uniquely the original's gdb port. Verified by evaluating the consumer:
  `devenv print-dev-env` exits 0 and exports `OPENVB_IMHEX_MCP_PORT='31347'`.
  - **SUPERSEDED (harness item 39):** the measurement above only proved the *variable* changed, not that the
    *socket* moved — ImHex binds 31337 regardless. The resolution is `OPENVB_ORIG_GDB_PORT='31437'` /
    `OPENVB_IMHEX_MCP_PORT='31337'`, and the original stub and ImHex now coexist. Anyone attaching to the
    **original** in IDA: remote gdb port is **31437** now, not 31337. Reimpl stays **31338**.
- **Still blocked on one manual step:** the *running* ImHex listener owns `127.0.0.1:31337` until it is restarted with
  the new config. That is a user/harness action, not a lane action — nobody should evict a session they don't own.
- Fresh stubs in this session's state proved unreliable (one `run`-over-RSP failure, one RSP timeout, one silent exit
  seconds after launch). Prefer launching through `f3-lockstep-debug` after the restart so both stubs have one owner.
- All six targets are **pre-resolved to loadable expressions** (`44544f0`), so once the port frees: one attach, one sitting.

## Acceptance
- One `discoveries/` doc: a six-row table (address / value / instant / breakpoint) **plus an explicit list of
  what was not reached and why**.
- R3/R4 are the valuable pair. Reaching a live entity mid-round is the point — **reading a cold entity is not a
  substitute**; the question is queue *depth under load*. If mid-round is unreachable, report the attempt.
- If R0 resolves to an EA other than `0x544a90`, say so loudly: `docs/f3/WorldTickPump.md` is then wrong and the
  tick-pump story needs re-deriving.

## Boundary
Six dwords under a debugger see a moment, not a mechanism. A single sample cannot distinguish "capacity 4" from
"capacity 4 at this instant"; if R3's index looks suspiciously near the cap, say that instead of concluding.

## LIVE HALF VERDICT 2026-10-21 (NiceHawk `01a15cb7`, post-incident session): ZERO of six rows - honest miss list per the row's rule
Full capture report + six-row table (VALUE column = NOT READ + why): `discoveries/2025-10-21-t034-live-capture-orig-boot-exception.md` (committed `5717bd9`). Summary:
- 5 attach cycles, all clean (MZ@0x400000 confirmed each time); ImHex/31337 never touched, 31437 workaround attached every cycle.
- THE BLOCKER (new, mechanism named): the gdb-INSTRUMENTED boot never consumes wine's startup `DbgBreakPoint` - main thread parked at `ntdll+0xe229` (4 identical interrupt samples), TICK probe `0x56af50` NEVER fired => the engine never completed a frame; `0x48ccf6`/`0x552bb5`/`0x550720`/`0x459f5d` all armed, none hit. Plain `continue` on this winedbg+gdb17.2 pair blocks forever; async is dead on the stub (continue & accepted, interrupt no-ops, no wait, stepi refused). Prior "alive at menu" was observed WITHOUT gdb attached - that delta is the whole game.
- NOT the blocker: port collision (fixed), xdotool (installed; no mapped window to click), attach (works).
- R0 loud-flag: no live contradiction to T-045 - the pump never ran at all; WorldTickPump assumption stays UNFALSIFIED, not confirmed. R1 handover claim "menus draw dice" did NOT reproduce instrumented.
- No cold-read padding taken (row forbids); R3/R4/R4b reported unreachable-by-tooling this session.
- NEXT (post-HOLD, §5 of the doc): fix the startup-stop dance first - winedbg WITHOUT `--no-start` + double-resume-after-attach before arming any judgment; if window still unmaps, escalate wined3d env (LIBGL_ALWAYS_SOFTWARE=1 / Xvfb, render-device-probe case 3). Working harness v3 (interrupt-tolerant, TICK probe, R4 pre/post, R4b deque walk) at /tmp/t034-run.py - re-land under tools/ at the retry owner's choice.
- Incident (disclosed, accepted): cross-prefix `pgrep -x F3.exe` kill removed the reimpl inferior 320383; 31338 stub 320381 left as-found (LISTEN, orphan); repair = `devenv processes restart ida-reimpl-gdb`, controller-owned. Durable rule proposed as harness item 38: comm-grep is never an owner test; gate kills on /proc/<pid>/environ WINEPREFIX.
- Environment left CLEAN: stubs killed, no IDB writes, no devenv edits, no commits by the lane.

**Row status: LIVE half CLOSED-as-miss.** The six dwords remain unread; the row is now GATED on the §5 harness retry, not on effort. Board row reflects this.
