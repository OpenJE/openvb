# ./devenv/opencode/mcp/ida-pro.nix

{ config, ... }:
let
  origBaseUrl = "http://127.0.0.1:${config.env.OPENVB_IDA_ORIG_MCP_PORT}/mcp?ext=dbg";
  reimplBaseUrl = "http://127.0.0.1:${config.env.OPENVB_IDA_REIMPL_MCP_PORT}/mcp?ext=dbg";
in {
  opencode.mcp.ida_original_f3 = {
    type = "remote";
    url = origBaseUrl;
    enabled = true;
  };

  opencode.mcp.ida_reimplementation_openvb = {
    type = "remote";
    url = reimplBaseUrl;
    enabled = true;
  };
}
