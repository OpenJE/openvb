# ./devenv/scripts/f3-reimpl-run-data.nix

{ config, pkgs, ... }:
let
  wine = "${pkgs.wine}/bin/wine";
  wineboot = "${pkgs.wine}/bin/wineboot";
  demoRoot = config.env.OPENVB_F3DEMO_ROOT;
in {
  scripts.f3-reimpl-run-data = {
    description = "Run rebuilt F3.exe with Wine from a data-staged working directory (symlinks the demo root)";
    exec = /* bash */ ''
      set -euo pipefail

      exePath="$PWD/build/F3.exe"
      if [ ! -f "$exePath" ]; then
        printf '%s\n' "Missing build/F3.exe. Build first with: msvc-build" >&2
        exit 1
      fi

      # Stage the original demo's data directory (read-only symlinks) so the engine finds
      # resource.rht, *.grp, STF, Fonts... exactly like f3demo-run's `cd "$demoRoot"`.
      dataDir="$PWD/.devenv/state/f3-reimpl-data"
      mkdir -p "$dataDir"
      find "$dataDir" -mindepth 1 -maxdepth 1 -type l -delete
      for entry in "${demoRoot}"/*; do
        ln -sf "$entry" "$dataDir/$(basename "$entry")"
      done
      # Granny/Bink DLLs must be resolvable next to our exe too (wine loads them by module dir).
      ln -sf "$dataDir/granny2.dll" "$PWD/build/granny2.dll"
      ln -sf "$dataDir/binkw32.dll" "$PWD/build/binkw32.dll"

      prefix="$PWD/.devenv/state/f3-reimpl-wineprefix"
      export WINEPREFIX="$prefix"
      export WINEARCH=''${WINEARCH:-win32}
      export WINEDLLOVERRIDES=''${WINEDLLOVERRIDES:-mscoree,mshtml=}
      export WINEDEBUG=''${WINEDEBUG:--all,+err}

      if [ ! -f "$prefix/system.reg" ]; then
        "${wineboot}" -u >/dev/null
      fi

      f3-wine-userdirs-fix "$prefix"

      cd "$dataDir"
      exec "${wine}" "$exePath"
    '';
  };
}
