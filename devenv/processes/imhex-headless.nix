# ./devenv/processes/imhex-headless.nix
#
# Headless ImHex process with MCP plugin.
# Runs ImHex under xvfb-run (virtual framebuffer) so it can start
# without a display. The MCP plugin's TCP server listens on port 31339.

{ config, pkgs, ... }: {
  processes.imhex-headless = {
    exec = ''
      mkdir -p "$HOME/.config/imhex"
      if [ ! -f "$HOME/.config/imhex/settings.json" ]; then
        echo '{"network": {"enabled": true, "port": ${config.env.OPENVB_IMHEX_MCP_PORT}}}' > "$HOME/.config/imhex/settings.json"
      fi
      exec ${pkgs.xorg.xvfb}/bin/xvfb-run ${pkgs.imhex}/bin/imhex
    '';
    cwd = config.git.root;
    ready.exec = "${pkgs.netcat-openbsd}/bin/nc -z 127.0.0.1 ${config.env.OPENVB_IMHEX_MCP_PORT}";
  };
}