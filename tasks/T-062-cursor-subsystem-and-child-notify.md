---
status: in-progress
assignee: KeenStorm (dispatched by LoudDragon `01a15f82`, 2026-10-21 — direct follow-on to T-061's named open items)
type: research (read-only on ida_original_f3; discoveries/ left DIRTY for coordinator checkpoint)
depends: T-061 (you decoded the facade; now decode the subsystem it fronts)
---

# T-062 — The cursor subsystem: GfxCursor object, engine cursor-manager, and Window child-notify targets

## Objective
T-061 surfaced a whole subsystem it explicitly did NOT decode. Finish the cursor/event picture so the
port can reproduce cursor behavior, and close the two open questions T-061 named as "adjacent, not this class."

## Scope
1. **GfxCursor object** — ctor `0x4c8100`, vft `0x67140C` (0xF8-byte object, `new` via `0x6054b0`): field
   layout, the `[obj vft+0](name)` loader (what does a cursor name DO — texture demand? the T-013 stem
   question recurs here for `GUI_BeamCursor.tga`/`GUI_DefaultCursor.tga`), and helper `0x4c7eb0(2,0,0)` shape.
2. **Engine cursor-manager** — reached `[[0x70BD90]+0x2C]+0xA4`; its vft+0x14 handler takes `(name,w,h,flag)`.
   Identify the manager class (0x70BD90 = engine session global — what class owns it?), decode vft+0x14
   (SetCursor?) and neighbours, and the `+0xA4` member meaning.
3. **Window child-notify targets** (T-061 §open): slot-1 of the CEngineInterface facade walks the
   iface `+0x28` list calling each `[[node+8] vft+0x30](float,float)`. Enumerate what `vft+0x30` resolves
   to across the Window-band vfts (the T-055 `4b28a0`/0x707E48 singleton family is adjacent) — is it a
   paint/resize/reposition notify? The float-accumulate semantics in slot-1 hints layout; confirm.
4. **Event-kind table refinement** (1/3/4/5/7) — T-056 is now producing a live boot; a render-side trace
   MAY be available soon. If so, cross-reference the shipped window-event dispatch to pin kind semantics;
   if not, keep it as a named open question (do NOT guess the table).

## Acceptance
`discoveries/` file (LEFT DIRTY): GfxCursor layout + name-loader verdict, cursor-manager class identity +
vft+0x14, the `vft+0x30` child-notify target set with a dispatch-census table, and an updated event-kind
status (pinned or explicitly still-open). Read-only honored; EA-anchored; names-as-locators.
Ping [DISCOVERY-BATCH ready].
