# ./devenv/scripts/f3-run-debug.nix

{ ... }: {
  scripts.f3-run-debug = {
    description = "Run traced F3-trace.exe with Wine (legacy alias)";
    exec = /* bash */ ''
      exec f3-reimpl-run-trace "$@"
    '';
  };
}
