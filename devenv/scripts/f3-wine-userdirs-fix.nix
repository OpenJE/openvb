# ./devenv/scripts/f3-wine-userdirs-fix.nix

{ pkgs, ... }: {
  scripts.f3-wine-userdirs-fix = {
    description = "Replace dangling Wine user-dir symlinks with real dirs (usage: f3-wine-userdirs-fix [wineprefix])";
    exec = /* bash */ ''
      set -euo pipefail

      # A freshly wineboot'ed prefix symlinks the standard user dirs (Documents, Desktop, ...)
      # to $HOME. When the host directory is missing, the link dangles and the engine's save
      # folder cannot be created: F3::SetupSaveDirectory -> _findfirst says "not a directory",
      # CreateDirectoryA returns ERROR_ALREADY_EXISTS (183) on the link itself and
      # ERROR_FILE_NOT_FOUND (2) for anything below it -> JE::FatalError("Unable to create
      # save folder."). Replacing the dangling links with real dirs also keeps saves inside
      # drive_c instead of the host home, which is how the working prefixes are laid out.
      #
      # Idempotent: live symlinks (host dir present) and real dirs are left untouched.

      prefix="''${1:-''${WINEPREFIX:-}}"
      if [ -z "$prefix" ]; then
        printf '%s\n' "usage: f3-wine-userdirs-fix <wineprefix> (or set WINEPREFIX)" >&2
        exit 2
      fi

      userDir="$prefix/drive_c/users/$(id -un)"
      if [ ! -d "$userDir" ]; then
        exit 0
      fi

      for d in "$userDir"/*; do
        if [ -L "$d" ] && [ ! -e "$d" ]; then
          "${pkgs.coreutils}/bin/rm" -f "$d"
          "${pkgs.coreutils}/bin/mkdir" -p "$d"
          printf '%s\n' "replaced dangling user-dir link: $d"
        fi
      done
    '';
  };
}
