# ./devenv/processes/imhex-headless.nix
#
# ImHex GUI-only process. Starts ImHex and waits for the Network Interface
# to be listening on port 31337. The MCP server is managed separately by
# OpenCode's opencode.mcp.imhex configuration.

{ config, pkgs, ... }:
let
  imhexNetworkPort = "31337";

  makeImHexHeadlessExec = /* bash */ ''
    set -euo pipefail

    is_imhex_network_listening() {
      ${pkgs.iproute2}/bin/ss -H -ltn "sport = :${imhexNetworkPort}" | grep -q .
    }

    printf '%s\n' "Starting normal visible ImHex GUI..." >&2

    # Uses patched ImHex from your devenv packages/PATH.
    imhex &
    imhex_pid=$!

    cleanup() {
      if [ -n "''${imhex_pid:-}" ]; then
        kill "$imhex_pid" 2>/dev/null || true
      fi
    }
    trap cleanup EXIT INT TERM

    printf 'Waiting for ImHex Network Interface on 127.0.0.1:${imhexNetworkPort}\n' >&2

    for i in $(seq 1 120); do
      if ! kill -0 "$imhex_pid" 2>/dev/null; then
        printf '%s\n' "ImHex exited before opening the Network Interface." >&2
        wait "$imhex_pid" || true
        exit 1
      fi

      if is_imhex_network_listening; then
        printf 'ImHex Network Interface is listening on 127.0.0.1:${imhexNetworkPort}\n' >&2
        break
      fi

      if [ "$i" -eq 120 ]; then
        printf '%s\n' "Timed out waiting for ImHex Network Interface." >&2
        ${pkgs.iproute2}/bin/ss -ltnp || true
        exit 1
      fi

      sleep 1
    done

    # Keep the process alive as long as ImHex is running.
    wait "$imhex_pid"
  '';
in {
  processes.imhex-headless = {
    exec = makeImHexHeadlessExec;
    cwd = config.git.root;

    # ImHex is ready when the Network Interface port is listening.
    ready.exec = "${pkgs.coreutils}/bin/true";
  };
}