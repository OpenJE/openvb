# ./devenv/opencode/skill/default.nix

{ ... }: {
  imports = [
    ./ida-callgraph-workflow.nix
    ./comparison-ladder.nix
    ./database-targeting.nix
    ./function-reconstruction.nix
    ./lockstep-checkpointing.nix
    ./type-recovery.nix
    ./engine-game-boundary.nix
    ./docs-sync-workflow.nix
    ./binary-faithfulness.nix
    ./implementation-alignment.nix
  ];
}
