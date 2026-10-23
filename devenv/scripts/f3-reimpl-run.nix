# ./devenv/scripts/f3-reimpl-run.nix

{ pkgs, ... }:
let
  wine = "${pkgs.wine}/bin/wine";
  wineboot = "${pkgs.wine}/bin/wineboot";
in {
  scripts.f3-reimpl-run = {
    description = "Run rebuilt F3.exe with Wine";
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

      # THE RUNNER WORKS OR IT SAYS WHAT IS MISSING - the user ran this and got JE::FatalError as a
      # MessageBox with a silent terminal, because the app booted from a cwd with no resource.rht and the
      # staging lives in a SEPARATE script nobody had a reason to run first. A prerequisite the script knows
      # it needs but does not arrange for is a bug in the script, not a user error; stage it here (idempotent),
      # and fail loudly if the staging itself fails.
      dataDir="$PWD/.devenv/state/f3-reimpl-data"   # f3-reimpl-run-data stages here; the RUN must exec from here too
      if [ ! -f "$dataDir/resource.rht" ]; then
        if command -v f3-reimpl-run-data >/dev/null 2>&1; then
          printf 'f3-run: resource.rht not in cwd - staging demo data (f3-reimpl-run-data)\n' >&2
          f3-reimpl-run-data
        else
          printf '%s\n' "resource.rht not in cwd and f3-reimpl-run-data not found - run it first:" \
                         "  f3-reimpl-run-data" >&2
          exit 1
        fi
      fi

      # THE INTRO IS THE PRODUCT'S DEFAULT BEHAVIOUR. Until now the intro movies needed a second env knob
      # (OPENVB_MOVIES=1) on top of the render gate, so a plain run showed no logos and the user reasonably
      # read that as the feature being broken. The runner is the INTERACTIVE path - CI never comes through
      # here (it runs via run-all-tests, whose app-boot stays ungated and env-clean), so the runner turns
      # both on by default and any value the user set explicitly still wins: ''${VAR+set} keeps their =0.
      export OPENVB_RENDER=''${OPENVB_RENDER:-1}
      export OPENVB_MOVIES=''${OPENVB_MOVIES:-1}

      # Self-reporting by default: the movie and render traces always land in state, so a run that looks
      # wrong can be diagnosed from files instead of from a console that a GUI-subsystem image empties.
      # Explicit paths still win. (The decode loop itself writes OPENVB_MOVIES_TRACE lines; OutputDebugString
      # is invisible under wine without a debugger, which is why a file channel was chosen in the first place.)
      # Trace paths are RELATIVE FILENAMES and the exec below cds into the staged data dir first. The first
      # version converted $PWD to a wine Z: path with sed inside a nix string, and produced mangled
      # multi-backslash paths - which is how a "default on" trace produced no file at all. Relative names
      # need no conversion at all, and the app's cwd is exactly the dir that staging maintains.
      export OPENVB_RENDER_TRACE=''${OPENVB_RENDER_TRACE:-render.log}
      export OPENVB_MOVIES_TRACE=''${OPENVB_MOVIES_TRACE:-movies.log}

      # Exec FROM THE DATA DIR, like f3demo-run does: the app resolves resource.rht/STF/Fonts relative to
      # cwd, and the repo root has none of them - that mismatch was the user's silent JE::FatalError.
      cd "$dataDir"
      exec "${wine}" "$exePath"
    '';
  };
}
