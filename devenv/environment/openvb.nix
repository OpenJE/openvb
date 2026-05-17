# ./devenv/environment/openvb.nix

{ pkgs, ... }:
let
  projectIdaDir = builtins.getEnv "OPENVB_IDADIR";
  nixIdaDir = builtins.getEnv "NIX_IDADIR";
  localPackages = import ../packages/local.nix { inherit pkgs; };
  resolvedIdaDir =
    if projectIdaDir != "" then projectIdaDir
    else nixIdaDir;
in {
  env = {
    MSVC_IMAGE = "ghcr.io/openje/msvc2002:latest";
    OPENVB_CMAKE_VERSION = "3.24.4";
    OPENVB_F3DEMO_ROOT = "${localPackages.f3demo}/share/f3demo";
    OPENVB_IDADIR = resolvedIdaDir;
    OPENVB_ORIG_EXE = "${localPackages.f3demo}/share/f3demo/F3.exe";
    OPENVB_ORIG_IDB = "./F3.exe.i64";
    OPENVB_ORIG_GDB_PORT = "31337";
    OPENVB_REIMPL_EXE = "./build/F3.exe";
    OPENVB_REIMPL_IDB = "./build/F3.exe.i64";
    OPENVB_REIMPL_TRACE_EXE = "./build/F3-trace.exe";
    OPENVB_REIMPL_GDB_PORT = "31338";
    OPENVB_IDA_ORIG_MCP_PORT = "8745";
    OPENVB_IDA_REIMPL_MCP_PORT = "8746";
    OPENVB_IMHEX_MCP_PORT = "31337";
  };
}
