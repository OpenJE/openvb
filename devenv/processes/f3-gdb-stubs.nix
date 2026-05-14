# ./devenv/processes/f3-gdb-stubs.nix

{ config, pkgs, ... }:
let
  wineboot = "${pkgs.wine}/bin/wineboot";
  winedbg = "${pkgs.wine}/bin/winedbg";
in {
  processes.ida-orig-gdb = {
    exec = /* bash */ ''
      set -euo pipefail

      stateDir="$PWD/.devenv/state"
      prefix="$stateDir/f3demo-wineprefix"
      demoRoot="${config.env.OPENVB_F3DEMO_ROOT}"
      gdbPort=''${OPENVB_ORIG_GDB_PORT:-${config.env.OPENVB_ORIG_GDB_PORT}}

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

      cd "$demoRoot"
      exec "${winedbg}" --gdb --port "$gdbPort" --no-start F3.exe
    '';
    cwd = config.git.root;
    ready.exec = "${pkgs.netcat-openbsd}/bin/nc -z 127.0.0.1 ${config.env.OPENVB_ORIG_GDB_PORT}";
  };

  processes.ida-reimpl-gdb = {
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
      gdbPort=''${OPENVB_REIMPL_GDB_PORT:-${config.env.OPENVB_REIMPL_GDB_PORT}}

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
    cwd = config.git.root;
    ready.exec = "${pkgs.netcat-openbsd}/bin/nc -z 127.0.0.1 ${config.env.OPENVB_REIMPL_GDB_PORT}";
  };
}