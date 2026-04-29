# ./devenv/scripts/f3-reimpl-run-debug.nix

{ config, pkgs, ... }:
let
  defaultPort = config.env.OPENVB_REIMPL_GDB_PORT;
  wineboot = "${pkgs.wine}/bin/wineboot";
  winedbg = "${pkgs.wine}/bin/winedbg";
in {
  scripts.f3-reimpl-run-debug = {
    description = "Start rebuilt F3.exe Wine GDB stub";
    exec = /* bash */ ''
      set -euo pipefail

      exePath="$PWD/build/F3.exe"
      if [ ! -f "$exePath" ]; then
        printf '%s\n' \
          "Missing build/F3.exe." \
          "" \
          "Build first with:" \
          "  msvc-build" >&2
        exit 1
      fi

      stateDir="$PWD/.devenv/state"
      prefix="$stateDir/f3-reimpl-wineprefix"
      gdbPort=''${OPENVB_REIMPL_GDB_PORT:-${defaultPort}}

      export WINEPREFIX="$prefix"
      export WINEARCH=''${WINEARCH:-win32}
      export WINEDLLOVERRIDES=''${WINEDLLOVERRIDES:-mscoree,mshtml=}
      export WINEDEBUG=''${WINEDEBUG:--all,+err}

      "${pkgs.coreutils}/bin/mkdir" -p "$prefix"

      if [ ! -f "$prefix/system.reg" ]; then
        "${wineboot}" -u >/dev/null
      fi

      printf '%s\n' \
        "Reimplementation GDB stub listening on 127.0.0.1:$gdbPort" \
        "Use IDA Remote GDB Debugger against the rebuilt binary IDB/database." \
        "The process will wait for the first debugger connection before starting."

      exec "${winedbg}" --gdb --port "$gdbPort" --no-start "$exePath"
    '';
  };
}
