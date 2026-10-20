# 2025-10-20 — World::PackState dissected: entity list global + prototype map identified (session 15)

Target: ORIGINAL_REFERENCE. Read-only. Names 2 of the 4 JE::World containers from session 14 §2
and pins the per-entity serialization surface. Trigger: reader scan for World field offsets.

## 1. `JE::World::PackState_545930(net::packet*)` — the world snapshot writer

Guards first: `if (world+0x34 != 3) FatalError("PackState() may only be called on a normal
world.")` — so **+0x34 is also read as a world-mode** (see §4 caveat). Payload order:

```
A) 8 × int32 world metadata: World+0x44, +0x48, +0x6C, +0x70, +0x74, +0x7C(id-base!), +0x80, +0x84
   (note: +0x7C = the &0x1FFFFFFF "set id base" sink from session 14 §4 — packed here)
B) live entity list, via GLOBAL sentinel `byte_70BE0C` (= F3::global_gui_interface_0x70BE08+4):
   node+0x08 → entity ptr; per entity pack:
     int32 [entity+476]     = 0x1DC  → net id (session 9's join-assignment slot CONFIRMED)
     int32 [entity+348]     = 0x15C  → classId tag (session 4/11) CONFIRMED in serialization
     entity->vft(0x54a610-thunk)::meth(packet)   ; per-entity virtual serialize
   terminator: int32 0
C) registry at World+0xE4 — **CORRECTION (session 16)**: this is the LOADED-MAPS registry
   (records are JE::Map*; their serializer throws "normal maps"), not prototypes.
   Per map: int32 [rec+0xB4] (map id) + rec->PackState(packet). See
   [entity-map-packstate-wire-order](2025-10-20-entity-map-packstate-wire-order.md) §1.
D) tail: int32 [World + 0x158+...] via `*&this->gap90[76]` (=+0x13C?)  *(offset via IDA member
   typing — treat as "trailing world field")*
```

Consequences:
- **The live entity registry is NOT a World tree** — it's the GUI::Interface-style global
  intrusive list at `0x70BE08/70BE0C` (same list the server warp case-0x46 iterated, where
  node entries matched client entities). World participates via that global; openje's
  "GuiInterface" already models this shape.
- **World+0xE4 map = loaded-maps registry** (mapId → JE::Map*) — see correction above;
  the prototype/template lookup pipeline (vft+48 by-name) lives elsewhere (World trees
  @+0x2C/+0x13C/+0x14C candidates, session 16 §5).
- Per-entity on-wire identity triple: **netId@+0x1DC, classId@+0x15C, +virtual-serialize**.
  Matches session 9 join path and session 11 operate dispatch — the whole identity story is
  now consistent across four sessions.

## 2. `cls_0x4d8d70::meth_0x546250` (reader-scan hit at +0x2C region) = STL map erase

Not a World registry op — it's generic `_Tree::_Erase_node` machinery (`"invalid map/set<T>
iterator"` throw, red-black rebalancing, key-string free at node+12..). Recorded so the
reader-count on +0x2C isn't mistaken for semantics: the STL helper family (0x546xxx,
0x548xxx) touches *whichever* container is passed; attribution must use call sites, not
instruction scans.

## 3. World dtor corroborates session 14 layout

`cls_0x4d8d70::meth_0x5483a0` (World dtor): clears vftptrs back to base tables, nulls
`global_cls_0x616328_0x70bd98`, then frees the containers in order map@+0x158 → tree@+0x14C
→ tree@+0x13C → subobj@+0x120(5 dwords block) → cls_0x56ae70@+0x108 → cls_0x569f10@+0xE8-ish
pointer chain → map@+0xE4(head+count) → subobjects @+0xB8,+0x90 → tree@+0x2C. All container
heads from session 14 §2 confirmed as real owning members (freed exactly once each).

## 4. +0x34: sharpened ambiguity (raw evidence added same session)

- PackState prologue (raw): `mov ebp,ecx` + `cmp DWORD PTR [ebp+0x34],0x3` — NO view shift;
  the mode check is exactly World-base +0x34.
- World ctor (raw 0x548b0d/0x548b10): after `call 0x56e0b0` (InitSentinel), writes
  `[ebx+4]` and `[ebx+8]` with ebx set pre-window — consistent with ebx = this+0x2C ⇒
  sentinel@+0x2C, head@+0x30, count@+0x34.
- So the same dword is read as "tree count" (ctor) and as "== 3 = normal world" (PackState).
  Full-.text grep found **no `[reg+0x34],3` store** — the mode value is set via computed
  addressing or written once through a differently-typed pointer. Two hypotheses:
  (a) tree #1's "count" slot is actually a world-mode field (IDA tree attribution from the
  ctor's InitSentinel call is right; the count-vs-mode reading is just dual-use); or
  (b) PackState's check is against a count ("normal world = exactly 3 entries"?) — weak.
  ⚠ unresolved — see RESPONSES note: the reimpl's live-pump can OBSERVE this field at runtime.

## 5. Independent corroboration: ammo registry

This agent independently decoded `GameWorld::meth_0x470220` (memset +0x164 ×0x230; 20×28B
string table 0x6F7E14..0x6F8044; new GameAmmo(0x7B0) each; `meth_0x551880` init; store via
`7*caliber+modifier` index) — matches the reimpl agent's
`2025-10-20-session9-ammo-prototype-registry.md` in full. Their doc adds the lookup bounds
(`caliber ≤ 0x14 && modifier < 8` via 0x46fac0) and the id-storage detail; both decodes agree,
considered CONFIRMED by two independent passes. GameWorld own region = +0x164..+0x393 (0x230)
— which also completes session 6 §1's GameWorld extent question: World prefix 0x164 + own 0x230
= 0x394, then vb slots.

## 6. Queue update

1. Resolve §4 view ambiguity (2-min raw check next round).
2. Entity virtual-serialize target (0x54a610 thunk → Entity vft slot?) + record serialize
   0x56d7f0 → gives the SAVE FILE per-entity/per-template field order (pairs with GENT).
3. Name trees @+0x2C, +0x13C, +0x14C via call-site attribution (find `lea ecx,[world+...]`
   callers in F3 methods).
4. Older queue: clip-name table, damage Apply math, CActionbarInterface fields.
