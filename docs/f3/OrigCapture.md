# T-031 shipped-side capture grammar (`tools/orig-digest.sh`)

The parity oracle needs the **shipped** side of the digest to exist as a file. It cannot come from
our code, and it should not come from a debugger sitting in a loop: it comes from a **capture** —
dwords somebody read off `F3.exe` at named instants — rendered by `tools/orig-digest.sh` into the
same line grammar as `src/F3/SimDigest.hpp`, so `tools/diff-digest.sh` stays the only comparator in
the repo (one comparator, two producers: `docs/f3/ParityOracleDesign.md` §4).

Run it:

```sh
tools/orig-digest.sh --capture tools/scripts/orig-capture-synthetic.txt \
                     --provenance synthetic-fixture --out /tmp/orig.digest
tools/diff-digest.sh /tmp/orig.digest /tmp/orig2.digest      # 0 same, 1 divergence,
                                                             # 2 usage/IO, 3 MIXED-PROVENANCE
tools/orig-digest-verify.sh                                   # 47 checks, every one can fail
```

## PROVENANCE is required input, and the label has to be earned

No provenance, no file: exit 3, nothing written. The field exists so a reader holding one digest
months later can tell captured state from invented state — a default would destroy the only thing
it is for.

* anything that is not a verified `lockstep` claim ⇒ the **file** says
  `# SYNTHETIC - exercises the tool, proves nothing about F3.exe`. Deliberately synthetic input is
  expected and fine; a synthetic side that does not say so is not.
* a `lockstep` claim is **checked, not believed**. It must
  1. name the reads it used (`r3` and `r4b` at minimum — they are what `q=` and `ak=` come from),
  2. carry a `record=read` line per named read (address / value / when / breakpoint — the same four
     columns T-034's acceptance demands),
  3. be **consistent with its own frames**: `r3`'s value must appear as some entity's `q=`, `r4b`'s
     as some `vt=`, `r1`'s as some frame's `lcgstate=`,
  4. cite `exe=<md5>` equal to `artifacts/F3.json`'s `filemd5` — otherwise the capture and the class
     table describe different binaries and every resolved name is a coincidence.

Exit 3 with `PROVENANCE-UNVERIFIED: <reason>` when any of that fails.

**This is sloppy-proof, not forgery-proof.** The md5 is public, so a determined author could write a
complete-looking fake. What it does stop is the failure mode that actually happened on this row: a
fixture compared as if it were evidence. The durable check is the session id plus the read table,
which a reviewer can re-run.

## The capture format

`key=value` tokens, one record per line, `#` comments allowed. Record types and their fields are
**closed**: an unknown record type, an unknown field, or a malformed value is refused
(`INVARIANT-VIOLATION`, exit 4, nothing written) rather than skipped — a capture full of typos would
otherwise render a clean-looking digest.

```
record=provenance provenance=lockstep r1,r3,r4,r4b session=<id> exe=<md5>
record=read  read=r3 address=Entity+0x314 value=2 when=mid-round breakpoint=0x459f5d
record=frame frame=100 tickms=3456789 clocksec=34567 [lcgstate=0x12345678]
record=entity frame=100 slot=0 name=CREEP pos=20.00/0.00/0.00 yaw=0.00 hpcur=45 hpmax=45 \
                 state=4 moving=0 acted=0 q=1 vt=0x649c1c,-,-,-
record=world frame=100 pathjobs=0 turns=2 turnorder=CREEP,OTHER
record=end frames=3
```

| field | where it comes from on the shipped side | note |
|---|---|---|
| `frame` `tickms` `clocksec` | the frame boundary you broke on; `JE_TickMilliseconds` = `timeGetTime`; `world+0x3C` → `Gregorian::totalSeconds` | `tickMs`/absolute `clock` are **CONTEXT** (not hashed); the digest hashes `clockDelta` |
| `lcgstate` | **R1** — `F3::start_time`, the dword advanced at 0x48ccde/0x48cce9, read after the first `sub_48CCC0` return | printed as `lcgState=` inside CONTEXT: it is the pinned state the run replays from, not per-frame sim state |
| `name` | the entity's own name string (names live in the entity, not the list) | |
| `pos` `yaw` | Entity `+0x04` (float[3]) and `+0x1F0` | `%.2f`, so a `pos`/`yaw`-only divergence is a formatting question before it is a state question |
| `hpcur` `hpmax` | the creature stat pair `+0x24`/`+0x20` | **both**, or the comparison is meaningless |
| `state` `moving` `acted` | `+0x1E0` (dword), the mover flag, `+0x319` (byte) | `st=` is three values since T-039 |
| **`q`** | **R3 — `Entity+0x314`, the action deque's live element count** (`++` at 0x54ea71, `--` at 0x54b637/0x54b672) | the D-2 number: shipped `q>4` beside our `q=4` **is** a dropped order, visible in the diff |
| **`vt`** | **R4b — `[action+0x00]` per deque slot**, four entries, `-` when the slot is empty | mapped to a CLASS NAME offline through `artifacts/F3.json`; unresolved prints `UNKNOWN_VT=<ea>` and is **counted** in `# AK-RESOLUTION=`, never named. `--require-resolved-ak` turns an unresolved slot into a refusal |
| `pathjobs` | `PathManager+0x0C` | omit it and the file prints `pathJobs=NA:...` — NA means *not measured*, never *unchanged* |
| `turns` `turnorder` | initiative manager `world+0x8C`: count `+0x20`, circular list `+0x1C` (order needs the WALK, not the count) | omit ⇒ `turns=NA:...` |

The contract block (`# SIM-DIGEST v1`, `# entity-fields=`, `# COMPARED=`, `# EXCLUDED=`,
`# NA-means=`, `# SEED-LIMIT`, `# PROVES=`) is a **copy** of the ours-side text maintained in two
places on purpose. Auto-extracting the C string literals was rejected — one reformat of
`WriteHeader` would turn the emitter into a silent garbage producer — so the check is at the other
end instead: `tools/orig-digest-verify.sh` compares the contract block of a real binary-produced
digest against the emitter's **byte for byte**, recomputes `fnv` independently for every ours-side
line, and asserts the line endings match (the binary writes CRLF through MSVC text mode; an
LF-only emitter made every frame of a real comparison "diverge" by one invisible byte, which this
test found the day it was written). A drift shows up as `CONTRACT differs`, at the comparator, on
both sides, and never as a state divergence.

## Why `draws` is `NA` on this side

The shipped generator (one global LCG, `sub_48CCC0`) has no draw counter — adding one means editing
`CombatMath.cpp`. Our harness injects a counting `RandFn` and prints `draws=N drawsSrc=injected-counter`;
the app path and this emitter print `draws=NA:...`. The comparator treats **NA-vs-value** and
**NA-vs-NA with different reasons** as a COVERAGE difference (reported, labelled, exit 0 if nothing
else moved), because otherwise every ours-vs-original run would report the same fake finding. The
pinned `lcgState` in CONTEXT is the equivalent observable, and it is what §1's capture-and-replay
seeding pins both sides to.

## Mixed provenance is refused, not compared

A SYNTHETIC side compared against a real capture (or against a run that declares no class) looks
*exactly* like a divergence finding and is actually a fixture bug. `tools/diff-digest.sh` therefore
refuses before comparing anything:

```
MIXED-PROVENANCE: no comparison performed
  orig-a.digest: PROVENANCE-CLASS=synthetic
  ours.digest:   PROVENANCE-CLASS=unrecorded
```
exit **3**. The sanctioned pairings: `lockstep` ↔ `unrecorded` (the real parity run), `unrecorded`
↔ `unrecorded` (self-consistency, as today), `synthetic` ↔ `synthetic` (a fixture exercise, which is
what exists today and is labelled as such). `synthetic` ↔ anything else is a refusal.

## Boundary, printed in the tool's own output

A capture is a render, not a read. A memory read sees what the shipped code **stored**, not what it
**intended**: presentation-side fields we do not have will differ harmlessly, and identical values on
a path we never reach prove nothing. So a divergence is a **question with a location**, not a verdict
on the port — and until T-034's R3/R0 land, every artifact this tool writes is SYNTHETIC and says so
in its first twelve lines.
