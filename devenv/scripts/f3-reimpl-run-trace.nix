# ./devenv/scripts/f3-reimpl-run-trace.nix

{ config, pkgs, ... }:
let
  wine = "${pkgs.wine}/bin/wine";
  wineboot = "${pkgs.wine}/bin/wineboot";
  demoRoot = config.env.OPENVB_F3DEMO_ROOT;
in {
  scripts.f3-reimpl-run-trace = {
    description = "Run rebuilt traced F3-trace.exe with Wine from a data-staged working directory";
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

      # Dangling user-dir symlinks left by a fresh wineboot break the save folder.
      f3-wine-userdirs-fix "$prefix"

      # Say out loud which d3d8 the render path will get (an "armed" prefix that is really using
      # the Wine builtin looks identical from the trace stream).
      # `|| true`: a REG-ERROR verdict must be LOUD but must never abort the boot it is reporting on.
      f3-wine-dxvk status "$prefix" | grep '^verdict:' >&2 || true

      # Stage the original demo's data directory (read-only symlinks) so the engine finds
      # resource.rht, *.grp, STF, Fonts... exactly like f3-reimpl-run-data's `cd "$dataDir"`.
      # Without this JE::ResourceSystem::Init fatals with "Couldn't open hashtable file".
      dataDir="$stateDir/f3-reimpl-data"
      mkdir -p "$dataDir"
      find "$dataDir" -mindepth 1 -maxdepth 1 -type l -delete
      for entry in "${demoRoot}"/*; do
        ln -sf "$entry" "$dataDir/$(basename "$entry")"
      done
      # Granny/Bink DLLs must be resolvable next to our exe too (wine loads them by module dir).
      ln -sf "$dataDir/granny2.dll" "$PWD/build/granny2.dll"
      ln -sf "$dataDir/binkw32.dll" "$PWD/build/binkw32.dll"

      cd "$dataDir"
      exec "${wine}" "$exePath"
    '';
  };
}
