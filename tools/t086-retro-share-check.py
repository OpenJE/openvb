#!/usr/bin/env python3
"""T-086: retroactive share-check across every landed mint (HappyStorm 01a16092).
Read-only: pinned cache + committed wavedumps + frozen export. No writes anywhere."""
import json, importlib.util, os, sys

REPO = "/mnt/shanes-ssd-1tb/code/shane/openvb"
spec = importlib.util.spec_from_file_location("ecxflow", os.path.join(REPO, "tools", "ecx-flow.py"))
mod = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mod)

EXE = os.environ["OPENVB_ORIG_EXE"]
F3 = os.path.join(REPO, "artifacts", "F3.json")
D = lambda f: json.load(open(os.path.join(REPO, "discoveries", "wavedumps", f)))

def dump_map(d):
    return {int(ea, 16): nm for ea, nm in zip(d["ea"], d["name"])}

pred = dump_map(D("t055-predump.json"))   # state after b1..b6
p55 = dump_map(D("t055-predump.json"))
q55 = dump_map(D("t055-postdump.json"))
p64 = dump_map(D("t064-predump.json"))
q64 = dump_map(D("t064-postdump.json"))

# export anonymous EAs
f3 = json.load(open(F3))
export_named, export_anon_eas = {}, set()
for key, cls in f3["structures"].items():
    for ea in (cls.get("methods") or {}):
        v = int(ea, 16)
        if key.startswith("cls_"):
            export_anon_eas.add(v)
        else:
            export_named[v] = key

def live_label(nm):
    if "::" in nm:
        return nm.rsplit("::", 1)[0]
    return nm

def anon_label(lab):
    return (lab.startswith("cls_") or lab.startswith("sub_") or lab.startswith("j_")
            or lab.startswith("call_") or "_with_" in lab
            or not ("::" in lab))

def vendor_label(lab):
    return (lab.startswith("std::") or lab.startswith("ATL::")
            or lab.startswith("boost::") or "net::" in lab)

def group_diff(before, after):
    bands = {}
    for ea, an in after.items():
        bn = before.get(ea)
        if bn == an:
            continue
        lab = live_label(an)
        if anon_label(lab):
            continue
        # landed mint iff pre-state was anonymous (cls_/sub_/j_ or absent-with-no-name)
        if bn is None or anon_label(live_label(bn)):
            bands.setdefault(lab, []).append(ea)
    return bands

def is_export_anon(ea):
    # named already in 2024 export => not a board mint even if renamed live
    return ea in export_anon_eas or ea not in export_named

bands = {}
vendor = {}
# b2..b6 survivors = export-anon EAs renamed in the post-b6 live state
bb = {}
for ea, nm in pred.items():
    lab = live_label(nm)
    if anon_label(lab):
        continue
    if not is_export_anon(ea):
        continue
    if vendor_label(lab):
        vendor.setdefault(lab, []).append(ea)
        continue
    bb.setdefault(lab, []).append(ea)
bands["b2-b6-survivors"] = bb

t055 = {}
for lab, eas in group_diff(p55, q55).items():
    if vendor_label(lab):
        vendor.setdefault(lab, []).extend(eas)
        continue
    t055.setdefault(lab, []).extend(eas)
bands["T-055"] = t055

t064 = {}
for lab, eas in group_diff(p64, q64).items():
    if vendor_label(lab):
        vendor.setdefault(lab, []).extend(eas)
        continue
    t064.setdefault(lab, []).extend(eas)
bands["T-064"] = t064

# queued bands (not yet applied): from frozen export + accepted docs
f3cls = f3["structures"]
def cls_eas(key, exclude=()):
    return [int(ea, 16) for ea in (f3cls.get(key, {}).get("methods") or {}) if int(ea, 16) not in exclude]

t073_A = cls_eas("cls_0x490150", exclude=(0x4915E0,))  # the nine
t073_A2 = [0x4950D0, 0x4903E0, 0x491F90]
B_rejected = [0x434120, 0x434250, 0x4342C0, 0x434760, 0x434910]
t075 = cls_eas("cls_0x4b28a0", exclude=(0x619B38,))
t073_B_band = cls_eas("cls_0x4fc0e0")

queued = {
    "T-073[A] Gfx_RenderManager ext (9)": t073_A,
    "T-073[A2] composite (ctor_0x4950D0+cls_0x4903e0 x2)": t073_A2,
    "T-073[B] cls_0x4fc0e0 (annotated, no mint)": t073_B_band,
    "T-064[B]-rejected (expected SHARING)": B_rejected,
    "T-075 queued GUI::Manager (cls_0x4b28a0, -0x619B38)": t075,
}

m = mod.Model(EXE, F3)
m.build(cache=True)

out = []
out.append("### LANDED bands (from on-disk wave-dump diffs; per class label) ###")
for wave, bl in bands.items():
    out.append("\n=== WAVE %s ===" % wave)
    for lab in sorted(bl):
        eas = sorted(bl[lab])
        rep = m.share_check_report(eas, "%s [%s] n=%d" % (lab, wave, len(eas)))
        verdict = [l for l in rep.splitlines() if l.startswith("VERDICT")][0]
        out.append("\nBAND %s (%d fns): %s" % (lab, len(eas), verdict))
        out.append(rep)
out.append("\n=== VENDOR/foreign-labelled bands (T-060/T-057 exclusion tier — reported, NOT flip-listed) ===")
for lab in sorted(vendor):
    eas = sorted(vendor[lab])
    rep = m.share_check_report(eas, "%s [vendor] n=%d" % (lab, len(eas)))
    verdict = [l for l in rep.splitlines() if l.startswith("VERDICT")][0]
    out.append("\nBAND %s (%d fns): %s" % (lab, len(eas), verdict))
out.append("\n=== QUEUED bands (not yet applied) ===")
for lab, eas in queued.items():
    rep = m.share_check_report(eas, lab)
    verdict = [l for l in rep.splitlines() if l.startswith("VERDICT")][0]
    out.append("\nBAND %s (%d fns): %s" % (lab, len(eas), verdict))
    out.append(rep)

print("\n".join(out))
