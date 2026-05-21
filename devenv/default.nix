# ./devenv/default.nix

{ ... }: {
  imports = [
    ./environment
    ./languages
    ./packages
    ./processes
    ./scripts
    ./services
    ./tasks
    ./opencode
  ];

  ohMyOpenagent.harness = "local";
  ohMyOpenagent.reHarness = "local";
}
