# ./devenv/opencode/mcp/math.nix

{ ... }: {
  # // https://github.com/EthanHenrickson/math-mcp
  opencode.mcp.math = {
    type = "local";
    enabled = true;
    command = [
      "math-mcp"
    ];
  };
}
