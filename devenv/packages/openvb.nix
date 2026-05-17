# ./devenv/packages/openvb.nix

{ inputs, lib, pkgs, ... }:
let
  localPackages = import ./local.nix { inherit pkgs; };
  imhex-unpatched = inputs.imhex.legacyPackages.${pkgs.stdenv.hostPlatform.system}.imhex;

  # Patched imhex with MCP plugin built in.
  # Applies 9 patches (0002 skipped due to API mismatch: FileProvider::open()
  # return type changed from bool to OpenResult in ImHex 1.38.1).
  # Patch order per imhexMCP PATCH_MANIFEST.md: 0007 → 0008 → 0009 → 0010 → 0011 → 0012 → 0013 → 0014 → 0001
  imhex = imhex-unpatched.overrideAttrs (old: {
    patches = (old.patches or []) ++ [
      "${localPackages.imhex-mcp-src}/patches/0007-fix-Replace-RequestOpenFile-event-based-approach-wit.patch"
      "${localPackages.imhex-mcp-src}/patches/0008-fix-Improve-disassembly-and-diff-error-handling.patch"
      "${localPackages.imhex-mcp-src}/patches/0009-fix-Implement-TaskManager-based-diff-analysis-ALL-v0.patch"
      "${localPackages.imhex-mcp-src}/patches/0010-feat-Add-batch-open_directory-endpoint-v1.0.0-Phase-.patch"
      "${localPackages.imhex-mcp-src}/patches/0011-Add-batch-search-endpoint-for-v1.0.0-Phase-2.patch"
      "${localPackages.imhex-mcp-src}/patches/0012-Add-batch-hash-endpoint-for-v1.0.0-Phase-2.patch"
      "${localPackages.imhex-mcp-src}/patches/0013-Fix-glob-pattern-matching-in-batch-open_directory.patch"
      "${localPackages.imhex-mcp-src}/patches/0014-Fix-glob-pattern-escaping-bug-in-batch-open_director.patch"
      "${localPackages.imhex-mcp-src}/patches/0001-feat-Implement-queue-based-file-opening-to-fix-netwo.patch"
      # NOTE: Patch 0002 skipped — API mismatch (bool -> OpenResult return type change)
    ];

    # Port configuration:
    # The C++ plugin registers endpoints via ImHex's CommunicationInterface (no
    # hardcoded port — ImHex manages it via settings). The Python MCP server
    # reads port from config.yaml (default 31337) and supports --port CLI arg.
    # The port is runtime-configurable; no source patch needed here.

    # postPatch: Fix MCP plugin CMakeLists.txt and source code for nixpkgs build.
    # - CMakeLists.txt: Remove 'builtin' from LIBRARIES (MODULE_LIBRARY can't link
    #   into other targets); add fonts include path for transitive <fonts/vscode_icons.hpp>.
    # - file_provider.hpp: Make open(bool) public — patches were designed for ImHex
    #   version where this was already public (old patch 02-fileprovider-public-open.patch).
    # - plugin_mcp.cpp: Handle OpenResult return type (ImHex 1.38.1 changed from bool).
    postPatch = (old.postPatch or "") + ''
      sed -i '/^[[:space:]]*builtin$/d' plugins/mcp/CMakeLists.txt
      sed -i 's|''${CMAKE_SOURCE_DIR}/plugins/builtin/include|&\n        ''${CMAKE_SOURCE_DIR}/plugins/fonts/include|' plugins/mcp/CMakeLists.txt
      sed -i '/^        OpenResult open(bool memoryMapped);/i\    public:' plugins/builtin/include/content/providers/file_provider.hpp
      sed -i 's|if (!fileProvider->open(false))|if (fileProvider->open(false).isFailure())|' plugins/mcp/source/plugin_mcp.cpp
    '';
  });
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
    localPackages.imhex-mcp-server
    localPackages.f3demo
    localPackages.hexpat-language-server
    localPackages.math-mcp
  ];
}
