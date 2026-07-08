# ./modules/packages/mcp-nixos.nix

{ inputs, ... }: {
  packages = with inputs.nixpkgs-master; [
    cudaPackages.cuda_cudart
    cudaPackages.cuda_nvcc
  ];
}
