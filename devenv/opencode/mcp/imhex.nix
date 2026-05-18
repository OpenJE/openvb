# ./devenv/opencode/mcp/imhex.nix

{ config, ... }:

let
  imhexPort = config.env.OPENVB_IMHEX_MCP_PORT or "31337";
in
{
  opencode.mcp.imhex = {
    type = "local";
    enabled = true;
    timeout = 30000;

    environment = {
      IMHEX_HOST = "127.0.0.1";
      IMHEX_PORT = imhexPort;
      PYTHONUNBUFFERED = "1";
    };

    command = [
      "imhex-mcp-server"
    ];
  };
}
