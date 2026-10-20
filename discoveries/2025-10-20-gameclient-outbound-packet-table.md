# 2025-10-20 — GameClient outbound packet table (ids 72..112) + net:: RTTI names (session 10)

Target: ORIGINAL_REFERENCE. Read-only. Supersedes session 9 §5.3's guess ("client-side handler
family of 0x556ea0") — the 0x45ef70..0x4605e0 family is **outbound request senders**, not
receive handlers.

## 1. Uniform sender shape (22 functions)

Every family member: stack `net::packet` (0x2044-byte object, buffer ~0x2000), `ctor_0x45f300`,
then **store packet id at packet+0x30**, serialize fields via `meth_0x45f130(int)` /
`meth_0x45f0b0(?string)`, `GameClient::meth_0x59c4f0(this, packet, 1)` = SEND, dtors
`net::address` (vft 0x6647a0) + `net::buffer` (vft 0x6648e8), overflow throws
`net::overflow<net::buffer>` with message `"Buffer overflow."`.

## 2. The F3→wire request table (sender EA → packet id)

| sender | id | payload (observed) |
|--------|----|--------------------|
| 0x45f370 | **91** | 3×int + bool(a5!=0) + optional 3×int triple |
| 0x45f4f0 | **72** | 2×int |
| 0x45f5d0 | **73** | 2×int |
| 0x45f6b0 | **74** | (1×int family) |
| 0x45f790 | **75** | |
| 0x45f870 | **76** | |
| 0x45f950 | **77** | |
| 0x45fa30 | **78** | |
| 0x45fb10 | **79** | |
| 0x45fbf0 | **80** | |
| 0x45fcd0 | **81** | |
| 0x45fdb0 | **82** | |
| 0x45fe90 | **83** | |
| 0x45ff80 | **100** | |
| 0x460040 | **84** | |
| 0x460100 | **90** | |
| 0x4601f0 | **110** | *empty* (no payload) |
| 0x4602a0 | **111** | |
| 0x460350 | **112** | |
| 0x460400 | **95** | |
| 0x460500 | **96** | |
| 0x460620ish | **89** | |

(Extraction method: first `[esp+disp],imm(1..0x200)` store after each `call 0x45f300` — script
in this repo's session history; ids verified 1:1 unique.)

**Sequential run 72..84 (13 ids) + 89..91/95/96/100/110..112** — 13 client-submittable
command types. Note: there are 17 `F3::GameAction*` classes in the DB, so it is NOT 1:1 with
actions — only ~13 are client-requestable; the remainder are server-spawned (die, floating
text, dialogue…). Naming requires the server receive-side (queue item 1).

## 3. `sub_436b40` (JE-side, 0x438 bytes) = pending-command flush

Raw: early `cmp eax,1 / cmp eax,0xE` arms pick 0x45f4f0/0x45f5d0; the long linear tail
(`test edi,edi; test ebx,ebx; <4 args>; call 0x45efXX`) walks the other senders in id order
(72,73,74,75,...,84,89,90,91) with identical arg plumbing = **iterating queued client commands
and sending each** — i.e. the client submit-flush pipeline, one arg-shape per id (4 pushed
values: ebp, [esp+0x14], +0x78, +0x7c slots = record header + params).

`GameClient::meth_0x45ef70` (in the same family, but a *gate*, not sender):
RTTI dynamic_casts `entity → GameEntity` and `client+44 → GameCSMenuInterface`, then
`cls_0_467240::meth` (GameEntity script gate) and `sub_4357C0(...)` = command post-filter
through the CS-menu (dialogue-in-progress blocks actions?) *(inferred)*.

## 4. RTTI is already fully harvested in F3.json (no extraction needed)

Checked `artifacts/F3.json`: all 1101 entries carry mangled/RTTI-derived names, including the
complete network class graph seen in this TU: `net::packet`, `net::buffer(+overflow/underflow)`,
`net::address(+exception)`, `net::client`, `net::server`, `net::client::server`,
`net::server::client`, `net::connection(+exception)`, `net::socket(+exception)`,
`net::critical_section`, `net::debug_stream`, `net::exception`, `net::base` — sizes known too
(`net::client` 964B = JE::Client; `net::server::client` 16740B = the server's per-client struct,
consistent with server field 0x5064 offsets seen in 0x556ea0; `Client::Server` = 16740 mirror).
Plus `GUI::` (16) and `std::` (62) types. The `__RTDynamicCast` sightings here were just proof
the binary kept RTTI; the DB already reflects it.

⇒ For openje: these net::* names + sizes are authoritative; use them when naming the network
layer instead of inventing equivalents.

## 5. Server RECEIVE side (added same session) — protocol skeleton complete

`F3::GameServer::virt_meth_0x46dd70` (0xD93 bytes) is the override: switch on `packet+0x30`
via two-tier jump tables (raw: `id-11`, bound 0x65 ⇒ ids **11..112**, byte-map 0x46eb68,
dword-table 0x46eb04, default 0x46eac5). Unhandled ids fall through to the JE base
dispatcher `JE::Server::virt_meth_0x556ea0` (two call sites inside: 0x46eab7, 0x46ead0) —
which owns the control ids (9,10,15,26,40,45,56..61,64,65,67,70) from session 9.

Id → receive-block table (from raw jump tables; ids not listed hit default 0x46eac5):

| id | block | notes |
|----|-------|-------|
| 11 | 0x46ddcc | |
| 58 | 0x46eaac | |
| 72 | 0x46dfdc | shared with id 75 |
| 73 | 0x46df38 | |
| 74 | 0x46e201 | shared with ids 77, 80 |
| 76 | 0x46e010 | |
| 78 | 0x46e044 | |
| 79 | 0x46e114 | |
| 81 | 0x46e235 | |
| 82 | 0x46e2e5 | |
| 83 | 0x46e34d | |
| 84 | 0x46e4dc | |
| 89 | 0x46e827 | |
| 90 | 0x46e567 | |
| 91 | 0x46de30 | |
| 93 | 0x46e63c | |
| 95 | 0x46e69c | |
| 96 | 0x46e788 | |
| 100 | 0x46e3f2 | |
| 101 | 0x46e460 | |
| 106, 107 | 0x46e8b5 | pair |
| 110 | 0x46e8de | |
| 111 | 0x46e97e | |
| 112 | 0x46ea1c | |

**Sender↔receiver pairing now complete for the game-command surface**: e.g. client
`0x45f5d0 → id 73 → server block 0x46df38`. Per-id semantics remain to be decoded block by
block (23 blocks, ~0x40–0x300 each) — the GameServer meths 0x46d020..0x46dc90 are the
probable callees where GameAction* objects get constructed.

## 6. Queue update

1. ~~Server-side receive handlers for ids 72..112~~ TABLE DONE §5; per-block semantics open
   (next: 72/73 pair, then 91 which carries the 3+3 int payload).
2. Per-sender callers (which UI/action queues which id) → confirms action↔id mapping.
3. ~~RTTI extraction sweep~~ CLOSED §4: F3.json already contains every RTTI name; the
   net::*/GUI::/std:: inventory is directly usable. Reimpl may rename address-derived
   classes from it at will.
4. Older: clip-name table, damage Apply remainder, JE::World layout, GENT f1..f6 consumers.
