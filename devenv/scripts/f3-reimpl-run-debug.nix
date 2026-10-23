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

      f3-wine-userdirs-fix "$prefix"

      # Render lane self-heal, machine-portable: the prefix is host-local but outlives
      # nixpkgs/lock drift, and this working tree can be booted from either of two hosts whose
      # declared dxvk builds differ. `ensure` re-arms from THIS host's source whenever the
      # verdict is not ARMED, so a stale-armed or foreign-armed prefix can never silently
      # render through the Wine builtin (the M-001 / item-68 failure class, made self-repairing).
      # `|| true`: a REG-ERROR must be LOUD but must never abort the boot it is reporting on.
      f3-wine-dxvk ensure "$prefix" | grep '^verdict:' >&2 || true

      printf '%s\n' \
        "Reimplementation GDB stub listening on 127.0.0.1:$gdbPort" \
        "Use IDA Remote GDB Debugger against the rebuilt binary IDB/database." \
        "The process will wait for the first debugger connection before starting."

      exec "${winedbg}" --gdb --port "$gdbPort" --no-start "$exePath"
    '';
  };
}
