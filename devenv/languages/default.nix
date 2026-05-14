# ./devenv/languages/default.nix

{ ... }: {
  imports = [
    ./nix.nix
    ./cplusplus.nix
    ./clang-configs.nix
  ];
}
