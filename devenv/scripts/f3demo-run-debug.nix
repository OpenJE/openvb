# ./devenv/scripts/f3demo-run-debug.nix

{ config, pkgs, ... }:
let
  demoRoot = config.env.OPENVB_F3DEMO_ROOT;
  defaultPort = config.env.OPENVB_ORIG_GDB_PORT;
  wineboot = "${pkgs.wine}/bin/wineboot";
  winedbg = "${pkgs.wine}/bin/winedbg";
in {
  scripts.f3demo-run-debug = {
    description = "Start original demo Wine GDB stub";
    exec = /* bash */ ''
      set -euo pipefail

      stateDir="$PWD/.devenv/state"
      prefix="$stateDir/f3demo-wineprefix"
      gdbPort=''${OPENVB_ORIG_GDB_PORT:-${defaultPort}}

      export WINEPREFIX="$prefix"
      export WINEARCH=''${WINEARCH:-win32}
      export WINEDLLOVERRIDES=''${WINEDLLOVERRIDES:-mscoree,mshtml=}
      export WINEDEBUG=''${WINEDEBUG:--all,+err}

      "${pkgs.coreutils}/bin/mkdir" -p "$prefix"

      if [ ! -f "$prefix/system.reg" ]; then
        "${wineboot}" -u >/dev/null
      fi

      printf '%s\n' \
        "Original demo GDB stub listening on 127.0.0.1:$gdbPort" \
        "Use IDA Remote GDB Debugger against the original demo IDB/database." \
        "The process will wait for the first debugger connection before starting."

      cd "${demoRoot}"
      exec "${winedbg}" --gdb --port "$gdbPort" --no-start F3.exe
    '';
  };
}
