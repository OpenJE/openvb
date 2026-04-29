# ./devenv/harness/commands/reverse-sync-docs.nix

{ lib, ... }:
let
  name = "reverse-sync-docs";
  description = "Force documentation synchronization for the current or specified reverse-engineering target";
  prompt = lib.strings.removeSuffix "\n" ''
    Synchronize documentation for `$ARGUMENTS`.

    Requirements:
    - Use stable findings only.
    - Preserve the existing docs/f3 structure and terminology.
    - Report if the current analysis is still too unstable for documentation.
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
