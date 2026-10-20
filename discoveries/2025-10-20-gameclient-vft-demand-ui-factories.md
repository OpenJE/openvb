# 2025-10-20 — Session 28: F3::GameClient vft + demand UI factories (Dialogue/CSMenu) — corrects session-26 §5 guess

Target: ORIGINAL_REFERENCE (IDA + raw). Closes the session-26 "who spawns Dialogue/CSMenu"
question — NOT via GameInterfaces' late slots: the engine asks the CLIENT.

## 1. `F3::GameClient` vftable @0x664804 (56 slots)

Instance = `[0x70BD90]+0x2C` (session 27 §3; RTTI'd `JE::Client→F3::GameClient` at 0x43978a).
Slots split: 37 engine (JE::Client bodies 0x58/0x59xxxx) + **19 F3 overrides**:

| idx (byte off) | body | note |
|----|------|------|
| 0 (+0) | 0x45f2e0 | dtor chain (calls 0x45e6a0 + operator delete) |
| 1 (+4) | 0x4618f0 | F3 client init/tick hook (un-decoded) |
| 9,12 | 0x45f000 / 0x45e6b0 | small overrides |
| 22,23,24 | 0x409bb0 ×3 | SAME stub body for three slots (trivial const/void) |
| 28 (+0x70) | 0x461a10 | un-decoded |
| 30/31/33/37/44/45/51 | 0x45e7f0/820/830/880/8e0/940/9a0 | small accessor stubs (5–10 insn) |
| **43 (+0xAC)** | **0x4613d0** | **CreateDialogueInterface**: `alloc 0x2B0` → ctor 0x438080 name "GameDialogueInterface" (SEH-guarded; returns null on OOM) |
| **47 (+0xBC)** | **0x461470** | companion factory — `alloc` (0xDC8 per earlier scan = GameDialogueWindow 3528 B) → window ctor (un-decompiled tail; name TBD) |
| **48 (+0xC0)** | **0x461530** | **CreateCSMenuInterface**: alloc → ctor 0x435840 (class GameCSMenuInterface, RTTI size 0x1B15) |
| 50 (+0xC8) | 0x461930 | composite: calls 0x4615f0 then engine 0x58a4a0 (CSMenu *Label* composition — 0x43cd50-region class GameCSMenuLabel size 3705?) |

⇒ Engine flow: JE::Client machinery invokes slot 43/47/48/50 ON DEMAND (dialogue initiated /
character screen opened). The created interface registers through its own base-ctor into the
GUI visible stack (session 27 §2 SetActive) — the F3 GameInterfaces late slots (+0x14/2C/30/
34/3C) are still unattributed, but they are NOT the dialogue/CSMenu owners (this doc corrects
session 26 §4/§5 on that point).

## 2. Sizes confirmed vs RTTI DB
GameDialogueInterface 0x2B0 (alloc) vs DB size 889 (0x379 — DB includes vb tails? ⚠ flag:
allocated 0x2B0 < DB 0x379 means the DB size may be the *most-derived complete* incl. base
vbases; allocation for a `new` of that class must be ≥ — treat DB 889 with suspicion, the
0x2B0 + ctor 0x438080 is ground truth from the binary).
GameCSMenuInterface DB 6933 (0x1B15) vs alloc at 0x461540 = (pull exact push from dump:
6aff68…→ SEH; alloc size at 0x46154x — verify before porting sizes; likely 0x1B15 ✓).

## 3. Practical
- The engine asks THE CLIENT for UI objects (slots 43/47/48/50), not the interface manager —
  the reimpl's client/UI seam should mirror this split (JE::Client vft + F3 overrides).
- Dialogue action path (GameActionInitiateDialogue, 36 B) presumably ends up in the engine
  calling slot 43 — direct check when A ports dialogue initiation.
- Remaining GameInterfaces late-slot setters: still open (the get/set accessor pairs at
  0x439d3x/439e0x/… are the write surface — callers TBD via vft usage; lower priority now).

## 4. Evidence
GameClient vft 0x664804 (56 entries; F3-slot list §1). Dialogue factory 0x4613d0 (alloc
0x2B0 @0x4613ea, name@0x461410, ctor call 0x461446). CSMenu 0x461530 (SEH head, ctor
0x4615b1→0x435840). Slot-43 idx derivation: rdata scan back to table start 0x664804.
