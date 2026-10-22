# P-CMBT Step 2 feeder evidence — what the damage path actually does at the push boundary

Read-only RE pass by NiceTiger (verifier seat), 2026-10-22, against `artifacts/F3.exe.i64`
(ORIGINAL_REFERENCE, ida_original_f3, py_eval + decompile; no IDB writes). Every claim below is a
`hex:instruction` quote from the artifact; instruction text is the authority — where the decompiler
disagreed with bytes, bytes are what is written.

## THE HEADLINE, STATED LOUDLY

**GameEffectDamage NEVER queues.** Its slot-10 wrapper sets the ready byte (`[effect+0xB] = 1`) at
`0x45c6b3` *unconditionally, before calling anything*, and the push site frees the effect in the same
call. The target's inbox list (`target+0x2E0`) is structurally unreachable for combat damage. So P-CMBT
Step 1's container is exercised **only by test traffic** for damage — Step 2 must model the SYNCHRONOUS
side effects of the push, not an inbox drain. If Step 2's plan says "route damage through the container",
the plan is wrong about shipped: shipped routes damage through the pusher's own stack frame.

## 0. Correction to the design-doc citation: apply is [vft+0x28], not [vft+0x2C]

`GameEffectDamage` vftable @`0x66073c`, all 16 cells read raw (`dd`):

```
+00 0x45c090  +04 0x45c0a0  +08 0x45c0f0  +0C 0x45c120
+10 0x4625c0  +14 0x462600  +18 0x4625e0  +1C 0x408d30
+20 0x45c100  +24 0x404b70  +28 0x45c6b0  +2C 0x404b70   <-- +2C is an EMPTY retn
+30 0x4626a0  +34 0x408d30  +38 0x462710  +3C 0x462740
```

- Byte `+0x2C` (slot 11) = `0x404b70` = `retn` — a do-nothing virtual, NOT apply. (Slot 9, `+0x24`, is
  the same empty stub.)
- Byte `+0x28` (slot 10) = `0x45c6b0`, and `sub_45C140` has exactly ONE code ref in the image:
  `0x45c6c5: jmp sub_45C140` — a tail-jump from inside slot 10.
- The doc's "+0x2C = 0x45C140" is the cells-vs-bytes ambiguity biting the slot-number citation: slot
  10 counted from 1, or +0x28 + 4 miscount. The lane's own catalogued class, arriving on schedule.

`0x45c140` extent read WHOLE: `0x45c140-0x45c667` (0x528 bytes, ~330 instructions; decompiler returns
null — SEH + cookie + x87 + two jumptables; everything below is raw disassembly).

## 1. The slot-10 wrapper: ready-first, gate, tail-jmp into apply

```
45c6b0: push    esi
45c6b1: mov     esi, ecx
45c6b3: mov     byte ptr [esi+0Bh], 1      ; READY BYTE SET FIRST - unconditional
45c6b7: call    GameEffect::virt_meth_0x462620
45c6bc: test    al, al
45c6be: jnz     short loc_45C6C2
45c6c0: pop     esi
45c6c1: retn                               ; gate 0 -> apply SKIPPED, returns 0
45c6c2: mov     ecx, esi
45c6c4: pop     esi
45c6c5: jmp     sub_45C140                 ; tail-jmp: APPLY runs, al = apply's al
```

The gate `0x462620` (whole extent read `0x462620-0x46269a`) does NOT normally return 0 — it:
checks the world-server flag (`0x46262c call AreaMapIcon::Get_mbr_0x4_0x5b4f30` @`0x462631 test al,1`;
flag clear -> `_CxxThrowException` with `"Invalid attempt to retrieve world server instance."` —
the same shipped throw string the feeder itself uses @0x4039b0); computes the singleton-adjusted
server object (guarded SELECTION at `0x462659-62`: `test eax,eax / jz / xor eax,eax` — then
`0x462664: mov ecx,[eax+4]` DEREFERENCES it anyway: the B8 "guarded selection, unguarded use" family,
live in the damage path too); reads `[server slot29] + [target-adjusted +0x2A0]` and stores the base
into `[effect+0x90]` (`0x46268d`); returns `mov al,1` (`0x462694`). So for damage the gate is a
setup-with-escape-hatch, and its normal result is TRUE. The only non-apply outcomes are the throw
(SEH propagates) and the never-taken `al==0` return.

## 2. The push site — why damage never reaches the +0x2E0 inbox

`JE::Entity::meth_0x5506c0(target, effect)` read whole (`0x5506c0-0x550713`):

```
5506c8: mov     eax, [edi+1DCh]        ; target's SCRIPT HANDLE (+0x1DC, the slot-9 field)
5506d1: call    meth_0x55a3a0          ; [effect+0x14] = eax   (bind, recurses +0x1C children)
5506d8: call    meth_0x55a460          ; [+8] mark via [vft+0x20]; FALSE pred -> sets [eff+0xB]=1 (0x55a4a2)
5506df: call    meth_0x55a4b0          ; [+9] mark via [vft+0x24]
5506e6: call    GameEffect::meth_0x55ae10   ; THE FIRE: fresh effect -> 55ae96 call [vft+28h] -> slot10 -> APPLY
5506ef: call    dword ptr [edx]        ; slot 0 = 0x45c090: mov al,[ecx+0Bh]; retn  (READY PREDICATE)
5506f1: test    al, al
5506f3: jz      short loc_550701
5506f7: call    meth_0x55a570          ; READY PATH: unlink + [vft+0xC](eff,1) = deleting dtor 0x45c120 -> FREED
...
55070c: call    cls_0x54e070::meth_0x54ea00(&[target+0x2E0], &effect)   ; ENQUEUE - needs al==0
550713: call    Entity::meth_0x54ef30(target)                          ; wake
```

For `GameEffectDamage`, `0x55ae10` reaches `[vft+28h]` (fresh: `[eff+0xA]==0` test `0x55ae42`, no
children `[eff+0x24]==0` test `0x55ae71`) and the slot-10 wrapper has already written `1` to `+0xB`
at `0x45c6b3`. So the `0x5506ef` predicate ALWAYS returns ready -> delete-and-return. The enqueue
branch at `0x550701` is **unreachable for damage**. Extent statement per item 73: this is the whole of
`0x5506c0-0x550713` (20 instructions) and the whole of `0x55ae10` fire-walk decision path for the
childless case; damage cannot leave `+0xB` at 0 by any path inside those extents — the ONLY writer
reachable from slot-10 is `0x45c6b3`, and it writes 1.

Consequence worth naming: the push OWNS the effect. After `meth_0x5506c0` returns, the damage effect
object is freed (`0x45c120` deleting dtor, `free` when flag&1). Step 2's port cannot keep pointers to
it post-push, because shipped doesn't either.

## 3. Q1 — one effect object per HIT, created inside the per-hit loop

`GameActionAttack::meth_0x403770` read whole (`0x403775-0x4039a0`). The inner loop over the attack's
hit list (`v8` walking `*v3->gap11`, `0x40381e..0x403910`) contains BOTH the factory call and the push:

```
403851: mov     eax, [v9 server vft]; call [eax+40](server, 1)   ; FACTORY: create effect, type 1
40385b: call ___RTDynamicCast(Effect -> GameEffectDamage)
403866: call meth_0x55a360          ; [effect+0x10] = source handle
403870/403880/40388d/40389a/4038a5: call meth_0x55a3e0(idx, val) ; param window [eff+0x28+4*idx], idx 1,2,0,3
4038ae: call meth_0x55a420(1)       ; [eff+0x38] = 1 (propagates to +0x1C children)
4038b3: mov edx,[esi+8]  / 4038b6: mov [ebp+9Ch], edx   ; DAMAGE CELL - copied, not computed
4038bf: mov ecx,[esi+0Ch]/ 4038c2: mov [ebp+0A0h], ecx  ; TYPE
4038c8: mov [ebp+0A4h], eax                              ; MATERIAL (eax = [esi+10h])
4038ce: mov edx,[esi+14h]/ 4038d1: mov [ebp+0A8h], edx  ; SOUND CLASS
4038e0: cmp esi,eax (list head) ; first-hit test
4038e8: mov [ebp+0ACh], al                              ; +0xAC = multi-hit flag (pre-scan 0x4037e8-804: 1 iff >1 flagged nodes)
4038ee: mov byte ptr [ebp+0ADh], 0                      ; first in batch
4038f7: mov byte ptr [ebp+0ADh], 1                      ; NOT first in batch
403906: call JE::Entity::meth_0x5506c0(local_10, v10)   ; PUSH to target
```

- **One `GameEffectDamage` per hit-list entry**, freshly created each iteration — so a multi-hit attack
  creates N objects and pushes N times to the SAME resolved target (`local_10 = 0x54be10(handle)` @0x40379e).
- The +39/40/41/42 the decompiler printed for the +0x9C..+0xA8 stores is a char*-arithmetic artifact;
  the RAW bytes (`mov [ebp+9Ch]` etc.) are quoted above. Rule 1 again: cite instruction text.
- The feeder **carries** numbers; it does not roll them. Damage/type/material/sound are copied from the
  hit entry `[v8+8/0xC/0x10/0x14]`. Roll-sites are upstream of 0x403770 and out of scope here.
- The batch marks are stamped HERE: `+0xAC` (multi-hit) and `+0xAD` (non-first), read later in apply.

## 4. Q3 — the target-side touch list of apply (what Step 2 must reproduce), in execution order

All from the whole read of `0x45c140-0x45c667`. "dead-predicate" = `[vft+0x50]` (slot 20) on the
target-adjusted subobject, called at three sites with a consistent reading of "already dead/zero":

1. `0x45c180: call [eax+50h]` — TRUE at entry -> `xor al,al; jmp exit` (no damage applied to the dead;
   apply returns 0). `0x45c190: mov ebp,[esi+9Ch]` — damage==0 -> return 0 (`0x45c19c`) WITHOUT the HP write.
2. HP WRITE: `0x45c1a3: mov ecx,[esi+48h]` (target); `0x45c1a6: mov eax,[ecx+24h]` (cur);
   `0x45c1a9: sub eax,ebp`; `0x45c1ac: call meth_0x4629c0` — clamp setter read whole
   (`0x4629c0-0x4629d1`): stores NEW VALUE UNCONDITIONALLY to `[tgt+0x24]`, caps at `[tgt+0x20]` ONLY
   ABOVE (`jle` to max branch); **no lower clamp — cur HP CAN GO NEGATIVE** (the lane's known clamp
   hole, now with its byte-level proof for the damage writer specifically).
3. DIRECTION: if `[eff+0x10]` (source handle) non-null and resolves (`0x45c1c1: call meth_0x54be10`),
   x87 block `0x45c1d4-0x45c2f...` builds target-minus-source position deltas, `fmul flt_649C50` (deg
   scale), `0x45c29c: fpatan`, quadrant compares against `flt_660820/824/828` + `lpSrcStr_6cb56c`
   -> `var_B1 = 0/1` (backhit flag) — the stack residue TrueRaven named; it is a LOCAL, computed, then
   consumed below. If no source: `var_B1` stays 0 (`0x45c1b6`).
4. REACTION ANIM: only if dead-predicate FALSE at `0x45c2f3` (so a LETHAL hit skips its own reaction
   anim) AND `[eff+0xAD]==0` at `0x45c30c` (only the FIRST hit of a batch reacts - batch marks earn
   their keep). Anim id: `[tgt-adj+0x3A6]!=0` -> `0x21+backhit` (33/34); `+0xAC!=0` -> `0x26 + (backhit?3:0)`
   (38/41); else -> `0x25 + (backhit?3:0)` (37/40). Pushed via `0x45c35a: call GameEntity::QueueAnimEvent_4673d0(id, 0)`.
   Then `0x45c35f: mov byte ptr [esi+0Bh],1` (belt-and-suspenders ready set).
5. IMPACT VEG: switch `[eff+0xA0]` (6 cases) at `0x45c37a` selects a .veg name string; case 0 is special:
   `[eff+0xA4]==5` -> `FX_HitBloodHighVelocity_01.veg`, damage!=0 -> `FX_HitBloodStandard...`,
   damage==0 -> `FX_HitRicochetMetal_01.veg` (a graze hits metal, @0x45c3a6); case 2 electric; case 5
   sub-switches material -> laser/fire. `0x45c420: call net_critical_section::meth_0x5512b0(tgt, name, 0, 1)` attaches.
6. IMPACT SOUND: switch `[eff+0xA8]` (8 cases, `0x45c42f`) rolls `sub_48CD00(1.0,0)`, maps to ids
   (4/9, 0xB, 0x10, 0x14, 0x17, 0x23, 0x26...), plays via `0x45c4ff: cls_0x48dc90::meth_0x48dc90([edi+164h], id)`.
7. SCRIPT HOOK: `0x45c572: push eax; 0x45c56d: push offset "OnDamage"; ... 0x45c575: call sub_55A220` —
   and `0x55a220` repacks 5 args and tail-calls `JE::Entity::RunScript_54afe0`. This is the target's
   named script entrypoint, invoked SYNCHRONOUSLY inside apply, with a 4-cell arg block
   `{0x90-base+slot29 value, source handle, damage amount, server object}`. Another shipped
   script-by-name witness for gap 3, on the combat side.
8. BROADCAST: `0x45c59c: call GameServer::meth_0x46d020([map_icons+18h], [tgt-adj+0x1E0], damage, type)` —
   server/network side of the hit (packet body not read here; extent statement: NOT part of this pass).
9. DEATH ACTION: recheck dead-predicate `0x45c5b2: call [eax+50h]`; if NOT dead -> `jz 0x45c62b` (skip).
   If dead: `0x45c5c6: Entity::meth_0x54c440` -> singleton-adjusted `0x45c5e5: call [eax+14h]` (factory
   slot 5) -> `0x45c5f7: ___RTDynamicCast(Action -> GameActionDie)` -> fills `[die+0x1C]=type,
   [die+0x20]=material, [die+0x28]=byte` -> `0x45c626: call net_critical_section::meth_0x550720(target-adj, die)`
   — **the same named seam 0x550720 = T-030 EnqueueEntityAction**. So a kill does enqueue exactly one
   action (GameActionDie) into the target's queue; damage itself never does. The inbox that P-CMBT
   Step 1 models IS reached by combat traffic — via the death action, not the damage effect.
10. Return: `0x45c640: mov al,1` full path; `0x45c642/0x45c644` cleanup, SEH unwind, `jmp __security_check_cookie`.

## 5. What this makes observable vs deferred for Step 2

- **Inbox-boundary (what a container would see for damage): nothing.** The effect never enters `+0x2E0`.
  Fields above are consumed synchronously during the pusher's call.
- **Externally visible, synchronously at push:** `[tgt+0x24]` clamp-store (negative-capable), anim-event
  queue (first-hit + survive + stance-gated ids 0x21/0x22/0x25/0x26/0x28/0x29), VEG attach, sound id,
  `OnDamage` script invoke, `0x46d020` broadcast, and on lethality: a `GameActionDie` **enqueued through
  the proven seam 0x550720**.
- **Internal to the effect (no port modeling needed unless the queue is kept for OTHER effect classes):**
  the `+0xA/+0x8/+0x9/+0xB` control-byte walk, `+0x1C` children recursion, `+0x90` clock base store,
  the `55ae10` RbTree gather, deleting-dtor-at-push.
- **Deferred, named:** the 0x46d020 packet body (network era); the hit-entry roll site upstream of
  0x403770 (combat RNG era — CombatRngSeam doc owns it); the `+0x3A6` stance-field writer.

## 6. Lock-step cautions for whoever replays this in the debugger

- apply carries SEH (`0x45c140-0x45c14e` frame setup), a security cookie (`0x45c15b/0x45c667`), and two
  jumptables (`0x45c38a`, `0x45c438`) — step counts through it will not match a naive reimplementation.
- The push path can THROW (`0x46264f`, and the feeder's own `0x4039b8` pre-check). A port with no
  world-server flag concept cannot enter that state — declare it, per the port's standing rule on
  unreproducible shipped faults.
- The damage path REUSES the null singleton the B8 law covers: `0x46265d-64` is guarded selection +
  unguarded use, so "null 0x70bd98 during combat apply" faults too, same family as the trio.

## 7. WITNESS #2 for gap 3 — the OnDamage sync-invoke chain, decomposed

TrueRaven asked for the OnDamage leg as a second script-by-name witness; here it is end to end, raw bytes.

**Call site (inside apply):**

```
45c561: lea  edx,[esp+var_70]     ; &out-ish cell
45c565: push edx
45c566: push 4                    ; arg count
45c56c: push ecx                  ; &arg block {eff+0x90 base, src handle, damage, server obj}
45c56d: push offset "OnDamage"    ; NAME
45c572: push eax                  ; adjusted target subobject
45c573: mov  ecx, esi             ; this = effect
45c575: call sub_55A220           ; 5-arg reorder thunk
```

`0x55a220` (read whole, 9 instructions) is a pure argument reorder: it calls
`JE::Entity::RunScript_54afe0(this = adjusted target, "OnDamage", 4, &block, &out)`.

**RunScript_54afe0 (read whole `0x54afe0-0x54b047`):**

```
54aff3: server-flag test -> 54b00f: _CxxThrowException("Entity::RunScript() method may only be called by the server.")
54b016: call Entity::GetScript_54aef0
54b01b: test eax, eax / jz -> return 0            ; no script => SILENT 0, no message
54b02c: mov  ecx,[esi+2BCh]                        ; the cached SCRIPT OBJECT, Entity+0x2BC
54b032: mov  edx,[ecx]                             ; its vft
54b03f..43: push name/count/block/out             ; FOUR args forwarded verbatim
54b044: call dword ptr [edx+14h]                   ; SCRIPT VFT SLOT 5 (byte +20)
```

**GetScript_54aef0 (read whole `0x54aef0-0x54afd7`)** adds the resolver law the typedef decision needs:
- `[esi+0x2C0]` = load-failure latch (checked first, cached-miss returns 0 fast);
- name string is an SSO at `Entity+0x2A4` (capacity field `+0x2B8`, the `10h` test at `0x54af41`);
- the script is fetched by SERVER OBJECT: `0x54af28 call GetWorldServerInstance_0x401150` then
  `0x54af38: call [server_vft+24]` with arg `[entity+0x1DCh]` — the slot-9 SCRIPT HANDLE — stored to
  `+0x2BC` at `0x54af3b`; so the script is resolved BY HANDLE from the same singleton the natives trail
  (B8 family) and cached on the entity;
- load failure logs `"Entity::GetScript() Unable to load script..."` via `LogDebugString_0x497120`
  (another shipped literal for T-022), latches `+0x2C0=1`, nulls `+0x2BC`, returns 0.

**What this adds over the case-0x51 witness (`0x46d...`, predicate `[script+16]` then invoke
`[script+24]` with out):** the OnDamage path uses slot **5 (+0x14)** with a COUNTED arg block and NO
existence predicate — the two shipped call-sites of the script-by-name interface therefore exercise
THREE distinct slots (+16 exists?, +20 fire-and-forget, +24 call-with-out) of ONE object shape:
`(script resolved by handle from server singleton) -> vft(name, argc, cells[, out])`. Shipped treats
entity scripts as named-entrypoint objects in exactly one consistent ABI, used by both the net dispatch
and the combat apply path, synchronously, server-only (both entry paths throw on the same world-server
flag — `0x54aff3` here, the recv-patch era there).

Absence bound per item 73: within `0x54afe0-0x54b047` there is NO test of any script-result value (the
return of `[vft+14]` is passed straight out as RunScript's al — apply ignores it entirely @0x45c57a
reloads `[esi+48h]` next), so the OnDamage invoke is fire-and-forget by construction, verified by the
absence of any consumer of eax between `0x54b044` and the `retn`.

Port-facing consequence for the typedef call: `NativeScriptFn = int(*)(Mover*, a2,a3,a4)` matches NEITHER
side of either witness — shipped has (a) natives as `__cdecl(amx, params)` (45-entry table) and (b) script
entrypoints as `vft(name, argc, cells[, out])` on a handle-resolved script object. A port modeling script
execution must pick (b)'s shape; the OnDamage chain is the combat-side proof that (b) is live code, not
just a network-packet curiosity. Script-vft exact class identity and slot 0/+4/+0Ch handler semantics
(used by GetScript's load path) are named-deferred — not needed for the witness.
