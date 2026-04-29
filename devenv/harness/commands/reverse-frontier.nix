# ./devenv/harness/commands/reverse-frontier.nix

{ lib, ... }:
let
  name = "reverse-frontier";
  description = "Continue the current unresolved reverse-engineering frontier for a target function";
  prompt = lib.strings.removeSuffix "\n" ''
    Continue the current unresolved reverse-engineering frontier for `$ARGUMENTS`.

    Requirements:
    - Treat the argument as an IDA-compatible function address or function name when provided.
    - Reuse any existing summaries and frontier state before descending further.
    - Continue autonomously until the next stable state or blocker is reached.
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
