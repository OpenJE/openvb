# ./devenv/harness/mcp-servers/devenv.nix

{ config, ... }:
let
  devenvRoot = config.devenv.root;
  sharedEnv = {
    DEVENV_ROOT = devenvRoot;
  };
in {
  # https://devenv.sh/mcp/
  opencode.mcp.devenv = {
    type = "local";
    command = [ "devenv" "mcp" ];
    environment = sharedEnv;
  };
}
