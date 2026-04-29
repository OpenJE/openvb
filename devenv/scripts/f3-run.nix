# ./devenv/scripts/f3-run.nix

{ ... }: {
  scripts.f3-run = {
    description = "Run rebuilt F3.exe with Wine (legacy alias)";
    exec = /* bash */ ''
      exec f3-reimpl-run "$@"
    '';
  };
}
