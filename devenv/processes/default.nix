# ./devenv/processes/default.nix

{ ... }: {
  imports = [
    ./ida-headless.nix
    ./imhex-headless.nix
    ./f3-gdb-stubs.nix
  ];
}
