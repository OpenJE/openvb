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
    OPENVB_ORIG_IDB = "./artifacts/F3.exe.i64";
    OPENVB_ORIG_GDB_PORT = "31337";
    OPENVB_REIMPL_EXE = "./build/F3.exe";
    OPENVB_REIMPL_IDB = "./build/F3.exe.i64";
    OPENVB_REIMPL_TRACE_EXE = "./build/F3-trace.exe";
    OPENVB_REIMPL_GDB_PORT = "31338";
    OPENVB_IDA_ORIG_MCP_PORT = "8745";
    OPENVB_IDA_REIMPL_MCP_PORT = "8746";
    OPENVB_IMHEX_MCP_PORT = "31337";

    # Engram memory (agent memory server). Absolute on purpose: the pi extension
    # compares the running server's instance-id against `$ENGRAM_DATA_DIR/.instance-id`
    # and FAILS CLOSED (no adopt, no spawn, no kill) on a mismatch, so a relative value
    # would split into two stores the moment a session is booted from a subdirectory
    # (e.g. modules/openje) instead of the repo root. Same reason it is declared here
    # rather than left to the ~/.engram default: both agents must resolve one store.
    ENGRAM_DATA_DIR = "${toString ./.}/.devenv/state/engram";
    # Port of the local server the extension probes and auto-spawns when ENGRAM_URL is
    # unset. Leave ENGRAM_URL unset; setting it marks the server externally managed and
    # skips the ownership check entirely.
    ENGRAM_PORT = "7437";
    # The release binary phones home for updates at startup; offline determinism wins.
    ENGRAM_NO_UPDATE_CHECK = "1";
  };
}
