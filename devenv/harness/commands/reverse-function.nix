# ./devenv/harness/commands/reverse-function.nix

{ lib, ... }:
let
  name = "reverse-function";
  description = "Start or resume the full autonomous reverse-engineering workflow from an IDA function address or function name";
  prompt = lib.strings.removeSuffix "\n" ''
    Start or resume the full autonomous reverse-engineering workflow for `$ARGUMENTS`.

    Target format:
    - Accept one IDA-compatible function address or function name.
    - Valid examples include `0x401000`, `401000`, `sub_401000`, or a resolved IDA function name.

    Requirements:
    - Resolve the target through IDA before traversal begins.
    - Use the resolved function address as the canonical identity internally.
    - Continue autonomously through analysis, review, documentation, implementation, and verification when confidence is high enough.
    - Only stop when blocked, ambiguous, unsafe to continue, or complete.
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
