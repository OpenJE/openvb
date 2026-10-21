# ./devenv/opencode/mcp/imhex.nix

{ config, ... }:

let
  # ImHex's own listen port is hardcoded to 31337 (see devenv/environment/openvb.nix for the
  # measurement), so this file and .pi/mcp.json point the client AT 31337 and the original's gdb
  # stub is the side that moved - to 31437. Never re-derive the number from a guess here.
  imhexHost = "127.0.0.1";
  imhexPort = config.env.OPENVB_IMHEX_MCP_PORT;
in
{
  opencode.mcp.imhex = {
    type = "local";
    enabled = true;
    timeout = 30000;

    environment = {
      IMHEX_HOST = imhexHost;
      IMHEX_PORT = imhexPort;
      PYTHONUNBUFFERED = "1";
    };

    # imhex-mcp-server ignores IMHEX_HOST/IMHEX_PORT: server.py reads only --host/--port, whose
    # defaults are "localhost"/31337. "localhost" resolves to ::1 first on this box while ImHex binds
    # IPv4-only, so even a live ImHex looked "Connection refused" and the MCP server exited during
    # initialize (pi: "imhex: failed: MCP connection closed"). Pass the flags explicitly.
    command = [
      "imhex-mcp-server"
      "--host"
      imhexHost
      "--port"
      imhexPort
    ];
  };
}
