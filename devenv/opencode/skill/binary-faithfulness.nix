# ./devenv/opencode/skill/binary-faithfulness.nix

{ lib, ... }:
let
  name = "binary-faithfulness";
  description = "rules for keeping analysis, documentation, and implementation aligned with observed binary behavior rather than aesthetic rewrites.";
  prompt = lib.strings.removeSuffix "\n" ''
    Prefer binary-faithful behavior over cleanup-oriented rewrites.

    Rules:
    - Do not treat nicer design as evidence.
    - Preserve observed side effects, call order, and behavior when known.
    - Avoid implementing speculative semantics as confirmed behavior.
    - When certainty is low, record the uncertainty instead of flattening it away.
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
