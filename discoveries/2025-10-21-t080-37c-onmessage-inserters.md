# T-080 — Entity+0x378/+0x37C "OnMessage" surface: the list is a 3-allocation script-mailbox; its inserters are the three SendMessage natives + the engine ActionMessage(kind 7) start-fan-out; the sentinel is a heap-allocated end-marker

**Agent:** WildZenith (sessionId `01a16092-2ef1-703e-b403-9e417653ef5f`) · **Date:** 2026-10-21 · **Row:** T-080 (claim `f28a465`) · **Method:** read-only `ida_original_f3` + native f3_* + objdump raw over the md5-pinned exe (`a68c22f3dd1c256ec9539a70e87cb434`). `tools/ecx-flow.py selftest` re-gated immediately before writing verdicts — now **6/6 PASS** (tool gained T5, the item-44 share-check gate; all green). Zero IDB writes. Warm start: T-074 §5 row-4 + §7 tail; this doc resolves that named-unresolved tail.

## VERDICT (front)

The +0x37C surface T-074 mapped as "pop only, inserters named-unresolved" is now **census-complete**:

1. **The sentinel's meaning is settled by its allocator: a HEAP-ALLOCATED SELF-LINKED END-MARKER** — neither a type-tag nor an owner-handle. The Entity ctor @0x5516d9-0x551706 calls the shared helper `0x56e0b0` (`new(0xC)` @0x56e0b0; `mov %eax,(%eax); mov %eax,0x4(%eax)` — raw @0x56e0be-0x56e0c7: block[0]=block, block[4]=block, block[8] untouched), stores the block into `[entity+0x37C]`, and zeroes `[entity+0x380]` (the size). **`[entity+0x378]` — the third slot of the 12-byte header triple — is NEVER written or read anywhere in the image** (raw sweep; also absent from the frozen export's Entity member census, which skips 0x374→0x37c and 0x380→0x388). Empty test = `*sentinel == sentinel` (classic `_Myhead == head`), confirmed at pop @0x54c653 and discard @0x54b407.
2. **Inserters: exactly 5 call sites through exactly 1 primitive** — `sub_54EE60` (0x54ee60-0x54eeeb, `this`=target Entity). IDA xref list complete (6 xrefs = 5 real + 1 tail-jmp thunk @0x54ee5f): the AMX natives `JE::SendMessage` @**0x5c5503**, `JE::SendMessageToTeam` @**0x5c5908**, `JE::SendMessageToSquad` @**0x5c5a7a** (direct push after handle→entity resolution), and `JE::ActionMessage::virt_meth_0x5a9080` @**0x5a9107/0x5a91a7/0x5a9214** (the engine action's **start-slot fan-out** to team / squad / individual member lists). The pop path T-074 mapped is the ONLY consumer; nothing else in the image ever reads the list.
3. **The Message action is engine kind 7** — both factory jump tables read raw (below), which independently confirms T-070's "case 6/7 = fall-through to JE::World::CreateAction" and the kind numbering now feeding T-071's alignment.

## 1. Structure (all raw-pinned)

| slot | role | evidence |
|---|---|---|
| Entity+0x378 | container-handle address passed to push/free helpers; **value never accessed** | lea-only: @0x5516d9 (ctor), @0x54eec0 (push), @0x55026b (destroy); no load/store anywhere |
| Entity+0x37C | pointer to 12-byte heap **end-marker block** (`0x56e0b0`: {next=self, prev=self, spare}) | ctor store @0x551703; pop loads @0x54c643/53/7b; push loads @0x54eeb4; discard @0x54b3f3/ff; destroy frees block + stores 0 @0x550290/95 |
| Entity+0x380 | live node count | ctor 0 @0x551706; `++` via `sub_54DBA0(this=&entity+0x378,1)` @0x54eeda (SEH-checked size adjust); `--` at pop @0x54c698-0x54c6a2, discard @0x54b421-0x54b430, free-loop @0x4b9d91-0x4b9d98; destroy zeroes @0x550298 |
| Entity+0x384 | in-flight record guard (NOT a queue member) | write @0x54c65e, read @0x54c6a8, zero @0x54c6c4 — pop-path only (see §3 re-entrancy) |

The pop's `test %eax,%eax` @0x54c649 is meaningful only post-destroy (the zeroing at @0x550295) — the block is always allocated between ctor and destroy. Item 8c honored: allocation site reached and typed (helper `0x56e0b0` shared with the sibling list at +0x2F4, ctor @0x5515b6-0x5515c8 — same shape).

## 2. Inserter anatomy — `sub_54EE60(targetEntity, from, msgId, payloadPtr, nDwords)`

Per message it creates **three allocations** (raw @0x54ee60-0x54eeb2):
- **record** = `new(8)` @0x54ee68: `{+0: nDwords+2 (size bookkeeping) @0x54ee84, +4: data*}`
- **data** = `new(4*(n+2))` @0x54ee8e (`0x605554` wrapper): `{+0: from, +4: msgId, +8..: n dwords copied from payloadPtr via rep movsd @0x54eeaf-0x54eeb2}`
- **node** = 12B `{next, prev, record*}` via `sub_433760(this=&hdr+0x378, sentinel, sentinel._Prev, &record)` @0x54eecf + tail splice `*sentinel._Prev=node; node→...` @0x54eedf-0x54eee7, then size++ @0x54eeda.

**Feeder contexts (all EA-anchored):**

(a) **`JE::SendMessage`** @0x5c5470-0x5c5533 — AMX native (arg-parse `sub_59AF90(…,2,"SendMessage",…)` @0x5c5435). Retrieves the sender's FROM id via `sub_5CCCC0(…,1179407181,…)` @0x5c5442 (0x464D4F52 = FOURCC **FROM**, LE); verbatim error strings banked: `"SendMessage() was unable to retrieve the 'from' id."` @0x5c5454, `"…was unable to find the requested entity."` @0x5c5482, `"…was unable to resolve address of parameter %d."` @0x5c5528. Target entity via `sub_59AEF0`, payload dwords resolved one-by-one via `sub_5CE350` @0x54ed3 loop, then push @0x5c54f1 + immediate `j_j__free` of the temp buffer @0x5c5509 (the push deep-copies — record/data/node own their bytes). **Direct push — no action, no deque hop.** `SendMessageToTeam` @0x5c5908 / `SendMessageToSquad` @0x5c5a7a: same primitive, broadcast member resolution.

(b) **`JE::ActionMessage` (engine kind 7)** — the two-stage path. Construction: sole `new(0x34)`+`ctor_0x5a8ff0` site = `JE::World::CreateAction` case 7 chunk @0x544fb7/0x544fd7; the F3 factory `GameWorld::CreateGameAction_0x46f290` reaches it on its **default** path @0x46f4e3 (`add $0xfffffdd0,%ecx` = the -0x230 **this**-adjust only — the kind value passes through in eax UNMODIFIED, so F3-side and engine-side numbering are IDENTICAL). Jump table @0x5450cc read raw: **0=Die(0x1C), 1=Idle(0x1C), 2=MoveTo(0x48), 3=PickUp(0x30), 4=Wander(0x44), 5=Wait(0x24), 6=Anim(0x28), 7=Message(0x34), 8=FATAL("Unknown action sent to World::CreateAction() factory." @0x5450ae), 9=Equip(0x1C), 10=Unequip(0x1C), 11=SetActiveWeapon(0x1C)** — T-070's "6/7 = engine fall-through" now pinned by both tables (and 8 is the engine-side gap the F3 table fills with FloatingSpeechText).
  vft @0x6a5f34 raw-dumped = **standard T-070 §1 action protocol**: slot0 dtor 0x405350, +0x04 isFinished 0x404b60 (shared getter), +0x08 cancel 0x404b70, +0x0C queued-getter 0x4010d0, **+0x10 START = 0x5a9080**, +0x14 update = 0x5a9070, +0x18 notify = 0x404bb0.
  **Start** (runs when walk #2 first touches it, `sub_55A1C0` protocol runner): selector cascade — team `[+0x24]` → worldsub vft+132 + `GetTeamMemberList_56a2f0`, push to each member @0x5a9107 (log `"ActionMessage was unable to retrieve a team entity."` @0x5a90e0→0x6a5fc4); squad → worldsub vft+136 + `GetSquadMemberList_569390` @0x5a91a7; individual `[+0x1C]` via `0x54be10` @0x5a9214 (each member handle resolved through the same `0x54be10` busy-list handle-match @0x5a90d4). **From-field = the actor's OWN reference handle: `[[action+0x14]+0x1DC]`** raw @0x5a90fa-0x5a9104 — the +0x14 funnel store (T-030/R4) composes with the T-043 +0x1DC handle; the same handle value the natives fetch as FROM.
  **Update** = `movb $0x1,[this+0x18]; xor eax,eax; ret 8` @0x5a9070-0x5a9076: sets the done byte, consumes 0 ms → the Message action **finishes on its first tick** (no idle-style HOL residency — the isFinished getter 0x404b60 then reads TRUE, pump erases+dtors). Payload buffer freed once after the fan-out @0x5a9113 (action owns it exactly one delivery).

## 3. Consumers (census-complete, all Entity-relative)

- **The pump pop is the only reader**: T-074 mapped shape re-verified raw — `mov 0x37c(%ebp),%eax` @0x54c643 → head=@0x54c651 → empty-check @0x54c653 → `RunScript("OnMessage", …, 2, record.data)` @0x54c66f-0x54c676 (string @0x69200c `'OnMessage\0'` raw-dumped) → **re-entrancy dance**: if the handler itself drained the node, skip unlink @0x54c67b-0x54c681; the in-flight record parked at +0x384 is freed exactly once @0x54c6a8-0x54c6c4 even if the node was consumed re-entrantly. **At most one message dispatched per pump frame.**
- **Discard-pop `meth_0x54b3f0`**: pops one node with full 3-free teardown (node @0x54b421, data via `0x616d87`, record @0x54b43e-0x54b444) — called exactly once from the entity reset/destroy `meth_0x54fff0` @0x5501b0.
- **Destroy sweep `meth_0x4b9d60(this=&entity+0x378, …, Block=head, …)` @0x550287**: frees remaining NODES (loop @0x4b9d83-0x4b9d9f: unlink → `j__free(node)` → `[+0x380]--`) and zeroes the header (`[+0x37C]=0`, `[+0x380]=0` @0x550290-0x550298). **Byte-level observation, stated without speculation:** the generic loop frees nodes only — record+data of messages still queued at destroy time have no visible free on this path (beyond the one discard-pop). Whether that is a shipped leak or a "≤1 message outstanding at destroy" invariant holds in practice is NOT determinable statically from the image — named, with EAs, for anyone who ever lives-instruments a destroy-behind-queued-mail scenario.

## 4. Canonical queue-map row (appended to the T-074 §5 table, same shape)

| | **Script mailbox (+0x378 triple)** |
|---|---|
| Entity offset | +0x378 unused / **+0x37C heap end-marker ptr** / +0x380 count / +0x384 in-flight guard |
| Structure | intrusive **std::list** (12B nodes {next,prev,record*}), NOT the deque family; record 8B + data 4(n+2)B; 3 allocs/message |
| Fed by | `sub_54EE60` ×5 sites: natives SendMessage 0x5c5503 / ToTeam 0x5c5908 / ToSquad 0x5c5a7a (direct) + ActionMessage **start** fan-out 0x5a9107/0x5a91a7/0x5a9214 (engine kind 7, via +0x304 pipeline) |
| Drained by | pump pop (≤1/frame, RunScript OnMessage, re-entrancy-guarded); discard ×1 + node-sweep at destroy/reset; marker block freed at destroy |
| Carries | script-visible **messages**: data = {from entity handle (+0x1DC-cell), msgId, n payload dwords} — what an AMX `OnMessage(from, id, …)` handler sees |
| Relations | ActionMessage start is a **bridge** +0x304→+0x378 (one frame later, per target); the +0x2E0 EFFECT inbox and T-048 +0x319 latch are uninvolved; the from-handle composition [[action+0x14]+0x1DC] = funnel(R4)∘T-043 |

## 5. Port relevance — named, not invented

- No currently paired observable reads this surface (digest R3 counts +0x314, R4 stores action+0x14, +0x319 lives in walk #2 — all +0x304-side; mailbox fields are invisible to every shipped-side pin in the table). **Nothing here changes T-071 or the digest pairing.**
- What this row files for the script-VM/combat-port era (NAMED only): (i) an `OnMessage` dispatch seam with **one-message-per-frame** cadence and a re-entrancy guard (cheap to mirror: pop head iff non-empty, else skip); (ii) ActionMessage kind-7 as a +0x304→mailbox **bridge action that finishes on first tick** — its observable set (start=deliver, update=done-byte, 0 ms consumed) is fully pinned above should a row ever pair it; (iii) the destroy-path record-free question (§3, with EAs).
- The engine-kind numbering table (§2b) is immediate inventory for T-071's kind-alignment step: **6=Anim, 7=Message, 8=reserved/FATAL, 9=Equip, 10=Unequip, 11=SetActiveWeapon** on the shared engine side, identical numbering across both factories (no rebase — -0x230 is this-only).

## 6. Census method & catches (per item 41/43 law)

- Query trio for "who touches the list": (a) raw offset sweep over the whole .text for `0x37c(%r)`/`0x380(%r)`/`0x378(%r)`/`0x384(%r)` (all base registers) — 40+ hits resolved to Entity-band vs offset-collisions: the non-Entity hits are string classes at +0x378 (0x469eb4/0x46a5f0 `meth_0x401b20` string assigns — decompile-confirmed different class), GUI/StringTree byte+float fields (0x4f8xxx/0x531xxx), and a 0x380-array lea @0x5eaaf1 (rep-stos block) — all excluded by context, stated not guessed; (b) IDA `xrefs_to` on the push primitive (complete, 6) and on `meth_0x54b3f0` (2); (c) vft/jump-table raw reads for the action protocol + both factory tables.
- **The frozen export missed the census entirely** this time: Entity's member list has NO +0x378 or +0x384 members at all, and its +0x37C usage list (7 sites) omits the push primitive (@0x54eeb4), the discard (@0x54b3f3/ff) and every count op — a stronger blind spot than T-074's +0x171 case (item 27 class, quoted at function/raw granularity per item 43).
- Item 44 posture: no membership claims made; `ActionMessage`/`SendMessage` identities come from RTTI + IDA live names + verbatim native-name strings, not band adjacency. Selftest 6/6 (incl. the new T5 share-check) run before the verdicts.
- Item 40 routing reused: no MCP arg-corruption hit today (objdump primary from the start).
