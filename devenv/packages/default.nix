# ./devenv/packages/default.nix

{ ... }: {
  imports = [
    ./openvb.nix
    ./codegraph.nix
    ./opencode.nix
    #./cuda.nix
    #./llama-cpp.nix
    #./vllm.nix
  ];
}
