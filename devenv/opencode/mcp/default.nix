# ./devenv/opencode/mcp/default.nix

{ ... }: {
  imports = [
    ./ida-pro.nix
    ./imhex.nix
    ./math.nix
    ./microsoft-learn.nix
    ./devenv.nix
  ];
}
