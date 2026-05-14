# ./devenv/opencode/skill/lockstep-checkpointing.nix

{ lib, ... }:
let
  name = "lockstep-checkpointing";
  description = "workflow for paired original-vs-reimplementation debugger checkpoints, snapshots, and short lockstep stepping windows.";
  prompt = lib.strings.removeSuffix "\n" ''
    When driving both binaries side by side:
    - Prefer paired checkpoints over unbounded instruction stepping.
    - Use matched breakpoints or symbolic checkpoints first.
    - Compare instruction pointer, general registers, stack trace, and selected memory/global regions.
    - Escalate to short step windows only after checkpoint drift is observed.

    Rules:
    - Keep checkpoints narrow and evidence-backed.
    - Record the first stable drift point rather than narrating every single step.
    - Treat environment noise and unrelated initialization drift separately from target behavior drift.
  '';
  yaml = lib.strings.removeSuffix "\n" /* yaml */ ''
    name: ${name}
    description: ${description}
  '';
in {
  opencode.skills.${name} = lib.strings.removeSuffix "\n" ''
    ---
    ${yaml}
    ---
    ${prompt}
  '';
}
