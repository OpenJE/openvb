# ./devenv/scripts/f3-reimpl-run-tracing.nix

{ pkgs, ... }:
let
  wine = "${pkgs.wine}/bin/wine";
  wineboot = "${pkgs.wine}/bin/wineboot";
in {
  scripts.f3-reimpl-run-trace = {
    description = "Run rebuilt traced F3-trace.exe";
    exec = /* bash */ ''
      set -euo pipefail

      exePath="$PWD/build/F3-trace.exe"
      if [ ! -f "$exePath" ]; then
        printf '%s\n' \
          "Missing build/F3-trace.exe." \
          "" \
          "Build first with:" \
          "  msvc-build-tracing" >&2
        exit 1
      fi

      stateDir="$PWD/.devenv/state"
      prefix="$stateDir/f3-reimpl-wineprefix"

      export WINEPREFIX="$prefix"
      export WINEARCH=''${WINEARCH:-win32}
      export WINEDLLOVERRIDES=''${WINEDLLOVERRIDES:-mscoree,mshtml=}
      export WINEDEBUG=''${WINEDEBUG:--all,+err}

      "${pkgs.coreutils}/bin/mkdir" -p "$prefix"

      if [ ! -f "$prefix/system.reg" ]; then
        "${wineboot}" -u >/dev/null
      fi

      exec "${wine}" "$exePath"
    '';
  };
}
