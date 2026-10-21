# T-054 — ecx-flow owner-attribution tool + three escalated-owner adjudications

**EpicRaven**, dispatched by LoudDragon `01a15f82`. OFFLINE lane: raw EXE + checked-in dumps only;
no 8745 queries, no IDB writes (this row proposes, a writer wave applies under ledger).

## 0. Evidence pins (cache lineage, cold-proof)

- EXE: `$OPENVB_ORIG_EXE` = `/nix/store/a1j412798wh979ig7xp3bp9aji4zfpz0-f3demo-1.0/share/f3demo/F3.exe`
  **md5 `a68c22f3dd1c256ec9539a70e87cb434`** (equals `artifacts/F3.json[filemd5]`, case-insensitive).
  The flow cache `obj/t054/flow-<md5>.json` is keyed by this md5: writer churn cannot touch it.
- Dump: `artifacts/F3.json` mtime 2026-10-20 22:39:50 — taken BEFORE the T-055 wave started.
  Its names are PRE-rename locators only; per LoudDragon's stale-boundary note, live-IDB names may
  differ (address-stable). Every claim below is carried by addresses + raw bytes, not names.
- Section map (objdump -h): .text 0x401000+0x245c52, .rdata 0x647000+0xa9790,
  .data 0x6f1000+0x17000, .data1 0x712000+0x8e0. Image base 0x400000.
- Cross-lane notes carried: CalmRaven's `\xc7`-pad string finding (string owner joins must xref
  EA and EA+1 — this tool does no string joins, recorded for cited b5/b6 string evidence);
  CalmRaven's sole-storer predicate (ctor = sole object-side storer, a `CEngineInterface::meth_*`
  factory may be a second storer — scored as EXPECTED below, never a disqualifier).

## 1. The tool — `tools/ecx-flow.py`

Offline `this`(ecx)-propagation engine: objdump-disassembles the raw EXE (827k instrs, ~4 s full
build, md5-keyed cache), joins the four classes T-033 batches lacked:
[A] singleton-this joins (`mov/lea $G→ecx` this==&G; `mov G→ecx` this==*G pointer-slot;
embed `this=&G+d`/`in-ecx+d`), [B] jmp thunks incl. the virtual-base-adjusted
`sub [ecx-4],ecx; jmp T` thiscall forwarders, [C] raw vftable scan of **.rdata+.data+.data1**
(the b5 window was 0x640000–0x680000 only) + table INSTALL sites + slot-offset dispatch sites,
[D] allocator/ctor callers (`new(size)`→ecx→ctor, MSVC ctor returns this ⇒ `[base+disp]←%eax`
install; weak tier = any [this+disp] store within 0x60B). CFG approximations (function-head
attribution, dead-region waking after ret/jmp, branch-merge write-set clearing, jump-table
resurrection) are documented in the file header.

Self-test gate (item 35): band/global verdicts are REFUSED while the gate is red. Current run:

```
PASS T1  Gfx_D3D <- &0x7082F8 singleton-this join      314 edges (b5 claimed 170+ sites)
PASS T2  AnimMeshInstance ctor <- allocator/caller     4 new(0xC8)->ctor_0x4A5BE0 edges,
         join (b6-corrected shape)                     1 strong install [this+0xC], 4 weak [this+0x2C4]-family
PASS T2b embed detector known-positive                 this=in-ecx+0x44 @ 0x564D3C (ModelInstance+0x44)
PASS T3  Gfx_Texture_Manager <- *[0x70B200]            36 edges
```

Reproduce: `python3 tools/ecx-flow.py selftest` / `cases` / `band --cls X` / `global 0xX`.

### Item-35 ladder walks performed (empty-results investigated before being believed)

1. **b5's `VFX_Model+0x200 embed` did NOT reproduce — and is a real absence, not a scan failure.**
   All four call sites of `ctor_0x4A5BE0` enumerated from raw relocations: 0x4F419A, 0x4F4490,
   0x4F490C (VFX_Model methods) and 0x564EA8 (`ctor_0x564CE0`). Every site is
   `push $0xC8; call new(0x6054B0); mov %eax,%ecx; call 0x4A5BE0` — **heap this, no embed anywhere**.
   The b6 correction is hereby confirmed at full-`.text` scope (b6 had corrected one site; the other
   three are the same shape; arg side-channel = `[container+0x2C0]`, instance stored at
   `[container+0x2C4]` (VFX, e.g. 0x4F41B3) / `[this+0xC]` (ModelInstance, 0x564EB1 — strong install)).
   The embed detector itself is proven live by T2b before this negative is asserted.
2. **`this==&0x70B200` does not exist; `this==*[0x70B200]` does (36 edges).** b6's wording
   "this=&dword_70B200 immediate at call site" describes the IDB symbol form; the bytes are
   `8B 0D 00 B2 70 00` = `mov 0x70b200,%ecx` (pointer-slot singleton, [A]-contents class).
   First-version scans keyed on the `B9/8D` forms returned EMPTY against this known positive —
   classic item-35; fixed by supporting both operand shapes before any conclusion was drawn.
3. **`slots into band = 0` for cls_0x445350 reproduced b5's vft negative — then beat it.**
   The scan (wider than b5's window) still finds no table pointing INTO the band; the band was
   never vft-driven — its HOST was. See §2: the table points to the host's thunk, the host calls
   the band with a constant `−0x198` this-offset. b5's question ("does any vft point into band?")
   was structurally unable to find containment through a delegating host.

## 2. Adjudication — cls_0x445350 (hotkey label-pair controller, b5 §2 escalation)

**Verdict: MINT-READY (owner found — a containment chain through thunk+vft indirection).**

Flow path that reaches (all sites re-verified against raw bytes):

- vft **0x659DB0 = `.?AVCOptionsHotKeysInterface@@`** (F3.json length 6; raw slots
  0x659db0:`0x445860`, +4:`0x4953C0`, **+8:`0x4468B0`**, +0x10/+0x14:`0x404B70`, +0x18:`0x445870`).
- **0x4468B0 is an adjusted thiscall thunk**: `sub -0x4(%ecx),%ecx; jmp 0x4465E0` — resolves to the
  handler **0x4465E0** (unnamed in F3.json; NOT in the band, which is why b5's direct-call scan saw
  "zero external sites": it never scanned callers-of-callers).
- **0x4465E0 calls 7 of the 8 band entries at 14 sites, every one with `this = in-ecx − 0x198`**
  (0x446654/5B/62/99/A0/F0/F7, 0x446705/3E/7A/85/A0/C8/DC; register chain
  `mov %ecx,%ebx` 0x4465E1 → `lea -0x198(%ebx),%esi` 0x4465ED → `mov %esi,%ecx` at each site).
  I.e. the band object is the OUTER object: the COptionsHotKeysInterface view sits at
  outer+0x198, and the thunk converts view→outer via the `[view−4]` delta (which stores −0x198,
  see installer below).
- The table's object-side installer: **`cls_0x4468C0::ctor_0x4468C0`** (F3.json size 457 ≥ 0x1C9,
  consistent) — at 0x446949 `movl $0x659DB0,(%esi,%ecx,1)` (installs the interface vft at the
  +0x198 view through the delta) and at 0x44695B stores the −0x198 displacement into `[view−4]`;
  it also installs the outer vft `movl $0x65A290,(%esi)` (0x446922) and constructs the subobject at
  `lea 0x198(%esi),%ecx; call 0x580330` (0x44691C/28).
  Factory-second storer at 0x445763 (`.?AVCEngineInterface@@::meth_0x445740`) — EXPECTED per
  CalmRaven's predicate, logged not disqualifying.
- The outer object itself is constructed by **`.?AUGameInterfaces@@::ctor_0x4398E0`** at site
  0x43A1CE (`mov %edi,%ecx; call 0x4468C0`), result installed at `[this+0x58]` (0x43A1D7) —
  the GameInterfaces slot-table lane.
- Band ownership: host-unique member functions of the 0x4468C0 outer object (OOAnalyzer split —
  same observation-floor artifact class as b5 §1a's `cls_0x502870`/`cls_0x490060` on one global).
- Residual consumers, unchanged in kind: `cls_0x442BF0::meth_0x442DC0` calls meth_0x445380 via the
  `*[*[*[*[0x70BD90]+0x2C]+0xA0]+0x58]` field chain.

**Wave queue (proposed, not applied):** mint band {0x445350,0x445380,0x4455D0,0x445970,0x445A40,
0x445E20,0x446080,0x4461D0} + {0x4465E0,0x4468C0,0x4461E0} as the `COptionsHotKeysInterface`
implementation object (name per coordinator — the RTTI for the OUTER class itself is not among the
shipped `.?AV` set; `HotKeyOption`/`HotKeyWindowFormat` are the ±0x10-adjacent table LIST-ITEM
classes installed at 0x445644/0x445694 from inside band code, not the owner).

## 3. Adjudication — cls_0x4b28a0 (GUI root, consumes CWorldContainerInterface)

**Verdict: ANNOTATE (membership question is VACUOUS — no container exists; singleton join is
proven at maximal purity; naming remains blocked, so it is not mint-ready).**

- 166 external call edges into the band, **166/166 with `this == &0x707E48`** — zero heap-ctor
  edges, zero `*global` edges, zero embed edges, zero `[field]←&0x707E48` installs, zero thunk
  edges, zero vft slots into the band (28 entries, full data-section scan). The band is ONE
  static object living at global 0x707E48 (.data image bytes all-zero at that VA — runtime-init).
- That is the identical join shape b5 used to mint `JE::Gfx_D3D` at &dword_7082F8 — the singleton
  join is not weaker here; what is missing is a NAME: b4 byte-rescan found zero shipped strings,
  no RTTI join exists, and no head-body proof (trivial getter). The escalation asked "who contains
  it" — the answer the flow gives: **nobody contains it; consumers surround it** (Edit@GUI,
  CMainmenuInterface, CEscapeMenuInterface, CPipBoyAddNoteInterface, CDebugInterface,
  CDebugToolsInterface, LoadMapInterface, Graphics — the GUI/modal pump family). Minting it under
  any containing class would fabricate membership the image denies.
- Suggested queue: comment at 0x4B28A0 recording 166/166 singleton purity + vacuous-membership
  finding; keep escalated for NAME only, not for owner.

## 4. Adjudication — cls_0x507810 epoch (byte_707CE0 anchor, b6 §3 negative)

**Verdict: MINT-READY (owner found — this is a NEW fact over b6's "owner unresolved").**

- Construction: **`.?AVGfx_Engine@@::ctor_0x5BFE10`** does `push $0x50;
  call new(0x6054B0); mov %eax,%ecx; call 0x507810` at **site 0x5BFFEC**, then installs the result at
  **[engine+0x30]** (field store 0x5BFFFC in the same flow; siblings: +0x2C ← 0x5211A0 =
  texture-manager epoch, +0x38 ← 0x5CB510 — the constructor allocates a trio).
  `new(0x50)`=80 == F3.json `cls_0x507810` size 80 (and == `cls_0x5067c0` size 80 — same object).
- Teardown corroboration from the other side: **`Gfx_Engine::dtor_0x5C1CD0`** site 0x5C1D41 calls
  **0x507580** (the b6-noted second 707CE0 writer, `89 1d` form) with `this = *[in-ecx+0x30]` —
  the dtor reads the epoch back through the SAME field.
- The anchor explained: `cls_0x507810::ctor_0x507810` self-registers the fresh heap pointer into
  **byte_707CE0** (flow-recorded write at 0x507831 inside the ctor — b6's "89 35"), and 0x507580
  (dtor path) rewrites/clears it at 0x50764D. 8 band edges read it back as `this==*[0x707CE0]`
  (incl. `Gfx_Engine::meth_0x5BEF20`→0x506890, `virt_meth_0x5C0C30`→0x5067C0) — **the global slot
  is a pointer cache of the engine member**, not an object address (contrast 7082F8).
- `cls_0x5067c0` (meth_0x5067C0/meth_0x506890) are called with `this==*[0x707CE0]` = the same
  engine+0x30 object → same class, two OOAnalyzer fragments.
- b6's negative came from looking for a GLOBAL object at 707CE0 (`[707CE0]` as &global anchor);
  the epoch is heap-owned, so only the allocator/ctor-caller join [D] can see it — exactly the
  class this tool adds.

**Wave queue (proposed, not applied):** merge `cls_0x507810` + `cls_0x5067c0` (+ sub_0x507580
dtor-side) into one class = member at `Gfx_Engine+0x30`, heap `new(0x50)`, singleton-cached at
byte_707CE0; mint under a name chosen by the wave (no shipped string names it).

## 5. Census of what the tool permanently arms

- 345 vftables recovered offline from raw bytes (vs b5's windowed scan), slot→target map for
  every table, install-site list per table; totals per pinned build: 44,690 call/alloc/thunk
  edges, 5,195 vft dispatch sites, 18,352 tracked stores, 983 thunk chains. The three
  escalations above are the first outputs; any future escalated owner is one `band --cls X`
  away from the same four joins.
- Tool exit codes: selftest 0/1; band mode refuses verdicts on a red gate (item 35) unless
  `--allow-red`. No IDB writes exist anywhere in the code path.
