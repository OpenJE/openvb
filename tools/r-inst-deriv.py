#!/usr/bin/env python3
"""R-INST — INSTALL-MODE derivor (TrueCastle 01a16096, wave-2b unlock).

Edge-kind: the HOLDER-INSTALL join. A call site whose ecx = caller_this + disp (plus/inecx
shape) CONSTITUTES an embed edge: <owner-of-site> hosts <target's component> at +disp.
Row->cluster resolution (identity from T064-Era KEYS, labels from current live names):
  (1) stored-vft equality (item-49 c7 0x imm-vft ctor stores), (2) vft-table membership,
  (3) same-key family (t064-postdump keys). Components with no vft are identity-resolved by
  the cluster's install votes, flagged vft-absent.

Selftests ship BEFORE any verdict (item 35; the gates ARE the claims):
  S1 +0x118 pair: all five cls_0x569f10-family rows must resolve install-host JE::World @+0x118
     and no JE::Client embed-vote may exist for them — the tool ANSWERS one-class-vs-two rather
     than inheriting the doc's Client/World split.
  S2 banked T2b positive recovered: an install edge (owner endswith ModelInstance, disp 0x44)
     must exist for 0x56E0B0 among its multi-host votes (helper-target case: regression is the
     edge, not uniqueness).
  Read-only. Zero IDB writes. Exit 1 on any selftest failure (verdicts withheld on red gate).

INST-GRAD (2026-10-23): the Derivor bodies, S-gates and the 74-row feed were GRADUATED
VERBATIM into tools/ecx-flow.py as the `install-derived` mode (one engine, one shared
obj/t054/flow-<md5>.json cache, both gate suites under `selftest`). This file remains
for history and open links only; it is a delegation shim and no longer the arbitration
entry point. Approve any future edit through ecx-flow.py, not through this shim.
"""
import importlib.util, os, sys

REPO = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
spec = importlib.util.spec_from_file_location("ecxflow", os.path.join(REPO, "tools", "ecx-flow.py"))
mod = importlib.util.module_from_spec(spec); spec.loader.exec_module(mod)

def main():
    # Delegate verbatim: entry gains the gate union + --run74 verdict feed of the mode.
    argv_tail = sys.argv[1:]
    sys.argv = [sys.argv[0], "install-derived"] + argv_tail
    return mod.main()

if __name__ == "__main__":
    sys.exit(main())
