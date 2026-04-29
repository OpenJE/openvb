# ./devenv/processes/ida-headless.nix

{ config, pkgs, ... }:
let
  localPackages = import ../packages/local.nix { inherit pkgs; };
  serverBin = "${localPackages.ida-pro-mcp}/bin/idalib-mcp";
  makeHeadlessExec = name: port: targetSelector: /* bash */ ''
    set -euo pipefail

    idadir=''${OPENVB_IDADIR:-''${NIX_IDADIR:-}}
    if [ -z "$idadir" ]; then
      printf '%s\n' \
        "No IDA directory configured for ${name}." \
        "Set OPENVB_IDADIR to your local IDA installation, or export NIX_IDADIR before entering devenv." >&2
      exit 1
    fi

    if [ ! -d "$idadir" ]; then
      printf 'Configured IDA directory does not exist for ${name}: %s\n' "$idadir" >&2
      exit 1
    fi

    export IDADIR="$idadir"
    # Reuse the interactive profile so the accepted license/EULA state is shared.
    export IDAUSR="$HOME/.idapro"
    export TVHEADLESS=1
    export IDA_LICENSE="keyfile=$idadir/idapro.hexlic:setpref"

    startup_target="$(${targetSelector})"

    exec "${serverBin}" \
      --host 127.0.0.1 \
      --port ${port} \
      --unsafe \
      "$startup_target"
  '';
in {
  processes.ida-orig-headless = {
    exec = makeHeadlessExec "ida-orig-headless" config.env.OPENVB_IDA_ORIG_MCP_PORT ''
      if [ -f "$OPENVB_ORIG_IDB" ]; then
        printf '%s' "$OPENVB_ORIG_IDB"
      elif [ -f "$OPENVB_ORIG_EXE" ]; then
        printf '%s' "$OPENVB_ORIG_EXE"
      else
        printf '%s\n' "No original startup target found: $OPENVB_ORIG_IDB or $OPENVB_ORIG_EXE" >&2
        exit 1
      fi
    '';
    cwd = config.git.root;
    ready.exec = "${pkgs.netcat-openbsd}/bin/nc -z 127.0.0.1 ${config.env.OPENVB_IDA_ORIG_MCP_PORT}";
  };

  processes.ida-reimpl-headless = {
    exec = makeHeadlessExec "ida-reimpl-headless" config.env.OPENVB_IDA_REIMPL_MCP_PORT ''
      if [ -f "$OPENVB_REIMPL_IDB" ]; then
        printf '%s' "$OPENVB_REIMPL_IDB"
      elif [ -f "$OPENVB_REIMPL_EXE" ]; then
        printf '%s' "$OPENVB_REIMPL_EXE"
      else
        printf '%s\n' "No reimplementation startup target found: $OPENVB_REIMPL_IDB or $OPENVB_REIMPL_EXE" >&2
        exit 1
      fi
    '';
    cwd = config.git.root;
    ready.exec = "${pkgs.netcat-openbsd}/bin/nc -z 127.0.0.1 ${config.env.OPENVB_IDA_REIMPL_MCP_PORT}";
  };
}
