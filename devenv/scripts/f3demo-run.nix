# ./devenv/scripts/f3demo-run.nix

{ config, pkgs, ... }:
let
  demoRoot = config.env.OPENVB_F3DEMO_ROOT;
  wine = "${pkgs.wine}/bin/wine";
  wineboot = "${pkgs.wine}/bin/wineboot";
in {
  scripts.f3demo-run = {
    description = "Run original Van Buren demo";
    exec = /* bash */ ''
      set -euo pipefail

      stateDir="$PWD/.devenv/state"
      prefix="$stateDir/f3demo-wineprefix"

      export WINEPREFIX="$prefix"
      export WINEARCH=''${WINEARCH:-win32}
      export WINEDLLOVERRIDES=''${WINEDLLOVERRIDES:-mscoree,mshtml=}
      export WINEDEBUG=''${WINEDEBUG:--all,+err}

      "${pkgs.coreutils}/bin/mkdir" -p "$prefix"

      if [ ! -f "$prefix/system.reg" ]; then
        "${wineboot}" -u >/dev/null
      fi

      cd "${demoRoot}"
      exec "${wine}" F3.exe
    '';
  };
}
