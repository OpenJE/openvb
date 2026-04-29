# ./devenv/tasks/default.nix

{ ... }: {
  imports = [
    ./build.nix
    ./check.nix
    ./run.nix
  ];
}
