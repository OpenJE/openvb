# Session 33 [RETITLED 2nd pass — does NOT close T-010c, see §0] — 0x6FF650 is JE::ModelInstance's SOCKET-name table (SetSocketModel_564f90); proven: the 9-part SET and id range 0..8 (9=UNSET); RETRACTED: the ORDER claim (socket order may be an unrelated index space — FastArrow's equip-slot finding cuts the bridge) + tooling hazard note

Target: ORIGINAL_REFERENCE (`ida_original_f3`). Read-only. Supersedes session32 §1.3's
"combat-UI init" attribution (mine — corrected, not B's or F's).

## 0. Status correction after FastArrow's B7 (verified by me at the raw this turn)

FastArrow's chain CONFIRMED independently: `GameInventory::ctor_0x46b970` memset(&mbr_0x14, 0,
0x18) ⇒ 6-dword equip-slot array; `vft[+40]=0x46A860` returns node {prev,next,**slotIndex**}
via `sub_433760(v5[0], v5[1], &local_8=loopCounter)` — loop `v3 < 6`, and aggregation
`meth_0x40A670` keys rows on `node+8`. ⇒ row A/B are **EQUIP-SLOT coverage** (slots 0/1),
not an anatomy partition per se. My §2 ORDER derivation consumed the socket list's display
order as the enum order; if the socket vector is ModelInstance art-index space, that bridge
is unsubstantiated — RETRACTED to hypothesis. **What survives proof** (independent of socket
order): (i) the called-target id SET = exactly the 9 names (Chest/Feet/Hands/Head/Legs/
UpperArms/Waist/ForeArms/Shins), ids 0..8, 9=UNSET — arithmetic of 9-names/9=UNSET/
FLT_MAX-slot is order-free; (ii) numeric grouping {0,1,2,3,4,7}→row A / {5,6}→row B /
default→log+row A (both damage fns); (iii) DR rows = 6 DT values each. The ORDER is now a
question of which slot-slot-1-gear protects: if slot1 = head-gear ⇒ my enum order is
refuted outright (head-family would need ids 5,6); if slot1 = leg-gear ⇒ {5,6}=Feet/Waist
would CORROBORATE part of it. Pending FastArrow's GameActionEquip slot-choice pass — naming
stays numeric (§12 policy), T-010c one-evidence-short with the reason narrowed to
"which slot takes a helmet", as the coordinator recorded.

## 1. Owner found (raw chain, all IDA-verified this time)

- Name-table builder function: `0x564AA0` (SEH frame, loops the 10 dwords at 0x6FF650,
  string-construction via 0x401BD0/0x401B20; blocks end 0x564B88).
- Its ONLY code xref: `0x5650C5` inside **`JE::ModelInstance::SetSocketModel_564f90`**
  (size 0x5AE). ⇒ the 10-entry list `["", Head, Chest, UpperArms, Hands, Legs, Feet, Waist,
  ForeArms, Shins]` is **ModelInstance's socket-name table** (art attachment sockets — the
  family session 16 already pegged to updater-map children). The helper trio 0x564A50 /
  0x565610 / 0x565650 are just 36-byte record copy/appends used by the socket vector
  (record = {int, string(24B), int +0x20}).
- Session32's "plausible combat-UI/picker initializer" was WRONG (single-owner traced now,
  no ambiguity). Patched pointer in session32.

## 2. The alignment becomes a DERIVATION, not a tiebreak

Facts (all code-anchored):
1. Socket list order: `("", Head, Chest, UpperArms, Hands, Legs, Feet, Waist, ForeArms, Shins)`
   — exactly **9 anatomy names** + a null-socket entry.
2. Every consumer treats part **9 as UNSET** (preflight cancel @0x404983, zero-on-9 guards
   @0x41333d/@0x409fec, "Unknown CALLEDTARGET" strings).
3. Penalty table has 9 slots: values idx0..7 + **idx8 = FLT_MAX** (session 17).

Nine names in a fixed order + id 9 reserved as UNSET ⇒ ids 0..8 in list order
(drop the null-socket first, since a 0-based picker cannot skip index 0):

**0 HEAD, 1 CHEST, 2 UPPER_ARMS, 3 HANDS, 4 LEGS, 5 FEET, 6 WAIST, 7 FORE_ARMS, 8 SHINS,
9 UNSET.** (Under this enum socketIdx = partId + 1; the "" entry is socket 0 = no socket.)

Plausibility checks (all pass):
- Penalties: Head 10 (big silhouette), Chest/UpperArms/Hands/Legs 40, Feet 60, **Waist 120**
  (thin target), ForeArms 60, Shins → FLT_MAX path = never penalized ⇒ consistent with the
  DR switch's `default:` handling id 8 (logs "Unknown CALLEDTARGET", still computes —
  behavioural fingerprint: **called-shot at Shins makes the original emit that log line**;
  lockstep-diffable, and if observed, the enum is confirmed end-to-end).
- DR dual-row group {5,6} = {FEET, WAIST} = lower-body row (+0x7C8); main row = the six
  limb/torso cases 0,1,2,3,4,7. My session-6 "legs/torso" guess is dead; my session-32
  hedge "{Legs,Feet} under A" is dead too (A-alignment can't satisfy fact 2).
- 13-skill array, 15-derived, 9-part: the +0x7B0/+0x7C8 rows are 6-ints indexed by the
  ammo/weapon **DT type** 0..5 (unchanged from session 29 §3 / B's C-33 — orthogonal to this).

## 3. Residual risk (one line for FastArrow)

The bridge assumption is that called-shot ids and art-socket ids share this order (both
descend from the same GCRE anatomy list). It is the only mapping consistent with the three
code-fixed facts; if the Shins-log fingerprint (or any part-name picker screenshot in the
demo) contradicts it, revert to session32 §3's runtime recipe. Until contradicted: ship the
enum above.

## 4. Tooling hazard discovered mid-session (for everyone using the offline fallback)

The `/tmp/F3.bin` flat image + `--adjust-vma=0x400000` **drifts at high .text VAs**
(0x56xxxx region: my objdump-based call-scan returned `e8 d5 f3 ff ff` where IDA reads
`e8 25 f4 ff ff` — function boundaries and padding-based attribution from the flat copy
were off by ~0x34 there). Anything I pinned with objdump ONLY (no IDA cross-check) in high
.text deserves re-verification; everything from ≤0x46xxxx matched IDA byte-for-byte in
past sessions. IDA MCP is ground truth; the flat fallback is a degraded view. (Not a
reason to distrust earlier low-address raw citations — they were consistently
double-checked. But future 0x56xxxx+ offline work must be IDA-confirmed.)

## 5. Net

- T-010c: deliverable = §2 enum + §3 single fingerprint. FastArrow can close the row
  statically (verify via Shins-log in lockstep if desired).
- T-010b adjacent: socket-vs-slot vocab now firm — 0x6FF650 = ART SOCKETS (part layers),
  0x671Bxx = ITEM equip slots; don't conflate when checking +0x3B0.
- Session 16 (ModelInstance) gains: SetSocketModel_564f90 + socket vector record layout {int,
  string, int}×36B.

## 6. 10f half-2 DONE: `JE::World::virt_0x545820(id)` = SET FOCUS/CONTROLLED ENTITY

Decompiled (address EAs in-line). Semantics:
- Scans the entity-record list at **World+0xDC** (display `gap90[76]`) for a record whose
  `record+0xB4` == id (node walk via `meth_0x4c8eb0`);
- if the stored current at **World+0xD4** (`gap90[68]`) differs:
  1. `old = World+0xD4`; `v6 = host.vft[+84](old)` — resolve/claim controller for OLD
     (host = `global_cls_0x616328` block, the same singleton sub_41D050 walks — session 31 §1);
  2. `v9 = host.vft[+84](id)` — controller for NEW;
  3. write `World+0xD4 = id`;
  4. `v6->vft[+40](id)` / `v9->vft[+40](World+0xD4)` — hand focus to both controllers
     (old one receives the NEW id ⇒ "you are no longer focus" signal).
⇒ Engine packet 63 (SELF-assign) = **the controlled-entity handoff**; packet 58 tail calls it
with the map's entry id. For the spawn slice: `SetFocusEntity(id)` + ack-64 is the complete
client adoption; host vft+84/+40 slots are the two engine hooks to mirror.
Display-unit caveat honored: +0xDC/+0xD4 derived as gap90 member-base (0x90) + index — one
debugger read to make them iron if used for naming.

## 7. FINAL STATUS (post helmet-data read, session 35)
The last gap is closed by SHIPPED DATA, not more statics: `build/data/_ARM.grp` shows
PowerArmorHelmet & LeatherOutfit-HeaMotorcycle records carry `GITM.eqslot = 1` while all
suits carry 0 ⇒ row B = helmet (head-family) coverage ⇒ called-target **{5,6} = head-family
ids (HEAD+EYES, EYES naming inferred)**. Consequence stated plainly: my retracted socket-
display ordering is DEAD outright (the true enum cannot sit in list order if head is at 5/6)
— the DR grouping is armor-zone-structured, not anatomy-neighbourhood-structured. Session 35
owns the datum.
