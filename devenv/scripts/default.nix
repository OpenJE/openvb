# ./devenv/scripts/default.nix

{ ... }: {
  imports = [
    ./cppcheck-openvb.nix
    ./cppcheck-openje.nix
    ./cppcheck-tracing.nix
    ./cppcheck-all.nix
    ./f3-lockstep-debug.nix
    ./f3-reimpl-run-debug.nix
    ./f3-reimpl-run-trace.nix
    ./f3-reimpl-run.nix
    ./f3demo-run-debug.nix
    ./f3demo-run.nix
    ./msvc-build.nix
    ./msvc-build-debug.nix
    ./msvc-build-tracing.nix
    ./f3-run.nix
    ./f3-run-debug.nix
  ];
}
