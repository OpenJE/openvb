# ./devenv/packages/ghidra.nix

{ pkgs, ... }:
let
  # Ghidra 12.x + JDK 21 - the documented requirements of the pi-ghidra package
  # (second-decompiler cross-check beside IDA). nixpkgs wraps the JVM itself, so
  # no openjdk needs to leak into PATH.
  ghidra = pkgs.ghidra;
in
{
  packages = [
    ghidra
    # pi-ghidra launches the official `analyzeHeadless`; nixpkgs ships it as
    # `ghidra-analyzeHeadless` plus the support script - expose the canonical name.
    (pkgs.writeShellScriptBin "analyzeHeadless"
      ''
        exec ${ghidra}/bin/ghidra-analyzeHeadless "$@"
      '')
  ];
}
