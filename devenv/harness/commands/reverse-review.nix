# ./devenv/harness/commands/reverse-review.nix

{ lib, ... }:
let
  name = "reverse-review";
  description = "Force an evidence and readiness review for the current or specified reverse-engineering target";
  prompt = lib.strings.removeSuffix "\n" ''
    Review the current or specified reverse-engineering target `$ARGUMENTS`.

    Requirements:
    - Check whether the current findings are evidence-backed.
    - Identify unsupported claims, unresolved blockers, and implementation risk.
    - Recommend whether the workflow should continue, document, implement, or stop.
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
