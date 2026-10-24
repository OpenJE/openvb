# 2025-10-24 — the "0x34 AddAction payload class" PINNED: it is F3::GameActionDie (RTTI evidence)

Follow-up to the 616328 dual-role re-cut (same day): the retraction left the 0x34 AddAction
payload class "stays unnamed until its vft is pinned". This note pins it from shipped bytes —
and rewrites the mechanism one layer deeper.

## The misread stack (two layers retracted)

1. **`0x6165e0` = `___RTDynamicCast`**, the MSVC RTTI runtime helper. Its `push 34h` is the
   SEH prolog FRAME SIZE, not an allocation. The "heap object from 0x6165e0 (new 0x34)"
   reading was a misread of an RTTI call site. Its xrefs are the whole action family (every
   dynamic_cast in the binary).
2. **`0x616328` = the adjustor thunk `___RTDynamicCast` itself CALLS** (at 0x616601) to adjust
   `this` — already split by the re-cut (thunk, not ctor).

## The real mechanism at the Apply tail (sub_45C140, window 0x45c5e2..0x45c626)

```
mov  eax, [ecx]              ; world-server singleton chain [0x70BD98]->[+8]->[+4]+8
push ebx
call dword ptr [eax+14h]     ; worldServer->vft[+0x14](ent)  ->  the entity's Action*
push ebx / offset ??_R0?AUGameActionDie@@ / offset ??_R0?AUAction@@ / ebx / edi
call ___RTDynamicCast        ; Action*  ->  GameActionDie*  (srcVftDelta=0, flags=0)
mov  [eax+1Ch], ecx          ; <- this->mbr_0xA0
mov  [eax+20h], edx          ; <- this->mbr_0xA4
mov  [eax+28h], cl           ; <- a flag byte from the effect
push edi
lea  ecx, [eax+esi+4]
call net_critical_section::meth_0x550720   ; the enqueue (AddAction-family call @0x45c626)
```

**No allocation. No "new 0x34".** The code obtains the victim's EXISTING die action via
`worldServer->vft[+0x14]`, `dynamic_cast`s it `Action*` → `GameActionDie*`, writes the pending
damage fields `{+0x1C damage?, +0x20, +0x28 flag}` from the effect into it, and enqueues it
through `0x550720`. The shipped `??_R0?AUGameActionDie@@` / `??_R0?AUAction@@` TypeDescriptor
literals ARE the pin: **the payload class = F3::GameActionDie** — whose vft (0x648694) and
methods (ctor_0x404e50, virt_meth_0x404e90/0x405020/0x405040) are already hosted in the IDB.
The "0x34 stays unnamed" caveat RETIRES.

## Consequences

- The victim-side HP/state message handler (tohit-apply-tail §3.0's "one log line names it") is
  the victim's GameActionDie action itself — the message IS the die action.
- The wire-id==message-id unification stays UNVERIFIED (the retraction's law holds — that claim
  was about net::packet and remains unsupported).
- Port-side: the P-CMBT PendingEffect/kDamage inbox model remains the reimplementation's shape;
  this note only corrects the SHIPPED-mechanism reading, no port code changes.
- IDB: comment landed at the Apply tail (sub_45C140); no renames needed (the class was already
  correctly hosted under GameActionDie).
