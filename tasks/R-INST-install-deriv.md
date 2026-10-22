---
status: done
assignee: TrueCastle `01a16096-c00e-7404-8afe-e521a9bb7240` (R-INST first-pull per PARK a97a259; owner standing order supersedes rest-grant, pull issued by LoudDragon `01a15f82`)
type: tooling + read-only re-derivation (NO IDB writes; ships a tool mode + regression + the 74-row install-resolved verdict feed for wave-2b arbitration)
depends: Q-W2 Gate-A artifacts (obj/t087q/regen3.py base, wavedumps/qw2-gateA-*), T-084 sweep inputs (t064-postdump, t084-mint.json, byhost), ecx-flow pinned cache (md5 a68c22f3), harness items 35/42/44/49/50/52
---

# R-INST — INSTALL-MODE derivor edge-kind: the holder-install join (wave-2b unlock)

## Scope (PARK row verbatim intent)
regen3's caller-mode + family transitivity are committed; the missing edge-kind is WHO EMBEDS
WHAT AT WHICH FIELD (holder-install join). Deliver:
1. `install` mode — banked sibling tool (tools/r-inst-deriv.py importing ecx-flow Model, the
   "banked sibling" option; non-invasive to the shared tool while other lanes run it).
2. SELFTEST THAT SHIPS FIRST, baked before any verdict:
   S1 the +0x118 PAIR: 0x569790-class rows vs 0x569F10 must resolve to DIFFERENT (or PROVEN-SAME)
      component classes via install evidence — caller-mode's cls_0x569f10 answer for 0x569790 is
      exactly the failure this mode exists to fix or confirm.
   S2 known install positive from banked history: ModelInstance ctor -> 0x56E0B0 @ this+0x44
      (ecx-flow T2b) must be recovered by the install join independently.
   S3 Manager shape: ctor_0x4b5640 embeds _Tree at [this+0x48] (T-075/T-081 byte facts) — the
      derivor must see (host=GUI::Manager, disp=+0x48) without being told.
3. THE 74-ROW RUN: the 65 Gate-A HOLDs + 9 §2-vs-§3 collision rows re-derived under install-mode;
   per-row output: install-host / doc-host / caller-host / agreement verdict
   (ADOPT-BY-INSTALL, CONFIRM-DOC, CONTRADICTS-DOC->arbitration, NO-INSTALL-EVIDENCE).
   EpicRaven arbitrates with this list in hand (Q-W2b PARK row); nothing is minted by this row.

## Key semantics to settle with bytes (from PARK row's own framing)
EpicRaven's hosts-mode made the two Client/World rows DISTINGUISHABLE BY KEYS; assignment needs
the install edge. Open question the tool must answer honestly: is the doc's Client/World split at
+0x118 (a) two DISTINCT component classes each embedded at its holder's +0x118 (template/sibling
shape), or (b) ONE component class the doc split across hosts (doc error, feeds errata)? Evidence:
component-ctor's stored vft (item-49 `c7 06` pattern) per band; shared vft => same class => (b).

## Method
- install edge: call site S with ecx=(inecx,+d), owner C=S's live class-root, target T =>
  C embeds T's object at +d. Component grouping: rows sharing a component-ctor's vft (scan each
  ctor-band's `c7 01/06 imm` vft store; cluster members = vft table entries + same-key family).
- Host of row E: the install edges into E's COMPONENT (any member of its cluster), not into E
  alone — embed-family methods are rarely installed directly; their cluster's ctor is.
- Live names via t064-postdump injection + Q-W2a applied renames (re-read: the wave moved 99 rows;
  use CURRENT live names from the IDB? NO — this lane stays read-only on 8745 writes; reads via
  cached snapshot are fine, but for post-wave names use the committed wavedumps/qw2-wave2a-poststate.txt.)

## Deliverables
- tools/r-inst-deriv.py (selftest S1-S3 gated; refuses verdicts on red selftest per item 35).
- discoveries/2025-10-21-rinst-install-deriv.md (LEFT DIRTY): selftest output, the 74-row verdict
  table, the Client/World (a)/(b) answer with byte evidence, and the derivor-mode spec text for
  eventual ecx-flow graduation.
- Relay: verdict feed to LoudDragon + EpicRaven (Q-W2b arbitration input).

## Rules
Read-only: raw EXE + F3.json + wavedumps + pinned cache; zero IDB writes; zero product touches.

## Result (TrueCastle, 2026-10-22)
SELFTEST S1/S2/S3 ALL PASS -> verdicts released. S1 ANSWERED the open question with evidence:
ONE component class (cls_0x569f10 family) embedded ONLY by JE::World @+0x118, zero Client
embed-votes (doc's Client/World split = caller-mode artifact; wave-2a's minted Client field-load
rows vindicated); +0x108 trio same shape (cls_0x56ae70). S2: banked T2b edge recovered among 49
hosts of the generic init helper 0x56E0B0 (helper-vs-component discriminator). S3 BLIND:
(GUI::Manager,+0x48) recovered for 0x4C6640 — the five-wave shape is routine for the install join.
74-row feed: 13 ADOPT-BY-INSTALL (incl 0x5ad7b0 Map->AreaMap@+0x60; 2 PipBoy-embed rows
recommended to follow their already-minted cluster host w/ annotation, NOT re-home),
12 CONFIRM-DOC (safe wave-3 material), 16 CONTRADICTS/MULTI-HOST, 33 NO-EVIDENCE (honest silence).
COLLISION-9 RESOLVED BY EVIDENCE: 11-25 install hosts each = shared machinery -> §3 annotate-foreign
is right, §2 mint rows are the error; item-52 contradiction class closed without adjudication
preference. Deliverables: tools/r-inst-deriv.py (committed), feed wavedumps/rinst-74-verdict-feed.txt
(committed), discoveries/2025-10-21-rinst-install-deriv.md LEFT DIRTY. Zero IDB writes.
