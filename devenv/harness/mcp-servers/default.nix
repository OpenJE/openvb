# ./devenv/harness/mcp-servers/default.nix

{ ... }: {
  imports = [
    ./devenv.nix
    ./ida-pro.nix
    ./math.nix
  ];
}
