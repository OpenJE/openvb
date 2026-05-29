# ./modules/packages/mcp-nixos.nix

{ inputs, pkgs, ... }: {
  packages = [
    inputs.codegraph.packages.${pkgs.stdenv.hostPlatform.system}.codegraph
  ];
}
