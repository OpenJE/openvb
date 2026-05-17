# ./devenv/opencode/mcp/imhex.nix
#
# ImHex MCP server - AI-powered binary analysis via ImHex hex editor.
#
# Manual launch:
#   1. Run imhex (GUI) and enable Network Interface in Settings → General
#   2. The MCP server will connect to ImHex on port 31337
#
# Headless launch:
#   devenv process start imhex-headless

{ config, ... }: {
  opencode.mcp.imhex = {
    type = "local";
    enabled = true;
    command = [
      "imhex-mcp-server"
      "--host" "127.0.0.1"
      "--port" config.env.OPENVB_IMHEX_MCP_PORT
      "--max-retries" "0"
    ];
  };
}