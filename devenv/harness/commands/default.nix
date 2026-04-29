# ./devenv/harness/commands/default.nix

{ ... }: {
  imports = [
    ./reverse-function.nix
    ./reverse-frontier.nix
    ./reverse-cycle.nix
    ./reverse-compare.nix
    ./reverse-debug.nix
    ./reverse-lockstep.nix
    ./reverse-curate-idb.nix
    ./reverse-sync-docs.nix
    ./reverse-implement.nix
    ./reverse-review.nix
  ];
}
