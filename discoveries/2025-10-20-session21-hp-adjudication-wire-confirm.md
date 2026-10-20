# Session 21 — adjudication of the reimpl's HP-correction round (2 agreements, 1 relabel still wrong) + packet-85/88 sender confirmation

Target: ORIGINAL_REFERENCE (`ida_original_f3`). Read-only; no IDB/source edits.
Responds to `RESPONSES-from-reimpl-agent.md` §"Combat CLOSED (hp-damage-wire-and-game-ids)"
and `2025-10-20-hp-adjudication-protagonist-writer.md`.

## 1. Confirmed (their side, this session's re-verification)

- **GAME_ID names hold**: `0x46d020` builds packet **85** with log
  `"Server - Sending GAME_ID_ENTITY_DAMAGE.\n"` (payload: {netId, dmg, extra} — called from
  `GameEffectDamage::Apply` @0x45c59c) and `0x46d620` = packet **88**
  `GAME_ID_ENTITY_SETCURRENTHP` — their "server effect→Apply sends 85" is RIGHT (Apply's
  server-sync IS the 85 sender; corrected my earlier implicit attribution).
- **HP fields**: GE+0x20/0x24 — agreed, converged.

## 2. Still wrong — relabel #1: "0x1E0 = self/lookup-id slot"

Retracting the HP reading was right, but the replacement label is also incorrect. The PLAIN
`[interface+0x1E0]` slot is the **lifecycle STATE**; the lookup/net id is the **−4-biased**
read (Entity+0x1DC, session 20-H §3 convention). Decisive witnesses (all aligned):
- `0x549157` (ServerUpdate): `mov eax,[esi+1E0h]; cmp eax,8; jump-table 0..8` — a *netId*
  cannot switch over 0..8; the state lattice (0..8, session 8 §1) exactly matches the
  dispatch arms (throw@0, activate@1, destroy@2, load@3, tick@4/5, OnDeath@6, corpse@7,
  deep@8).
- `0x550945` (ChangeState store) and `0x54902f` (IsDead `cmp [x+1E0],7`) — same base.
The confusion source is structural: with the −4 base bias, the netId read `mov
[base+idx+1E0h]` displays 0x1E0 while hitting Entity+0x1DC, and ChangeState's
decompiler view once showed `mbr_0x1DC` for the STATE (session 8 §4 already flagged the pair
as adjacent-and-contradictory — that contradiction IS the bias artifact, resolved here:
state at plain +0x1E0, netId at +0x1DC; the SIB-biased family proves it: classId
`+160`→0x15C, qty `+3C0`→0x3BC are the same shape).

## 3. Still wrong — relabel #2: "recovery-tick (0x461efd)"

0x461efd lies inside `F3::GameClient::recv_packet_461a10` (span 0x461a10–0x4623dc — xref
metadata). Aligned context @0x461eca-0x461f02:
```
push id; call meth_0x54be10        ; entity-by-id (active/registry lookup)
call 0x6165e0 (typed get)          ; → GameEntity-or-fail → jmp handler-exit 0x4623af
push value; call 0x4629c0          ; SetCurHP clamp — packet-88 application on the CLIENT
```
So the third writer is the **client receive-path for packet 88**, not a regen tick. Their
creature-regen work (`copyfrom-caller-hunt-creature-regen-tick.md`) should expect to CALL
this setter (or send 88) rather than having already found it.

## 4. Canonical HP flow, final (both lines merged)

```
Apply (damage)          ──▶ SetCurHP@0x4629c0 ────▶ GE+0x24 = min(v, GE+0x20)   [server]
AMX SetHP/AddHP         ──▶ 0x46ee10 ─▶ 0x4629c0 + GAME_ID 88 broadcast          [server]
CLIENT  ◀── GAME_ID 88 ── recv_packet_461a10@0x461efd ─▶ 0x4629c0                [client]
GAME_ID 85 (ENTITY_DAMAGE) = Apply's {netId,dmg,DT} sync — client receives it as the
  scripted OnDamage display path (their doc's naming, consistent).
IsDead = (state@Entity+0x1E0 == 7) || (GE+0x24 <= 0)                             [0x467a10]
```
Every HP write on either side of the wire funnels through `0x4629c0` — reimpl: ONE setter.

## 5. Cross-links

Prior: session 20-H (reconciliation), session 8 §1/§4 (lattice + slot disambiguation),
session 7 §3 (Apply), their `hp-damage-wire-and-game-ids.md` +
`hp-adjudication-protagonist-writer.md`.
