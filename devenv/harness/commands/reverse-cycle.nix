# ./devenv/harness/commands/reverse-cycle.nix

{ lib, ... }:
let
  name = "reverse-cycle";
  description = "Analyze a recursive or cyclic function region as one bounded reverse-engineering unit";
  prompt = lib.strings.removeSuffix "\n" ''
    Analyze `$ARGUMENTS` as a recursive or cyclic function region.

    Requirements:
    - Resolve the target through IDA first.
    - Treat the recursive region as one bounded unit.
    - Continue autonomously through review, docs, implementation, and verification when safe.
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
