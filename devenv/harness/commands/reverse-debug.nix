# ./devenv/harness/commands/reverse-debug.nix

{ lib, ... }:
let
  name = "reverse-debug";
  description = "Force a runtime-debugging phase for the current or specified target";
  prompt = lib.strings.removeSuffix "\n" ''
    Investigate `$ARGUMENTS` with runtime debugging.

    Requirements:
    - Prefer bounded checkpoints over unbounded stepping.
    - Use debugger evidence to confirm or reject specific hypotheses.
    - Report any debugger preconditions or attach blockers explicitly.
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
