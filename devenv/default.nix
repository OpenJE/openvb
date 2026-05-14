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

  ohMyOpenagent.useLocalModels = true;
}
