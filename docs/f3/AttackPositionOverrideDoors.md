# Who calls `GameActionAttack::meth_0x401100` — the position-override setter (T-044)

PureKnight (sessionId 01a15cb7-e27e). ORIGINAL_REFERENCE (`ida_original_f3` / `artifacts/F3.exe.i64`).
Companion to `docs/f3/GameActionAttack.md`, `docs/f3/ScriptNativeActionDoors.md` (row `+0x44/+0x48/+0x4C/+0x50/+0x54`
which said "whoever calls `meth_0x401100` is a different door" — it is now identified).

## Verdict — a location-targeting attack DOES exist in the shipped surface

Exactly one caller, from the **server packet door**:

| caller EA | door | sets |
|---|---|---|
| `0x46df14` | **server packet ID 91 (0x5B)** — `F3::GameServer::virt_meth_0x46dd70`, `switch (packet->mbr_0x30)` `0x46ddc5`, `jumptable 0046DDC5 case 91` at `0x46de30`; function logs `"GameServer::recv_packet() - Entities for attack action not found."` (`0x46df2b`) — it IS the server receive dispatch | `meth_0x401100(new GameActionAttack, hasPos, &pos)` |

Case 91 record build (EAs in order):
- int#1 `0x46de37` → attacker ref id; int#2 `0x46de43` → target ref id; int#3 `0x46de4f` → stored to **record `+0x78`** (`mov [esi+78h],edx` `0x46df11`);
- byte `0x46de5b` (`meth_0x45e460` = byte reader) → **hasPos** flag (bl, `0x46de60`);
- if hasPos: three floats (`sub_45E560` = float reader) at `0x46de87/0x46de93/0x46dea4-ish` → `v130/v131/v132`;
- entity lookups via ref→entity `0x54be10`; accepted iff `attacker && (hasPos || target)` (`0x46decb`);
- `meth_0x403fa0(rec, targetEntity)` `0x46df00` (target ref group `+0x34/+0x38`);
- **`meth_0x401100(rec, hasPos, &v130)` `0x46df14`** → flag byte `+0x48`, override position `+0x4C/+0x50/+0x54`
  (member-DB usage EAs `0x401106/0x401115/0x40111b/0x401121`; `ScriptNativeActionDoors.md` line 180 agrees);
- enqueue for the attacker via `Entity::meth_0x550720` `0x46df1c` (same seam the `Attack()` native uses).

**Record fields that carry the attack-at-location**: `GameActionAttack+0x48` (presence byte) +
`+0x4C,+0x50,+0x54` (X,Y,Z float override). With `+0x48 != 0` the target entity ID may be 0/absent —
case 91's accept-guard is `attacker && (hasPos || target)`.

## Consumers prefer the override over the target entity (verified)

- `GameActionAttack::OnCreate_0x4011d0` (vft `0x647bbc` slot 4): `FatalError "No target was specified"` only when
  **both** target ref and `+0x48` are empty (`0x4011da`); liveness checks skipped when `+0x48` set (`0x40121f`).
- `OnServerUpdate_404320` (vft slot 5): `mov al,[esi+48h]` `0x404410`, `jnz 0x404446` **skips the GameEntity cast/resolution
  of the target entirely** and proceeds on the record position (also reads `+0x48` at `0x4046d1`, `+0x54` at `0x4046e3`).
- `meth_0x4012e0` `0x401315` (`+0x48`) / `0x40132b` (`+0x54`) and `meth_0x402e00` `0x402e7e` (`+0x4C`) / `0x402e84` (`+0x48`)
  — the aim/position helper group prefers the override.

## Door classification and why no script native reaches it

`ScriptNativeActionDoors.md` §`+0x44..+0x54`: the `Attack`/`JustAttack` natives build the record in-process and
**never** set `+0x48` — so through the AMX native surface only *entity*-targeted attacks exist. The location mode is
reachable only by **injecting packet ID 91 with the hasPos byte set** (remote client → server socket, or the in-proc
server loop of the demo). AI: no caller found in AI ranges. Engine: no other caller.

## Boundary — completeness statement (honest scope)

- `meth_0x401100` is a plain method, **not in any vftable** (`GameActionAttack` vftable `0x647bbc` entries:
  `0x403f80,0x403d50,0x402080,0x4011d0,0x404320,0x4021b0`); IDA xrefs to it are only `0x46df14` + the sequential
  artifact `0x4010ff`. No `call [reg+disp]` shape can reach a non-virtual setter without a data reference, and there is
  none. Caller table is complete **for the shipped image**.
- The *caller itself* (`0x46dd70`) is reached via `GameServer` vftable `0x667dd4` slot 26 — resolved, so the door is
  fully attributed. What is NOT claimed: that no external process other than a demo client speaks this packet ID —
  the whole point is the socket path accepts it.

## Follow-on (for the coordinator to number)

Driver impact: `attack <actor> <target>` cannot express `attack <actor> <x y z>`. A separate driver verb
(e.g. `attack-pos <actor> <x> <y> <z>` writing packet 91 fields `hasPos=1, +0x4C/+0x50/+0x54`) is warranted —
the mode is real, shipped, and consumed by `OnServerUpdate`. Row to be added by ZenNova (numbering owned there).
