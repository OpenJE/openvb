# ./devenv/harness/agents/default.nix

{ ... }: {
  imports = [
    ./overseer.nix
    ./pathfinder.nix
    ./comparator.nix
    ./decompiler.nix
    ./idb-curator.nix
    ./typesmith.nix
    ./scribe.nix
    ./reconstructor.nix
    ./auditor.nix
  ];
}
