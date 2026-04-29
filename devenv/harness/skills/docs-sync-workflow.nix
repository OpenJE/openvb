# ./devenv/harness/skills/docs-sync-workflow.nix

{ lib, ... }:
let
  name = "docs-sync-workflow";
  description = "workflow for promoting stable reverse-engineering findings into docs/f3 while keeping confirmed findings separate from hypotheses and open questions.";
  prompt = lib.strings.removeSuffix "\n" ''
    When updating docs:
    - Preserve existing docs/f3 structure and links.
    - Prefer additive updates over broad rewrites.
    - Record confirmed findings clearly.
    - Label hypotheses and unresolved items explicitly.
    - Keep documentation useful for future implementation work.
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
