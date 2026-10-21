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
    # WAS "31337" - the SAME value as OPENVB_ORIG_GDB_PORT above. That collision is what blocked
    # T-034: the managed headless ImHex binds 31337, so winedbg's single-client stub for the
    # original could never be attached (ss -ltnp showed the listener owned by the ImHex pid).
    # ImHex is the non-game consumer, so it moves; the number is no longer duplicated anywhere -
    # both the process definition and the MCP config now read THIS variable (see
    # devenv/processes/imhex-headless.nix and devenv/opencode/mcp/imhex.nix), which is the actual
    # root cause: one port literal written in three places.
    OPENVB_IMHEX_MCP_PORT = "31347";

    # Engram memory (agent memory server). NOTE: deliberately NO ENGRAM_DATA_DIR here.
    # The pi extension compares the running server's instance-id against
    # `$ENGRAM_DATA_DIR/.instance-id` and FAILS CLOSED (no adopt, no spawn, no kill) on a
    # mismatch, so pinning a repo-local path would hand in-shell sessions one store and
    # out-of-shell sessions the ~/.engram default: two stores, two instance-ids, and memory
    # that silently stops working for anyone booted outside the devenv (which is exactly what
    # happened to a live session). One canonical store (~/.engram); project separation comes
    # from the tracked .engram/config.json (verified: GET /project/current returns
    # project_source "config"), not from the directory. `engram sync` still exists if
    # memories ever need to travel through the repo.
    # ENGRAM_URL stays unset: setting it marks the server externally managed and skips the
    # ownership check. ENGRAM_PORT 7437 is already the extension default, so it is unset too.
    # The release binary phones home for update checks at startup; offline determinism wins.
    ENGRAM_NO_UPDATE_CHECK = "1";
  };
}
