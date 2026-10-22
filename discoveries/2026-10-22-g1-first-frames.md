# 2026-10-22 — G1 POSITIVE: the first port-rendered frames exist (verdict note; PNG = artifacts/frames/)

**The artifact:** `artifacts/frames/2026-10-22-first-port-frame-800x600.png` — captured mid-run-2 via
`DISPLAY=:96 import -window root`, 800x600, histogram **464,000 px solid (255,0,255) + 16,000 px black**.
464,000 == 800x580 == the DXVK-reported swapchain client area: the magenta is the *device's own clear*,
presented through the real swapchain, not a canvas paint. The first frame of Van Buren's engine rendered
by its reimplementation, since neither existed.

**The run (two independent boots, replicated):** gated exe (HEAD ~ece5726-era), rig per
tasks/R-RIG-redquartz-rig-g1.md; run-2 reaped by timeout at 120s **rc=124 ALIVE, 13,800 frames
presented, zero fatal/err lines**. No shutdown-trace = killed running, not exited or faulted —
endless-window, the same posture T-067 recorded for the shipped exe under DXVK. Pre-flight datum for
the sitting: nothing between boot and 120s stops it.

**Trace line set (run-1, verbatim):** PHASE1 enter gate OK → Direct3DCreate8 SUCCEEDED pre-Startup →
PHASE2 opt-in, hwnd → GetAdapterDisplayMode hr=0 format=0x16 → CreateDevice OK, magenta armed →
F3::Main entered → presents 1..1200.

**Rig state:** Xorg :96 LEFT WARM (pid file /tmp/r-rig-redquartz/xorg.pid; runner
/tmp/r-rig-redquartz/g1-run.sh re-runs + writes g1-rc.txt/g1-trace2.log). Gotchas memorized by the
rig's keeper (engram #125): registry-not-env overrides, THREE dlls, lib32 ICD, no softpipe, and the
Modeline-bearing /tmp/t056-xorg.conf — NOT the incomplete /tmp/xorg-dummy.conf (documented, not
cleaned; likely SageBear's stall cause).
