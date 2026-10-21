# 2025-10-20 — T-016 read #2 CLOSED statically: [Mainmenu] map name + the shipped config is IN ENGINE.GRP

Method note first: this is the third instance of harness item 13 (data before debugger) and the
first where **the data was inside a shipped archive, not beside it**. Earlier "no .ini exists in
the payload" was nearly the excuse to book a debugger read — wrong: the demo's `globals.ini` is
stored as PLAINTEXT INSIDE `data/Engine.grp` (grep the archive, not just the tree).

## 1. The fallback chain — code (0x546fd0, one decompile)
Boot/request path (`JE::World::virt_meth_0x546fd0`, entered via `ChangeState(3)`):
```
candidate := "none.map"                                   (compiled seed, 0x547033)
tree      := sub_56AF40() = JE::Host::GetParameterTree    (globals.ini tree; my sim-driver stubs NULL)
if tree["Mainmenu"] found and ["Map Name"] in it:         (0x54706a..0x547142)
        candidate := that value
if ResourceSystem::exists(candidate):                     (0x54715d)
        world vft+88 request(candidate)
else: no request at all  — "none.map" is NOT a safety net that loads something; it only works
        if a file literally named none.map is in the archives.
```
⇒ ordering is **INI > compiled-seed, gated by resource existence**. No command-line door here;
`[Server] Start map` (below) is the NEW-GAME map, a different request path, not a boot fallback.

Case-sensitivity resolved too: tree lookup compares via `sub_433930` = `_stricmp` less-than
(both SSO branches, 0x433960/0x433975) — code key "Map Name" matches shipped "map name".

## 2. The shipped values — data (Engine.grp plaintext offsets)
| key | value | offset in Engine.grp |
|---|---|---|
| `[Mainmenu] map name` | **mainmenu.map** | 0x256 (section at 0x24a) |
| `[Mainmenu] target x/y/z, azimuth, elevation, distance, fov` | 0.0/5.5/0.0/0/0/0/35-ish block | 0x256+ |
| `[Server] Start map` | **00_03_Tutorial_Junktown.map** | 0x466 |
| `[Server] Start map entry point` | 0 | 0x466+ |
| `[World] Seconds Per Round` | **6** | (re-confirms T-010d from data, not code) |
| `[World] Year` | **2253** (+Month/…) | — see §3 |

## 3. Clock consequence, routed not owned
`[World] Year = 2253` means the **shipped world clock is date-seeded from config**, which bears
directly on the T-026 digest header's open clock question (`clock=29242` at frame 0 = our uptime
folding in). The StringSystem suite already tests "World-clock", so the reimpl models seeding;
what this adds is the shipped SEED VALUE (2253) and the proof the pump reads it from the same
tree. If a lockstep comparison ever pins clock, start there. ⚠ I did not verify where 2253 is
*consumed*, only that it ships — one citation short of the full claim, stated as such.

> **RESOLVED by WildYak T-045 (4bd2d53/80b45ce)** — the flagged question ("what does the original's
> pump pass as ms") goes the shipped's way, and my caveat earns its keep: the tick is an **elapsed
> accumulator**, not uptime. `0x56af50` = `mov eax,[0x70C2B8]; ret` (bytes verified independently this
> sitting: `a1 b8 c2 70 00 c3`); `0x70C2B8` is written ONLY by `F3::ProcessMessagesAndUpdateTime`
> (`elapsed += now − [0x70C2BC]`, boot-seeded at WinMain `0x56b8aa`); both globals sit past `.data`'s
> file-backed end (vsize 0x200e0 > rawsize 0x17000 — my own read, same geometry WildYak cited) so
> `elapsed` boots at 0. Consequences, per their page and my checks: (1) the frame-0 `clock=29242` fold
> is **OUR artifact** — `src/F3.cpp:786` keeps shipped's EA but returns raw `timeGetTime()`; (2) the
> faithful fix is seeding the stamp (mirror `0x5466b8/0x5466bd`), the change WorldTickPump previously
> forbade; (3) three pages still assert 'tick == uptime' (`SimDigest.md:173-180`,
> `ParityOracleDesign.md:47`, `OrigCapture.md:63`) — WildYak flagged, not mine to edit; (4) T-011:
> suspect this bug before believing any first-frame-anchored round-timing difference.

## 4. Boundaries
- "plaintext inside Engine.grp" = the demo payload's uncompressed archive content at these
  offsets; archive layout itself is unmodelled here (grep-visible, offsets cited).
- The 0x546fd0 decompile names v16/v17 confusingly (IDA split one 0x1C string across stack
  slots); assignment target verified as the candidate string via the SSO size-cap pattern.
- exists() = meth_0x49abd0 name from IDA; its body (does it consult a file table or try-open)
  unread — behaviorally constrained by: demo boots mainmenu.map, so exists("mainmenu.map")=true.

## 5. Evidence
0x547033 seed / 0x54706a section / 0x5470fa key / 0x547142 assign / 0x54715d exists / 0x547186
request; 0x433960/433975 _stricmp; Engine.grp 0x256/0x466 with surrounding text quoted;
payload path `/nix/store/a1j4...-f3demo-1.0/share/f3demo/data/Engine.grp`.
