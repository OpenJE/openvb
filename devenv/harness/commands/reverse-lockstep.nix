# ./devenv/harness/commands/reverse-lockstep.nix

{ lib, ... }:
let
  name = "reverse-lockstep";
  description = "Drive the original demo and rebuilt reimplementation side by side through paired debugger checkpoints";
  prompt = lib.strings.removeSuffix "\n" ''
    Run original-versus-reimplementation lockstep debugging for `$ARGUMENTS`.

    Requirements:
    - Use the original and rebuilt debugger contexts together.
    - Compare checkpoints, snapshots, and short stepping windows.
    - Report the first stable drift point instead of raw step spam.
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
