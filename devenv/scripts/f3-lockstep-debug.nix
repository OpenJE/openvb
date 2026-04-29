# ./devenv/scripts/f3-lockstep-debug.nix

{ config, pkgs, ... }:
let
  origRoot = config.env.OPENVB_F3DEMO_ROOT;
  origDefaultPort = config.env.OPENVB_ORIG_GDB_PORT;
  reimplDefaultPort = config.env.OPENVB_REIMPL_GDB_PORT;
  wineboot = "${pkgs.wine}/bin/wineboot";
  winedbg = "${pkgs.wine}/bin/winedbg";
in {
  scripts.f3-lockstep-debug = {
    description = "Start original and rebuilt GDB stubs";
    exec = /* bash */ ''
      set -euo pipefail

      reimplExe="$PWD/build/F3.exe"
      if [ ! -f "$reimplExe" ]; then
        printf '%s\n' \
          "Missing build/F3.exe." \
          "" \
          "Build first with:" \
          "  msvc-build" >&2
        exit 1
      fi

      stateDir="$PWD/.devenv/state"
      origPrefix="$stateDir/f3demo-wineprefix"
      reimplPrefix="$stateDir/f3-reimpl-wineprefix"
      origPort=''${OPENVB_ORIG_GDB_PORT:-${origDefaultPort}}
      reimplPort=''${OPENVB_REIMPL_GDB_PORT:-${reimplDefaultPort}}

      run_wineboot() {
        local prefix="$1"
        export WINEPREFIX="$prefix"
        export WINEARCH=''${WINEARCH:-win32}
        export WINEDLLOVERRIDES=''${WINEDLLOVERRIDES:-mscoree,mshtml=}
        export WINEDEBUG=''${WINEDEBUG:--all,+err}
        "${pkgs.coreutils}/bin/mkdir" -p "$prefix"
        if [ ! -f "$prefix/system.reg" ]; then
          "${wineboot}" -u >/dev/null
        fi
      }

      run_wineboot "$origPrefix"
      (
        export WINEPREFIX="$origPrefix"
        export WINEARCH=''${WINEARCH:-win32}
        export WINEDLLOVERRIDES=''${WINEDLLOVERRIDES:-mscoree,mshtml=}
        export WINEDEBUG=''${WINEDEBUG:--all,+err}
        cd "${origRoot}"
        exec "${winedbg}" --gdb --port "$origPort" --no-start F3.exe
      ) &
      origPid=$!

      run_wineboot "$reimplPrefix"
      (
        export WINEPREFIX="$reimplPrefix"
        export WINEARCH=''${WINEARCH:-win32}
        export WINEDLLOVERRIDES=''${WINEDLLOVERRIDES:-mscoree,mshtml=}
        export WINEDEBUG=''${WINEDEBUG:--all,+err}
        exec "${winedbg}" --gdb --port "$reimplPort" --no-start "$reimplExe"
      ) &
      reimplPid=$!

      cleanup() {
        kill "$origPid" "$reimplPid" 2>/dev/null || true
      }
      trap cleanup EXIT INT TERM

      printf '%s\n' \
        "Original demo GDB stub:  127.0.0.1:$origPort" \
        "Reimplementation stub:   127.0.0.1:$reimplPort" \
        "Attach the original and rebuilt IDA debugger sessions to their respective stubs." \
        "Then use the lockstep-debugger agent to coordinate checkpoints and stepping."

      wait "$origPid" "$reimplPid"
    '';
  };
}
