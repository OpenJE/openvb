# 2025-10-20 — Clip-index naming machinery + animation cues (session 26)

Target: ORIGINAL_REFERENCE. Read-only, native IDA. Answers the long-standing "clip names"
queue item: the *mechanism* is fully static-verified; the *data* (517 prefix strings) is
runtime-filled BSS → handed to the reimpl as a one-liner dump.

## 1. `GameAnimationCache::ReadAnimations` (0x45d300, vft slot 4)

Iterates global clip indices; range = **0..516** (517 clips), or 416..516 when the
bool arg (`a3.byte0`) is set (a "creature-override?" secondary pass — same loop, tail
96 indices; candidate: gender/skeleton variants — UNVERIFIED).

Per index `i`:
```
name := TBL[i]                          ; 28-byte std::string records at 0x6F4170 (BSS,
                                        ;   filled at runtime — 517 entries)
file  := name + "_" + ("0" if i<10) + itoa(i) + ".gr2"      ; primary lookup
try open(file) via sub_59B050 (resource open)
if not found:  file2 := A3name[0..len-3] + "_" + <same>     ; fallback = caller's base name
               (A3 = caller-supplied resource name; last 3 chars stripped = ".gr2" ext cut)
               try again; still missing → byte_707DF0=0 flag path + continue
if found: Gfx_AnimationObject::LoadAnimationFileFromBuffer(obj, i, handle..., 10.0)
```
⇒ **clip index → resource name is `"<prefix-table>_<NN>.gr2"`**, GR2 = Granny model
(dependency `dependencies/granny2` in this repo matches). The 517 prefixes are populated
before this runs (settings/rht scan — writer TBD, see §3).

## 2. Animation CUES (the beat source for QueueAnimEvent!)

After load, per clip: parse key `"cueno"` (count); for k=1..count:
```
key "cue<k>"  → cue NAME  (string)
key "time<k>" → time ms   → seconds = ms*0.001
Gfx_AnimationObject::AddAnimationCue(obj, clipIdx i, animNo, timeSec, cueName)
```
Keys looked up in `cls_0x4d8d70`-style map (v27) = **the settings registry family**
(cls_0x56af40 from session 24-BONUS) ⇒ cue names/times are DATA-driven per anim.
⇒ This is where the session-8 tokens ("DM","RL","EP","UP","PU","FL","FR") and the
QueueAnimEvent ids ultimately anchor: anim events fire from cue timeline data.

## 3. What we still can't get statically (delegated, cheap)

1. **The 517 prefix strings** at 0x6F4170 — dump at runtime after startup
   (`for i in 0..517: read string at 0x6F4170+28*i` — std::string SSO layout: buf@+0,
   size@+0x10, cap@+0x14). Gives the complete clip-index→name table in one gdb/ReadProcess call.
2. Their filler (settings section? rht manifest) — search for stores to the 0x6F4170 range
   after the strings exist at runtime (writer is likely a "Animations" settings section).
3. The 416-range bool-arg meaning (call sites of ReadAnimations — reimpl can log).

## 4. Cross-session integration

- Session 8's clip indices (26,74,...,511) are indices into THIS 517-entry namespace; the
  "clip-name table" queue item is now mechanism-complete.
- Session 24's cls_0x56af40 rename ("Settings registry") confirmed again: cues read through
  it; the "cue%d/time%d" keys ride the same ini map the Network writer (session 24) uses.
