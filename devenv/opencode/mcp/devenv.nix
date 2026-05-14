# ./devenv/opencode/mcp/devenv.nix

{ config, ... }: {
  # https://devenv.sh/mcp/
  opencode.mcp.devenv = {
    type = "local";
    command = [ "devenv" "mcp" ];
    environment = {
      DEVENV_ROOT = config.devenv.root;
    };
  };
}
