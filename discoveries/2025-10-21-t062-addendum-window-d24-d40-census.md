# 2025-10-21 — ADDENDUM to T-062: who writes/uses Window+0xD24 / +0xD40 (EpicRaven, flow census; banked by coordinator LoudDragon 01a15f82)

Source: EpicRaven's census on the md5-pinned flow cache (a68c22f3dd1c256ec9539a70e87cb434), names from
LIVE t060-postdump; two queries, no IDB access, no 8745 entry. Answers T-062's named-open
"who fills the +0xD24/+0xD40 cursor-sound name strings (absent from the sparse Window member map)".

## Verdicts
1. **BOTH offsets are std::string SUBOBJECTS.** The known string ctor (`cls_0x50db20::meth_0x401b20`
   string machinery) is called with `this = [ctor-this]+0xD24` and `+0xD40` inside
   `JE::GUI::Button::ctor_0x4a6ed0` at sites 0x4A6F5E / 0x4A6F6D — Window carries two string members
   there and every derived ctor constructs them. Shape settled WITHOUT booting anything.
2. **Writer census (direct [this+disp] stores):**
   - +0xD24 <- GUI::Picture (ctor store 0x4AA8D1; meth_0x4aaf10 @0x4AAF46/0x4AB0DE;
     virt_meth_0x4aab20 @0x4AAC0F/0x4AAC4F); GUI::Label ctor 0x4A9134, dtor 0x4A9033;
     JE::CSMenuInterface::ctor 0x568FE7.
   - +0xD40 <- GUI::Label dtor 0x4A9003; AreaMapWindow::ctor_0x422d40 store 0x422DA0;
     CSMenuInterface::ctor 0x56900D; sub_568970 @0x568A44.
3. **Consumer edges (+0xD40):** GUI::Button vfuncs call the UNNAMED trio 0x50B790/0x50B7F0/0x50B880 —
   same epoch family as cls_0x50b560 (T-057 §2: this+0x18 map members) — consistent with a
   sound/asset-manager lookup keyed by the string. Edit side: sub_59D9F0 / sub_59DEB0 pass
   this=+0xD24 into `JE::GUI::Edit::meth_0x4C2360`.

**Net**: cursor-SOUND name field = **Window+0xD40** (string; written by Label/CSMenu ctors, read
through the 0x50Bxxx manager trio); +0xD24 = the Picture/Edit-facing string.

## Downstream
- T-062 named-open (fillers) -> CLOSED by this census; T-056 BONUS-5 REDUCED: the field shape is no
  longer lockstep-worthy; what remains is only the trio-owner identity, which is statics work.
- The 0x50B790/7F0/880 trio ownership is the natural next hop for the T-062 surface owner
  (KeenStorm) — offered as their starting map, flow evidence attached.
