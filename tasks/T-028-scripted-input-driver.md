---
status: open
assignee: HappyWolf (01a157ec)
depends: T-026 (a driver without something to observe is just a macro)
---

# T-028 — Drive the sim with a script, and map how input really enters the client

## Why
`f3-lockstep-debug` is manual and there is no input queue at all, so we cannot reproduce a frame
sequence on either side unattended — which is the prerequisite for any end-to-end parity claim. Split
the work so half of it is doable **today, with no exe**.

## Scope
1. **Recon (now, no build):** establish how input reaches gameplay state — the GUIMap/`GameClient`
   receive paths, what a click/hotkey turns into (Action queueing? direct method calls?), and where
   the queue is consumed. Cite by filename per item 15; write to `discoveries/`.
2. **Driver (host-side, now):** a scripted event source feeding the same seams a real input path
   would (spawn, move order, attack order, wait), consumed by `GameStateLoop`/`Host::` seams, taking a
   script file as input and producing T-026 digests.
3. **Deferred, named as deferred:** feeding the actual `F3.exe` needs a bootable exe (D3D8 decision)
   and is explicitly not in this row.

## Acceptance
- A script reliably reaches a combat round in the **host** sim, or the row reports the exact missing
  seam that stops it — a documented "cannot reach round N because X" is an accepted outcome here,
  since the missing pieces are knowledge, not coding.
- Same script + same seed produces identical digests across runs.
- Recon answers the question the ledger needs: is a scripted path through the input layer reachable at
  all in this slice, or does it require `RecvPacket` plumbing we have not ported?

## Refs
- `docs/notes/harness-items.md` 2 (deterministic driver recorded as unported)
- `discoveries/2025-10-20-gameclient-vft-demand-ui-factories.md`, `...f3-ui-command-handlers-window-slots.md`
