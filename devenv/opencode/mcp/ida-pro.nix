# ./devenv/opencode/mcp/ida-pro.nix

{ config, ... }:
let
  origBaseUrl = "http://127.0.0.1:${config.env.OPENVB_IDA_ORIG_MCP_PORT}/mcp?ext=dbg";
  reimplBaseUrl = "http://127.0.0.1:${config.env.OPENVB_IDA_REIMPL_MCP_PORT}/mcp?ext=dbg";
in {
  opencode.mcp.f3_exe_ida = {
    type = "remote";
    url = origBaseUrl;
    enabled = true;
  };

  opencode.mcp.openvb_exe_ida = {
    type = "remote";
    url = reimplBaseUrl;
    enabled = true;
  };
}
