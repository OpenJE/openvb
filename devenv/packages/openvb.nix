# ./devenv/packages/openvb.nix

{ lib, pkgs, ... }:
let
  localPackages = import ./local.nix { inherit pkgs; };
in {
  packages = with pkgs; [
    docker
    gdb
    ninja
    (lib.getBin cppcheck)
    pkgsCross.mingw32.stdenv.cc
    uv
    localPackages.f3demo
    localPackages.math-mcp
  ];
}
