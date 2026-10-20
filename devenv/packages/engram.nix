# ./devenv/packages/engram.nix

# Engram (persistent agent memory) — prebuilt Go binary, pinned to a release asset.
# Verified against GitHub's release API on 2026-10-20:
#   latest tag  v3.5.1
#   asset       engram_3.5.1_linux_amd64.tar.gz  (8064652 bytes, contains ./engram + tools/)
#   sha256      739d188cb03634a77d435ad4a8039de30e599b0e696256ebf7118b29675307ca
#               -> SRI sha256-c50YjLA2NKd9Q1rUqAOd4w5Zmw5pYlbr9xGLKWdTB8o=
# The tarball is a plain .tar.gz with two top-level entries (see sourceRoot below),
# so the default unpackPhase handles it — no dontUnpack and no custom unpackPhase.
{ stdenvNoCC, fetchurl, lib }:

stdenvNoCC.mkDerivation rec {
  pname = "engram";
  version = "3.5.1";

  src = fetchurl {
    url = "https://github.com/Gentleman-Programming/engram/releases/download/v${version}/engram_${version}_linux_amd64.tar.gz";
    hash = "sha256-c50YjLA2NKd9Q1rUqAOd4w5Zmw5pYlbr9xGLKWdTB8o=";
  };

  # The tarball unpacks to TWO top-level entries (`engram` and `tools/`), so nix's
  # auto-detected sourceRoot picks the `tools` directory and the binary is not in $PWD.
  # Pin the build directory to the unpack root instead.
  sourceRoot = ".";

  # go binary, no compiler needed; the bundled tools/*.sh are cloud-sync helpers
  # we do not use (and are not on any PATH we care about).
  installPhase = ''
    runHook preInstall
    test -f engram || { echo "engram binary missing from $PWD"; ls -la; exit 1; }
    mkdir -p "$out/bin"
    install -m755 engram "$out/bin/engram"
    runHook postInstall
  '';

  # Retrieval is SQLite FTS5 + BM25 (modernc.org/sqlite, pure Go) — no vector store,
  # no embedding API keys. `--semantic` conflict scanning delegates to an agent CLI
  # (ENGRAM_AGENT_CLI), it does not call a provider itself.
  # Extension pairing: npm:gentle-engram@0.4.2 (npm `latest`) requires core >= 3.0.0
  # with capabilities.root_session_resume, both advertised by this release.
  meta = with lib; {
    description = "Persistent memory for AI coding agents (HTTP server + MCP + CLI)";
    homepage = "https://github.com/Gentleman-Programming/engram";
    license = licenses.mit;
    platforms = [ "x86_64-linux" ];
    mainProgram = "engram";
    sourceProvenance = with sourceTypes; [ binaryNativeCode ];
  };
}
