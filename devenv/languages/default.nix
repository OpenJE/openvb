# ./devenv/languages/default.nix

{ ... }: {
  imports = [
    ./nix.nix
    ./cpp.nix
  ];
}
