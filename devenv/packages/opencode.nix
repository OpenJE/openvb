# ./devenv/packages/opencode.nix

{ inputs, pkgs, ... }: {
  packages = [
    inputs.opencode.legacyPackages.${pkgs.stdenv.hostPlatform.system}.opencode
  ];
}
