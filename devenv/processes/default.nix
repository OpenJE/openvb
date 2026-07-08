# ./devenv/processes/default.nix

{ ... }: {
  imports = [
    ./docker-rootless.nix
    ./ida-headless.nix
    ./imhex-headless.nix
    ./f3-gdb-stubs.nix
  ];
}
