# ./devenv/harness/skills/database-targeting.nix

{ lib, ... }:
let
  name = "database-targeting";
  description = "workflow for consistently opening and saving the original and reimplementation IDA databases in the OpenVB headless harness.";
  prompt = lib.strings.removeSuffix "\n" ''
    Database conventions for this repository:
    - Original binary: `OPENVB_ORIG_EXE`
    - Original database copy: `OPENVB_ORIG_IDB` (currently `./F3.exe.i64`)
    - Reimplementation binary: `OPENVB_REIMPL_EXE`
    - Reimplementation traced binary: `OPENVB_REIMPL_TRACE_EXE`
    - Canonical reimplementation database path: `OPENVB_REIMPL_IDB`

    Rules:
    - On the original side, prefer opening `OPENVB_ORIG_IDB` when it exists.
    - On the reimplementation side, prefer opening `OPENVB_REIMPL_IDB` when it exists.
    - If the reimplementation database does not exist yet, open `OPENVB_REIMPL_EXE` and later save to `OPENVB_REIMPL_IDB`.
    - Treat session state as ephemeral; verify the active database explicitly with `idalib_current` or `idalib_list` before assuming context.
  '';
  yaml = lib.strings.removeSuffix "\n" /* yaml */ ''
    name: ${name}
    description: ${description}
  '';
in {
  opencode.skills.${name} = lib.strings.removeSuffix "\n" ''
    ---
    ${yaml}
    ---
    ${prompt}
  '';
}
