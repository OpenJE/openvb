# ./devenv/processes/imhex-mcp.nix

{ config, pkgs, ... }:
let
  localPackages = import ../packages/local.nix { inherit pkgs; };

  serverBin = "${localPackages.imhex-mcp-server}/bin/imhex-mcp-server";

  # ImHex Network Interface port.
  # This is what imhex-mcp-server connects to.
  imhexNetworkPort = "31337";

  makeImHexMcpExec = /* bash */ ''
    set -euo pipefail

    # Symlink Van Buren patterns into ImHex user includes path.
    # ImHex resolves `import je.xxx` from ~/.local/share/imhex/includes/je/
    patternsDir="${config.git.root}/modules/patterns/includes/je"
    imhexIncludesDir="$HOME/.local/share/imhex/includes"
    imhexJeLink="$imhexIncludesDir/je"

    if [ -d "$patternsDir" ]; then
      mkdir -p "$imhexIncludesDir"
      ln -sfn "$patternsDir" "$imhexJeLink"
      printf 'ImHex patterns symlinked: %s -> %s\n' "$imhexJeLink" "$patternsDir" >&2
    else
      printf 'WARNING: Van Buren patterns directory not found at %s\n' "$patternsDir" >&2
      printf '  Run: git submodule update --init modules/patterns\n' >&2
    fi

    is_imhex_network_listening() {
      ${pkgs.iproute2}/bin/ss -H -ltn "sport = :${imhexNetworkPort}" | grep -q .
    }

    printf '%s\n' "Starting normal visible ImHex GUI..." >&2

    # Uses patched ImHex from your devenv packages/PATH.
    imhex &
    imhex_pid=$!

    cleanup() {
      if [ -n "''${server_pid:-}" ]; then
        kill "$server_pid" 2>/dev/null || true
      fi
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

      # Do not use nc here; ss checks the listener without connecting to it.
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

    printf 'Starting imhex-mcp-server connected to ImHex on 127.0.0.1:${imhexNetworkPort}\n' >&2

    "${serverBin}" \
      --host 127.0.0.1 \
      --port ${imhexNetworkPort} &
    server_pid=$!

    wait "$server_pid"
  '';
in {
  processes.imhex-mcp = {
    exec = makeImHexMcpExec;
    cwd = config.git.root;

    # The MCP server is probably stdio-based, so do not wait on a TCP MCP port.
    ready.exec = "${pkgs.coreutils}/bin/true";
  };
}
