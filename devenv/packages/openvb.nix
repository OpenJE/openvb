{ inputs, lib, pkgs, ... }:

let
  localPackages = import ./local.nix { inherit pkgs; };

  f3demoDir = "${localPackages.f3demo}/share/f3demo";

  f3demoBuildFiles =
    lib.mapAttrs'
      (name: _type:
        lib.nameValuePair "build/${name}" {
          source = "${f3demoDir}/${name}";
        }
      )
      (
        lib.filterAttrs
          (name: _type: name != "F3.exe")
          (builtins.readDir f3demoDir)
      );

  imhex-mcp-src = localPackages.imhex-mcp-src;

  imhex-src = pkgs.fetchFromGitHub {
    owner = "WerWolv";
    repo = "ImHex";

    # Match whatever the imhexMCP fork expects.
    # If this hash is wrong, Nix will print the correct one.
    rev = "v1.38.1";
    hash = "sha256-lkpFiXuEF72nBkPuInv683Ct1Uu+uZ0PGejI9cVEUp0=";

    fetchSubmodules = true;
  };

  imhexBuildInputs = with pkgs; [
    openssl
    curl
    zlib
    xz
    bzip2
    mbedtls
    file

    libGL
    glfw
    freetype
    fontconfig
    dbus
    gtk3

    libx11
    libxext
    libxcursor
    libxi
    libxinerama
    libxrandr
    libxrender
    libxcb
    libSM
    libICE

    wayland

    # Provides libstdc++.so.6 and libgcc_s.so.1.
    stdenv.cc.cc.lib
  ];

  imhex-mcp = pkgs.stdenv.mkDerivation {
    pname = "imhex-mcp";
    version = "unstable";

    src = imhex-mcp-src;

    nativeBuildInputs = with pkgs; [
      cmake
      ninja
      pkg-config
      python3
      git
      patch
      patchelf
    ];

    buildInputs = imhexBuildInputs;
    dontUseCmakeConfigure = true;

    configurePhase = ''
      runHook preConfigure

      chmod -R u+w .
      patchShebangs .

      cp -R ${imhex-src} ImHex
      chmod -R u+w ImHex

      echo "Applying imhexMCP patches in manifest-compatible order..."

      # Do not apply every patch in the directory.
      # Some old compatibility patches target older ImHex APIs.
      # This order matches the working manifest path:
      # 0007 -> 0008 -> 0009 -> 0010 -> 0011 -> 0012 -> 0013 -> 0014 -> 0001
      for patch_num in 0007 0008 0009 0010 0011 0012 0013 0014 0001; do
        patch_file="$(find patches -maxdepth 1 -type f -name "$patch_num-*.patch" | sort | head -n 1)"

        if [ -z "$patch_file" ]; then
          echo "Missing imhexMCP patch $patch_num"
          find patches -maxdepth 1 -type f -name '*.patch' | sort
          exit 1
        fi

        echo "Applying $patch_file"
        patch -d ImHex -p1 < "$patch_file"
      done

      echo "Fixing MCP plugin CMake for Nix/ImHex MODULE_LIBRARY builtin target..."

      sed -i '/^[[:space:]]*builtin[[:space:]]*$/d' ImHex/plugins/mcp/CMakeLists.txt

      sed -i \
        's|''${CMAKE_SOURCE_DIR}/plugins/builtin/include|&\n        ''${CMAKE_SOURCE_DIR}/plugins/fonts/include|' \
        ImHex/plugins/mcp/CMakeLists.txt

      sed -i \
        '/^        OpenResult open(bool memoryMapped);/i\    public:' \
        ImHex/plugins/builtin/include/content/providers/file_provider.hpp

      substituteInPlace ImHex/plugins/mcp/source/plugin_mcp.cpp \
        --replace-fail \
          'if (!fileProvider->open(false))' \
          'if (fileProvider->open(false).isFailure())'

      runHook postConfigure
    '';

    buildPhase = ''
      runHook preBuild

      cd ImHex

      cmake -S . -B build \
        -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$out" \
        -DSYSTEM_PLUGINS_LOCATION="$out/lib/imhex/plugins"

      cmake --build build

      cd ..

      runHook postBuild
    '';

    installPhase = ''
      runHook preInstall

      mkdir -p "$out/bin"
      mkdir -p "$out/lib"

      imhex_bin="$(find ImHex/build -type f -perm -0100 -name imhex | head -n 1)"

      if [ -z "$imhex_bin" ]; then
        echo "Could not find built imhex binary"
        find ImHex/build -type f -perm -0100 | sort || true
        exit 1
      fi

      imhex_build_dir="$(dirname "$imhex_bin")"

      echo "ImHex binary: $imhex_bin"
      echo "ImHex build dir: $imhex_build_dir"

      cp "$imhex_bin" "$out/bin/imhex"

      # This is the important part:
      # ImHex expects plugins next to the executable in a sibling/relative plugins dir.
      # The CMake macro emits plugins to ''${IMHEX_MAIN_OUTPUT_DIRECTORY}/plugins.
      if [ ! -d "$imhex_build_dir/plugins" ]; then
        echo "Expected plugin output dir missing: $imhex_build_dir/plugins"
        echo "Available hexplug files:"
        find ImHex/build -type f \( -name '*.hexplug' -o -name '*.hexpluglib' \) -print | sort || true
        exit 1
      fi

      cp -R "$imhex_build_dir/plugins" "$out/bin/plugins"

      if [ ! -f "$out/bin/plugins/builtin.hexplug" ]; then
        echo "builtin.hexplug missing from $out/bin/plugins"
        echo "Installed plugins:"
        find "$out/bin/plugins" -maxdepth 2 -type f -print | sort || true
        echo "Built plugins:"
        find ImHex/build -type f \( -name '*.hexplug' -o -name '*.hexpluglib' \) -print | sort || true
        exit 1
      fi

      # Install ImHex's own shared libraries.
      find ImHex/build -type f \
        \( -name 'libimhex.so*' -o -name 'lib*.so*' \) \
        -exec cp -P {} "$out/lib/" \;

      runtime_rpath="$out/lib:$out/bin/plugins:${lib.makeLibraryPath imhexBuildInputs}"

      patchelf --set-rpath "$runtime_rpath" "$out/bin/imhex"

      find "$out/lib" "$out/bin/plugins" -type f | while read -r elf; do
        if patchelf --print-rpath "$elf" >/dev/null 2>&1; then
          patchelf --set-rpath "$runtime_rpath" "$elf" || true
        fi
      done

      echo "Final installed plugin tree:"
      find "$out/bin/plugins" -maxdepth 1 -type f -name '*.hexplug*' -print | sort

      runHook postInstall
    '';

    meta = with lib; {
      description = "ImHex built with dfdgsdfg/imhexMCP patches";
      homepage = "https://github.com/dfdgsdfg/imhexMCP";
      platforms = platforms.linux;
      mainProgram = "imhex";
    };
  };
in
{
  files = f3demoBuildFiles;

  packages = (with pkgs; [
    docker
    gdb
    ninja
    (lib.getBin pkgs.cppcheck)
    pkgsCross.mingw32.stdenv.cc
    uv
    iproute2
  ]) ++ [
    inputs.wrkflw.legacyPackages.${pkgs.stdenv.hostPlatform.system}.wrkflw
    imhex-mcp
    localPackages.imhex-mcp-server
    localPackages.f3demo
    localPackages.hexpat-language-server
    localPackages.math-mcp
  ];
}
