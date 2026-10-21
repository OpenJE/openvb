# ./devenv/opencode/mcp/imhex.nix

{ config, ... }:

let
  # no "31337" fallback: a default that collides with the original's gdb port is how this
  # bug came back after being fixed once.
  imhexPort = config.env.OPENVB_IMHEX_MCP_PORT;
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
