# ./devenv/harness/commands/reverse-compare.nix

{ lib, ... }:
let
  name = "reverse-compare";
  description = "Compare the original Van Buren demo against the rebuilt reimplementation for a target function, subsystem, or behavior.";
  prompt = lib.strings.removeSuffix "\n" ''
    Compare the original demo and rebuilt reimplementation for `$ARGUMENTS`.

    Requirements:
    - Start from the cheapest comparison level that can answer the question.
    - Escalate through structure, types, pseudocode, and runtime only when needed.
    - Report the first stable divergence point when possible.
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
