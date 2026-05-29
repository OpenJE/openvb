# ./devenv/opencode/mcp/codegraph.nix

{ inputs, pkgs, ... }:
let
  system = pkgs.stdenv.hostPlatform.system;
  codegraph = inputs.codegraph.packages.${system}.codegraph;
in {
  # https://github.com/colbymchenry/codegraph
  opencode.mcp.codegraph = {
    type = "local";
    enabled = true;
    command = [
      "${codegraph}/bin/codegraph"
      "serve"
      "--mcp"
    ];
  };
}
