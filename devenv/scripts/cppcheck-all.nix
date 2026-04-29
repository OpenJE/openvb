# ./devenv/scripts/cppcheck-all.nix

{ ... }: {
  scripts.cppcheck-all = {
    description = "Run cppcheck on all modules";
    exec = /* bash */ ''
      set -euo pipefail
      cppcheck-openvb
      cppcheck-openje
      exec cppcheck-tracing
    '';
  };
}
