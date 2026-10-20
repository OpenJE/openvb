# 2025-10-20 — 10e SETTLED: the turn-dispatch mode flags are a dormant path (session 43)

Task row: T-010 §10e (owner NiceWolf). Object: inline 36-byte manager at **GameWorld+0x8C**
(external caller `JE::World::virt_meth_0x545D60` @0x545E0C: `mov ecx,[esi+0x8C]`).
DB vftable_count 0 = plain struct, no vft — inline. Methods: setter 0x5A9330,
processor 0x5A94A0 (+push 0x5A93C0).

## 1. Field map (evidence addresses)

| off | role | evidence |
|-----|------|----------|
| +0x4 | "per-turn mode" per the row — actually **caller-arg store** (both setter branches: 0x5A936E and 0x5A93AD `mov [edi+4],ebp`) | consumer @0x5A951D `if (!this->mbr_0x4)` selects full-sweep vs single-dispatch |
| +0x8 | in-flight guard (set 0x5A94AB, cleared 0x5A9551) | purge phase runs once per outer entry |
| +0xC | queue cursor | sweep loop 0x5A953B..4F |
| +0x10 | round-clock accumulator ms (+= 1000×GW+0x40 per dispatch) | 0x5A95A1-A7 (canonical §13) |
| +0x14 | "recursion condition" byte | READ 0x5A9575; CLEARED 0x5A9371/0x5A93B0 — **never set, see §2** |
| +0x1C | queue sentinel (circular) | push 0x5A93C0; iter 0x5A94BF |
| +0x20 | entry count (push ++ / free --) | 0x5A94EF |
Node = {next, prev, event@+8}; event+0x1E0==5 → dispatch via event vft[+96](dt), else the node is freed during purge (0x5A94CC..). Per-entry tick = `F3::ServerUpdate(event, dt)` @0x5A953B.

## 2. The dormancy chain (raw-scan verified, three independent negatives)

1. `+0x14 = 1` has **no writer anywhere in .text**: no `[reg+0x14],0x1` (cluster scan
   0x5A8E00..0x5AB000), no register-valued byte store to that shape exe-wide (scan
   0x460000..0x5C0000; hits were esp-local stack vars), DB usage list has only the 2 clears
   + 1 read. Init: JE::World/GameWorld ctor zeroes the 0x80..0x8C band (session 14 raw
   window @0x548b54..0x548b66 — +0x8C is the band's fourth dword).
2. ⇒ the READ @0x5A9575 is never true ⇒ the only call to the setter (`call 0x5A9330`
   @0x5A957F, passing the CLOCK value — xrefs to 0x5A9330: this site + self-entry artifact
   only) **never executes**.
3. ⇒ `+0x4` keeps its ctor zero forever ⇒ processor branch `if (+0x4==0)` (0x5A951D) is
   **always taken**: purge-then-full-sweep `ServerUpdate` over every queued event.
   The single-dispatch half (0x5A955A.. + event+0x319 explicit-deadline → event+0x29C,
   session 41's alternate) is **unreachable in the shipped build**.

⇒ The row's either/or ("another event queued" vs "multiple entries") was a false binary:
the flag belongs to a **disabled re-arm/turn-stepping feature** — likely a paused-combat or
turn-mode toggle that shipped unhooked (compare: quest natives `return 1`, evasion stub,
console-only death). Answer to "who writes +0x4/+0x14": the setter writes both, and nothing
calls the setter.

## 3. Consequences (for T-011 and the defect ledger)

- **T-011 dispatch loop:** ship behaviour = full-sweep branch only. Model the
  single-dispatch branch as present-but-unreached (mirror + cite under the defect-ledger
  rule; do NOT invent a producer).
- The event node's type field (`event+0x1E0==5`) gates purge-survival: type 5 events stay,
  everything else is freed un-dispatched during purge — worth one line in T-011's spec:
  **non-5 queue entries are dropped at next dispatch**, they do not execute.
- `+0x296` entity byte sweep by the setter (notify via 0x548EE0 on branch-1 arg==1,
  0x548F30 on branch-2 — per-entity notify pair): dormant with the setter; the +0x296
  participation byte convergence (PackState step-18) remains live via other readers —
  unchanged, just no longer tied to this path.
- Coordinator: propose ledger entry "dormant combat-turn stepping (GameWorld+0x8C manager:
  setter unreachable, mode flags frozen at ctor zero, single-dispatch branch dead)" beside
  the null-page addend / quest stubs / console-stub commands.

## 4. Method notes (for reuse, per harness items 8b family)

- DB member usage lists (7 members, +0x4 had 3 usages, +0x14 had 3) MISSED nothing here —
  but would have missed it if the setter were reached via vb-adjusted this; raw
  immediate+register-store scans both ran precisely because the census has that known blind
  spot (session 28/34 history). Two negatives + a call-graph dead-end = the claim.
- "Who calls 0x5A9330" single-xref fact came from IDA MCP via curl fallback (harness tool
  rotation) — lane-verified after the fact.
