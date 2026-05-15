# ./devenv/packages/openvb.nix

{ inputs, lib, pkgs, ... }:
let
  localPackages = import ./local.nix { inherit pkgs; };
  imhex = inputs.imhex.legacyPackages.${pkgs.stdenv.hostPlatform.system}.imhex;
in {
  packages = (with pkgs; [
    docker
    gdb
    ninja
    (lib.getBin cppcheck)
    pkgsCross.mingw32.stdenv.cc
    uv
  ]) ++ [
    imhex
    localPackages.f3demo
    localPackages.hexpat-language-server
    localPackages.math-mcp
  ];
}
