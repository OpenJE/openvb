# 2025-10-20 — Full settings/sys.ini surface: sections, keys, readers & writers (session 27)

Target: ORIGINAL_REFERENCE. Read-only. Sweep of every call site of the settings-registry API
(cls_0x56af40 sections via 0x4e0470; key-map find 0x4df070 / 0x433A20; key write 0x4dfeb0).
**This is the complete config contract** for the reimpl's sys.ini parser and openje's
settings layer.

## 1. Sections and keys (READ)

| section | keys (map-find @site) | consumer |
|---------|----------------------|----------|
| **[Mainmenu]** | `Map Name` @0x433d14; `Target X/Y/Z`, `Azimuth`, `Elevation`, `Distance`, `FOV`, `zNear`, `zFar` @0x460e97.. | menu 3D scene camera + startup map (the reimpl's startup flow keys ✓) |
| **[Server]** | `Start map` @0x46ef94, `Start map entry point` @0x46f014; + [Network-family] configuration/port/debug/rate/timeout @0x543b.. (JE::Network) | world start + network |
| **[World]** | `Seconds Per Round`, `Year`, `Month`, `Day`, `Hour`, `Minute`, `Second` @0x546c46.. | InitClock (session 23 §3.6) |
| **[Graphics]** | `DebugOctree`, `adapter`, `device`, `displaymode`, `fullscreen`, `refresh`, `mode32bpp`, `stats`, `clear red/green/blue`, `shadow type`, `enable lighting`, `enable bilinear filtering` @0x581d66.. (+ width/height @0x56b654 in display-mode enumeration) | video settings read |
| **[Audio]** | (section @0x591196 — key reads use a variant pattern; sweep Audio next if needed) | |
| **[HotKeys]** | section @0x599b14; builds `"Ctrl+"`/`"Shift+"` accelerator strings @0x599c09/33 | hotkey display/parse |
| **[Combat]** | section @0x5a9923; `mode` write @0x5a99af | combat settings (writes!) |
| anim-clip meta | `count`, `cue%d`, `time%d` @0x45dbd9.. | ReadAnimations cues (session 26 §2) |
| material/texture | `flags`, `texture`, `color`, `width`, `height`, `rate` @0x5234ed.. | Gfx material config |

## 2. Writers (SETTINGS SAVE) — key-set sites

* `[Network] configuration` (0x544300) + `[Server]/[Client] port/debug/rate/timeout` —
  Network ini writer (session 24 bonus).
* `[Graphics]`: adapter, device, displaymode, fullscreen, width, height, refresh, mode32bpp,
  stats, clear red/green/blue, shadow type, enable lighting, enable bilinear filtering,
  enable startup movies, vsync, enable wireframe, enable transparency (0x582cb5..0x5837f0)
  — the video-options SAVE path; **note: writer-set ⊃ reader-set** (`enable startup movies`,
  `vsync`, `enable wireframe`, `enable transparency` written; `width/height` read via the
  display-mode enumeration path) ⇒ those extra keys round-trip through a different reader
  (likely direct at startup) — enumerate their reader by reverse-xref if parity matters.
* `[Combat] mode` write @0x5a99af.

## 3. Adjacent finds from the sweep (leads, not yet chased)

* Scripting engine diagnostics seen in-line: `"Couldn't find variable %s"`,
  `"Couldn't find string %s"`, `'<Bad Token>'` @0x56c0xx — GameScripting variable/string
  tables (future combat-script work, session 21 §"scripted effects").
* Resource-path literals: `'\\cache\\'` @0x50d5a4 (cache dir!), `.wav` @0x52d9c4,
  `.b3d` @0x577a73 (model ext), `.gr2` (session 26), `Characters` + `.CRT` @0x429ee3/f
  (character resource folder!), `'Strings'` @0x4c5eef (the Strings resource the reimpl already
  loads), `'grp'` @0x49b093.

## 4. For the reimpl

sys.ini parity checklist = §1 read-set exactly; anything your parser ignores cannot affect
the original's behavior. §2 gives the save-file shape (the original REWRITES these keys —
parity: your writer should emit the same key set or diffs will look like corruption).
`[Mainmenu]` camera keys are consumed per-frame (0x460e97 in an update-shaped site —
candidates for live camera tweaking, test if unknown).
