# ./devenv/harness/commands/reverse-curate-idb.nix

{ lib, ... }:
let
  name = "reverse-curate-idb";
  description = "Apply stable names, comments, and type information back into the repo-specific IDA database copy";
  prompt = lib.strings.removeSuffix "\n" ''
    Curate the reimplementation IDA database for `$ARGUMENTS`.

    Requirements:
    - Only apply stable, evidence-backed metadata.
    - Save the curated database after meaningful changes.
    - Do not patch bytes or perform arbitrary IDAPython execution.
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
