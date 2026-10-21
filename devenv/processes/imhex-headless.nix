{ config, pkgs, ... }:

let
  imhexNetworkHost = "127.0.0.1";
  # ImHex's listen port is hardcoded to 31337 inside ImHex itself - this value is a *description* of
  # that fact, kept as a variable so the wait loop, the readiness probe and the MCP client all read one
  # number. Attempting to move ImHex off 31337 (the earlier 31347 here) is what produced the restart
  # loop: the wait never succeeded, the 120 s timeout killed ImHex, and process-compose restarted it.
  # See devenv/environment/openvb.nix for the measurement; the original's gdb stub moved to 31437.
  imhexNetworkPort = config.env.OPENVB_IMHEX_MCP_PORT;

  # Owner-verified readiness, same discipline as devenv/processes/f3-gdb-stubs.nix: `nc -z` (or
  # ready.exec = true) reports a green for any listener, which is how a dead stub was shipped as
  # "ready" for a whole investigation. Here the owner must be ImHex, not wineserver/winedbg.
  imhexReady = pkgs.writeShellScriptBin "imhex-network-ready" ''
    port="''${1:?usage: imhex-network-ready <port>}"
    "${pkgs.iproute2}/bin/ss" -H -ltnp "sport = :''${port}" \
      | "${pkgs.gnugrep}/bin/grep" -Eq "ImHex|imhex"
  '';

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
          echo "Timed out waiting for ImHex Network Interface on port ${imhexNetworkPort}." >&2
          echo "ImHex hardcodes 31337; if this port differs, the wait can never succeed." >&2
          echo "Who owns the port:" >&2
          ss -ltnp "sport = :${imhexNetworkPort}" >&2 || true
          ss -ltnp >&2 || true
          exit 1
        fi

        sleep 1
      done
    '';
  };
in
{
  # Informational only: imhex-mcp-server does NOT read these - it takes --host/--port CLI flags.
  # The MCP entries in devenv/opencode/mcp/imhex.nix and .pi/mcp.json pass the flags explicitly.
  env.IMHEX_HOST = imhexNetworkHost;
  env.IMHEX_PORT = imhexNetworkPort;

  processes.imhex = {
    exec = "${runImHex}/bin/run-imhex-mcp";
    cwd = config.git.root;
    ready.exec = "${imhexReady}/bin/imhex-network-ready ${imhexNetworkPort}";
  };
}
