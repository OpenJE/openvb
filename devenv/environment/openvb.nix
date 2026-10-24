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
    # M-001/T-067 render lane: the VERIFIED DXVK x32 dll set (d3d8/d3d9/dxgi). Declared here so the
    # arming tool (scripts/f3-wine-dxvk.nix) has a store-backed source instead of an ad-hoc
    # /nix/store path that a later GC silently eats - which is exactly how a prefix ended up holding
    # Wine's OWN builtin d3d8 proxy in system32 and still looked "DXVK-armed" (2026-10-22).
    # `pkgs.dxvk.bin` is the bin output; x32 -> pkgsCross.mingw32.dxvk_2/bin. 2.7.1, the version the
    # working rig recorded (md5 d3d8 76c44ce4..., d3d9 cf8a8df6..., dxgi a107ae82...).
    OPENVB_DXVK_X32 = "${pkgs.dxvk.bin}/x32";
    OPENVB_IDADIR = resolvedIdaDir;
    OPENVB_ORIG_EXE = "${localPackages.f3demo}/share/f3demo/F3.exe";
    OPENVB_ORIG_IDB = "./artifacts/F3.exe.i64";
    # 2026-10-21, measured: the ORIGINAL stub moves, not ImHex. The T-034 fix assumed ImHex's
    # Network Interface port was configurable and put it on 31347; it is not - ImHex 1.38.1 (and the
    # dfdgsdfg/imhexMCP fork built here) binds a hardcoded 31337 and exposes only an on/off setting
    # (hex.builtin.setting.general.network_interface). Verified two ways: `ss` shows the ImHex pid on
    # 31337 while ~/.config/imhex/settings.json says port 31347, and no IMHEX_* env/CLI knob exists in
    # the binary (strings: only IMHEX_SKIP_SPLASH_SCREEN / IMHEX_MEH / IMHEX_FORCE_OOBE).
    # Consequences, both of which were live bugs:
    #   * run-imhex-mcp waited on 31347 forever, timed out at 120 s, killed ImHex, and process-compose
    #     restarted it -> infinite start/kill loop (devenv processes list: restart_count climbing).
    #   * winedbg still lost 31337 to ImHex, so ida-orig-gdb still exited. Port 31337 is ImHex's by
    #     fact; the fully env-parameterized consumer (winedbg --gdb --port) is the one that can move.
    # 31437 is the port the T-034/T-052 lanes already attached with successfully (5/5 cycles).
    OPENVB_ORIG_GDB_PORT = "31437";
    OPENVB_REIMPL_EXE = "./build/F3.exe";
    OPENVB_REIMPL_IDB = "./build/F3.exe.i64";
    OPENVB_REIMPL_TRACE_EXE = "./build/F3-trace.exe";
    OPENVB_REIMPL_GDB_PORT = "31338";
    OPENVB_IDA_ORIG_MCP_PORT = "8745";
    OPENVB_IDA_REIMPL_MCP_PORT = "8746";
    # MUST stay 31337: that is ImHex's hardcoded listen port, not a choice (see the note above).
    # Renaming the contract to some other number is what broke both the process and the MCP client.
    # Also note OPENVB_IMHEX_MCP_PORT/IMHEX_PORT are IGNORED by imhex-mcp-server itself - it only reads
    # --host/--port CLI flags (default localhost:31337, and "localhost" resolves to ::1 first, which
    # ImHex never binds -> IPv4-only listener looks "refused"). Clients must pass --host 127.0.0.1.
    OPENVB_IMHEX_MCP_PORT = "31337";

    # Engram memory (agent memory server). 2026-10-23 DECISION REVERSAL (user-approved, goal
    # 20261023205641-q4x65d): the store now lives IN THIS REPO at .engram-data/ (gitignored,
    # location-only - never committed to git history; it is a live SQLite+WAL database, not a
    # mergeable text asset), migrated with `engram migrate-copy`, and ENGRAM_DATA_DIR is pinned
    # here so openvb dev sessions resolve to the repo store. ~/.engram is kept untouched as a
    # fallback and stays the shared store for every OTHER project (openvb-only split, accepted).
    # WHY pin here despite the incident below: the split-brain risk is real but only bites
    # sessions booted OUTSIDE the devenv shell - those read ~/.engram default, see an
    # instance-id mismatch with the running server, and FAIL CLOSED (no adopt, no spawn, no
    # kill) rather than corrupting anything. In-shell sessions and any `engram serve` started
    # with this env all resolve to .engram-data/ consistently. The prior breakage ('2026 live
    # session') came from pinning one path in the shell while the running server sat on the
    # other WITHOUT a decision to reverse it; the two were never reconciled. If memory ever
    # silently stops working in a session, check: which store the running server holds (see
    # the .instance-id it matches) and whether this env pin is present in that shell.
    # The pi extension compares the running server's instance-id against
    # `$ENGRAM_DATA_DIR/.instance-id` and FAILS CLOSED (no adopt, no spawn, no kill) on a
    # mismatch.
    # Project separation ALSO comes from the tracked .engram/config.json (verified:
    # GET /project/current returns project_source "config"). `engram sync` still exists if
    # memories ever need to travel through git.
    # NOTE on the path expression: this file lives at devenv/environment/, and Nix resolves
    # relative path literals against the CONTAINING FILE's directory - so the repo root is
    # ../../. (first auditor falsification pass, 2026-10-23: a ../. pin silently resolved to
    # devenv/.engram-data, a nonexistent path, exporting a broken pin while "nix parse OK")
    # Verify with: devenv print-dev-env | grep ENGRAM_DATA_DIR
    ENGRAM_DATA_DIR = "${toString ../../.}/.engram-data";
    # ENGRAM_URL stays unset: setting it marks the server externally managed and skips the
    # ownership check. ENGRAM_PORT 7437 is already the extension default, so it is unset too.
    # The release binary phones home for update checks at startup; offline determinism wins.
    ENGRAM_NO_UPDATE_CHECK = "1";
  };
}
