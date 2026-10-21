# wavedumps — per-wave IDB census dumps

`t055-predump.json` = the PRE-WRITE function dump taken by CalmRaven at T-055 wave start, read-only,
against `ida_original_f3`:8745 while `artifacts/F3.exe.i64` mtime was **14:26:04** (the b6 end-of-wave save).
Baseline counts (game region 0x401000..0x5FD000, T-029 denominator 8,876): **T2 (cls_)=2,046 = 23.05%** —
bit-exact reproduction of the registered b6 post-value. This is the writer-slot anchor for EpicRaven's
offline dump-stale boundary: any analysis dump taken after the first T-055 IDB write is stale vs this state.
