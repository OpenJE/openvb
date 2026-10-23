# X-PUSH successor band — full-set SHARING sweep + annotation receipt (splitseat, 2026-10-23)

Claim: the X-PUSH (T-084) closure's successor — "remaining 62/82 SHARING annotations
regenerate from the committed machine across the remaining 609 classes" — is EXECUTED.
Result superseded the estimate: a fresh full-set sweep found MORE rows than the T-084
batch reported, and the band is now closed end-to-end.

## Method (honest regen, committed machine only)
1. Live IDB census: 1,721 fns still `cls_`-anonymous (py_eval enumeration, written to
   /tmp/anon-eas.json via the server; census: 10,173 fns / 7,690 named).
2. `tools/ecx-flow.py share-check --addrs` over the full anonymous set in 5 chunks
   (350 fns each, ~18s/chunk; machine reads the PE + F3.json, NO IDB writes).
   Gate: selftest green (item-35 rule); verdicts live.
3. Sweep result: **109 unique SHARING fns** (vs T-084 batch's 82 — the batch ran on
   tier-2-era state; tier-2 renames since moved some fns to SHARING as their call-site
   fans revealed cross-class field access).

## Applied (read-back verified)
- **86 new annotations**: `long-tail helper epoch per T-084 (full-set sweep 2026-10-23);
  owner = call-site fan, not membership; key=<cls_0x...>` — key = the fn's CURRENT
  live class label (survives rename waves; law: land-last).
- 20 already annotated by the X-PUSH wave (a1ae175) — verified still present, untouched.
- 3 deliberately skipped — they carry STRONGER evidence from other lanes and the
  sweep's SHARING verdict is superseded by those comments:
  - 0x40a220 `labelled helper-noise (batch-2/3) — two-SSO-string init…`
  - 0x4395a0 `embeds@CEngineInterface (R36-e composite; base-vs-member ope…`
  - 0x548d10 `T-033 b5 labelled tail-alias: DB-attributed to the Gfx_AnimM…`

## Read-back
106/109 carry the sweep annotation exactly; the 3 skips verified unchanged.

## Save
save_database proven 19:06:01Z (ledger artifacts/ledgers/ida_original_f3-save-2026-10-23T19-06-01-133Z.jsonl);
10,173 fns / 7,690 named unchanged (comments-only pass, rename-tier purity held).

## Tooling note (same sitting)
`ida.ts` evalfile action landed (commit 5d2597e) — durable fix for the payload
corruption class (wrapper arg envelope lossy for quoted payloads + hand-copied hex
error-prone). Applied THIS band through the raw MCP py_eval path (lossless, quoted
payload applied in one shot, 86/86 read back). evalfile activates on next reload.
