{ config, pkgs, ... }:

let
  imhexNetworkHost = "127.0.0.1";
  imhexNetworkPort = "31337";

  runImHex = pkgs.writeShellApplication {
    name = "run-imhex-mcp";

    runtimeInputs = with pkgs; [
      coreutils
      gnugrep
      iproute2
    ];

    text = ''
      set -euo pipefail

      is_imhex_network_listening() {
        ss -H -ltn "sport = :${imhexNetworkPort}" | grep -q .
      }

      echo "Starting imhexMCP ImHex GUI..." >&2

      imhex &
      imhex_pid="$!"

      cleanup() {
        kill "$imhex_pid" 2>/dev/null || true
      }

      trap cleanup EXIT INT TERM

      echo "Waiting for ImHex Network Interface on ${imhexNetworkHost}:${imhexNetworkPort}" >&2

      for i in $(seq 1 120); do
        if ! kill -0 "$imhex_pid" 2>/dev/null; then
          echo "ImHex exited before the Network Interface became ready." >&2
          wait "$imhex_pid" || true
          exit 1
        fi

        if is_imhex_network_listening; then
          echo "ImHex Network Interface is ready on ${imhexNetworkHost}:${imhexNetworkPort}" >&2
          wait "$imhex_pid"
          exit "$?"
        fi

        if [ "$i" -eq 120 ]; then
          echo "Timed out waiting for ImHex Network Interface." >&2
          ss -ltnp >&2 || true
          exit 1
        fi

        sleep 1
      done
    '';
  };
in
{
  env.IMHEX_HOST = imhexNetworkHost;
  env.IMHEX_PORT = imhexNetworkPort;

  processes.imhex = {
    exec = "${runImHex}/bin/run-imhex-mcp";
    cwd = config.git.root;
    ready.exec = "${pkgs.coreutils}/bin/true";
  };
}
