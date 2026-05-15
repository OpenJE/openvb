# ./devenv/opencode/mcp/math.nix

{ ... }: {
  opencode.mcp.microsoft = {
    type = "remote";
    url = "https://learn.microsoft.com/api/mcp";
    enabled = true;
  };
}
