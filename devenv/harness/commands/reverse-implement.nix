# ./devenv/harness/commands/reverse-implement.nix

{ lib, ... }:
let
  name = "reverse-implement";
  description = "Force implementation from the current or specified reverse-engineering target";
  prompt = lib.strings.removeSuffix "\n" ''
    Implement the current or specified reverse-engineering target `$ARGUMENTS`.

    Requirements:
    - Use stable summaries and docs as constraints.
    - Preserve binary-faithful behavior.
    - Report clearly if implementation is still too speculative to proceed.
  '';
  yaml = lib.strings.removeSuffix "\n" /* yaml */ ''
    description: ${description}
    agent: overseer
    subtask: true
  '';
in {
  opencode.commands.${name} = lib.strings.removeSuffix "\n" ''
    ---
    ${yaml}
    ---
    ${prompt}
  '';
}
