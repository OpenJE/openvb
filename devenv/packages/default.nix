# ./devenv/packages/default.nix

{ ... }: {
  imports = [
    ./openvb.nix
    ./codegraph.nix
  ];
}
