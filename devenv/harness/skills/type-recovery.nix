# ./devenv/harness/skills/type-recovery.nix

{ lib, ... }:
let
  name = "type-recovery";
  description = "conservative workflow for recovering function signatures, stack variables, structs, enums, and field meanings from ida evidence.";
  prompt = lib.strings.removeSuffix "\n" ''
    Use this skill when type uncertainty blocks behavioral understanding.

    Rules:
    - Prefer minimal, evidence-backed type changes.
    - Use stack frame, type, struct, and xref evidence.
    - Distinguish safe assertions from tentative guesses.
    - Record any unresolved ambiguity instead of forcing a type.
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
