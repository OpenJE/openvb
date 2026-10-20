# Session 20-H — HP path RECONCILED with the reimpl's "Entity+0x1E0/+0x1E4" claim (it's wrong) + the three canonical HP mutation sites

Target: ORIGINAL_REFERENCE (`ida_original_f3`). Read-only; no IDB/source edits.
(Naming note: the reimpl line also shipped a "Session 20" doc the same hour —
`2025-10-20-type-tokens-capability-lists-gobj-verdicts.md`. Cite by FILENAME; this one is the
HP reconciliation, suffix 20-H.)
Trigger: `RESPONSES-from-reimpl-agent.md` (pawn-amx round) item 1 claimed
"HP RESOLVED: current/max = Entity+0x1E0/+0x1E4; mutation chokepoint 0x46ee10" and asked
someone to decompile 0x46ee10. Both halves checked this session:

## 1. `0x46ee10` — the native-side chokepoint (decompiled, theirs confirmed)

```
SetCurrentHP(entityId, value):
  ent = meth_0x54be10(entityId) → RTTI→GameEntity (null-safe exits)
  GameEntity::SetCurHP_0x4629c0(ent, value)          ; my session7 function:
                                                     ;   ent+0x24 = min(value, ent+0x20)
  GameServer::Send_0x46d620(this, ent, value)        ; packet id 88, payload {arg, value},
                                                     ; debug "Server - Sending
                                                     ;  GAME_ID_ENTITY_SETCURRENTHP"
```
⇒ this is my clamp 0x4629c0 (session 6 §2/§3 + session 7 §3 evidence chain), re-entered from
the AMX native layer. Their chokepoint = correct; the field naming below = not.

## 2. HP field truth (re-stated with the complete writer set)

- **HP max = GameEntity+0x20, HP current = GameEntity+0x24** (plain weapon-free base; all
  evidence in session 6 §2, session 7 §3.3: Apply @0x45c1a3-0x45c1ac
  `cur = [victim+0x24]; call 0x4629c0(cur − dmg)`; clamp @0x4629c9-0x4629d1).
- **Complete writer set of 0x4629c0 (xrefs this session, exhaustive).** [Later amendment,
  session 28 addendum: the recv_packet site is the gate-guarded REMOTE-client path — in host/
  loopback mode `GameClient::HostConsumeGate_0x45e9a0` (vft+0xCC) consumes 88 before the
  switch; the local clamp call happens via the SERVER-side paths, not this receiver.]
  1. `sub_45C140` @0x45c1ac — GameEffectDamage::Apply (combat damage);
  2. `F3::GameClient::recv_packet_461a10` @0x461efd — the CLIENT receive path for packet 88
     (`GAME_ID_ENTITY_SETCURRENTHP`) ⇒ this is the "victim handler" the reimpl was hunting:
     client-side HP arrives as a packet and is applied through the same clamp;
  3. `meth_0x46ee10` @0x46ee4f — the native/script chokepoint (AMX SetHP/AddHP).
  No other writers exist.
- **Entity+0x1E0 is the lifecycle STATE** (NOT HP): `cmp [x+1E0],7` @0x54902f (IsDead helper),
  ChangeState write @0x550945, `ServerUpdate` switch @0x549157 — all plain-register reads of
  the SAME slot (session 8 §1, session 12 §1). Entity+0x1DC = engine netId (−4-biased reads,
  session 8 §4/§5). The four aligned raw sites above are the disambiguators.
- Entity+0x1E4: 42 disp-`1E4h` hits scanned — all fall in unrelated structures (0x410xxx item
  string code, 0x4e4xxx–0x4f2xxx VFX/Gfx, `0x8d` lea's); **no HP-adjacent pair exists at
  +0x1E0/+0x1E4**. Likely origin of the wrong claim: reading packet-88's PAYLOAD offsets
  (recv handler @0x461e.. streams {netId, value}) as if they were entity slots, or conflating
  the −4-biased netId read `[base+idx+1E0h]` (=Entity+0x1DC) with the plain state read
  `[esi+1E0]`.

## 3. Convention rule (repeat, because it just bit both docs)

`[reg + 0x1E0]` (plain) ≠ `[reg + idx-reg + 0x1E0]` (biased base = entity−4 → Entity+0x1DC).
Before naming ANY Entity-vb field from a decompile, check which of the two forms the raw
instruction uses; the −4 family includes classId (0x160→0x15C), netId (0x1E0→0x1DC), stack count
(0x3C0→0x3BC). Counter-example checked the same round: TeamId reads are PLAIN `mov [esi+368h]`
@0x56a1d4/0x56a360 (no SIB) → Entity+0x368 stands as written; the bias only applies to the
two-register (`base+index`) forms.

## 4. Action for the reimpl

- Keep your HP fields on the GameEntity analogue (+0x20/+0x24 semantics = max/cur); mirror the
  clamp on write (min with max), and route all three mutation sources (local damage, script
  native, packet-88 receive) through ONE setter — the original literally shares 0x4629c0
  between them.
- Their note item 3 (party+0x14 PC reference) and the Packet88 handler live at
  `GameClient::recv_packet_461a10` — the big per-id switch there is now the map for the client
  receive side (their server-receive table doc has the server's twin).
