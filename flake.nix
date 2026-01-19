{
  description = "openvb dev shell";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-25.11";

  outputs = { self, nixpkgs }:
  let
    system = "x86_64-linux";
    pkgs = import nixpkgs {
      inherit system;
      config.allowUnsupportedSystem = true;
    };
    msvcImage = "ghcr.io/openje/msvc2002:latest";

  in {
    devShells = {
      "${system}" = rec {
        host = pkgs.mkShell {
          packages = [
            pkgs.cmake
            pkgs.docker
            pkgs.ninja
            pkgs.llvmPackages.clang-tools
          ];
        };

        mingw32 = pkgs.mkShell {
          packages = [
            pkgs.cmake
            pkgs.ninja
            pkgs.llvmPackages.clang-tools

            pkgs.pkgsCross.mingw32.clangStdenv.cc
            pkgs.pkgsCross.mingw32.windows.mcfgthreads.dev
          ];

          shellHook = ''
            export CC=i686-w64-mingw32-clang
            export CXX=i686-w64-mingw32-clang++
          '';
        };

        default = mingw32;
      };
    };

    apps = {
      ${system} = {
        msvc-build = {
          type = "app";
          program = toString (pkgs.writeShellScript "msvc-build" ''
            set -euo pipefail

            docker="${pkgs.docker}/bin/docker"
            uid="$(id -u)"
            gid="$(id -g)"

            rootless_docker=false
            if [ -z "''${DOCKER_HOST:-}" ]; then
              if [ -n "''${XDG_RUNTIME_DIR:-}" ] && [ -S "''${XDG_RUNTIME_DIR}/docker.sock" ]; then
                export DOCKER_HOST="unix://''${XDG_RUNTIME_DIR}/docker.sock"
                rootless_docker=true
              elif [ -S "/run/user/$uid/docker.sock" ]; then
                export DOCKER_HOST="unix:///run/user/$uid/docker.sock"
                rootless_docker=true
              fi
            else
              case "''${DOCKER_HOST}" in
                "unix://''${XDG_RUNTIME_DIR:-}/docker.sock"|"unix:///run/user/$uid/docker.sock")
                  rootless_docker=true
                  ;;
              esac
            fi

            prep_user_arg="--user 0:0"
            docker_user_arg="--user $uid:$gid"

            if ! "$docker" info >/dev/null 2>&1; then
              cat >&2 <<EOF
Docker daemon not reachable.

You're using rootless Docker on NixOS. Try:
  systemctl --user enable --now docker

If needed, set:
  export DOCKER_HOST=unix:///run/user/$uid/docker.sock

Then re-run:
  nix run .#msvc-build
EOF
              exit 1
            fi

            curl="${pkgs.curl}/bin/curl"
            unzip="${pkgs.unzip}/bin/unzip"

            cmakeVersion="3.24.4"
            cmakeDir="build/tools/cmake-''${cmakeVersion}-windows-i386"
            cmakeExe="$PWD/$cmakeDir/bin/cmake.exe"

            if [ ! -x "$cmakeExe" ]; then
              "${pkgs.coreutils}/bin/mkdir" -p "build/tools"
              tmpZip="build/tools/cmake-''${cmakeVersion}-windows-i386.zip"
              "$curl" -L -o "$tmpZip" \
                "https://github.com/Kitware/CMake/releases/download/v$cmakeVersion/cmake-$cmakeVersion-windows-i386.zip"
              "$unzip" -q "$tmpZip" -d "build/tools"
            fi

            cmakeExeWin="Z:\\work\\build\\tools\\cmake-$cmakeVersion-windows-i386\\bin\\cmake.exe"

            "$docker" pull ${msvcImage} >/dev/null

            chown_cmd=""
            if [ "$rootless_docker" != true ]; then
              chown_cmd=" && chown -R $uid:$gid /work/build-msvc /work/obj /work/build /work/build/wineprefix"
            fi

            "$docker" run --rm -t \
              $prep_user_arg \
              -v "$PWD:/work" -w /work \
              ${msvcImage} \
              -lc "mkdir -p /work/build-msvc /work/obj /work/build /work/build/wineprefix && if [ ! -f /work/build/wineprefix/system.reg ]; then cp -a --no-preserve=ownership /home/msvc2002/.wine32/. /work/build/wineprefix/; fi && chmod -R a+rwX /work/build-msvc /work/obj /work/build /work/build/wineprefix$chown_cmd"

            "$docker" run --rm -t \
              $docker_user_arg \
              -e CMAKE_EXE_WIN="$cmakeExeWin" \
              -v "$PWD:/work" -w /work \
              ${msvcImage} \
              -lc '
                WINEPREFIX=/work/build/wineprefix \
                  wine cmd /c "call Z:\\home\\msvc2002\\bin\\vcvars32.bat >NUL && %CMAKE_EXE_WIN% -S Z:\\work -B Z:\\work\\build-msvc -DCMAKE_BUILD_TYPE=Release -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY"
                WINEPREFIX=/work/build/wineprefix \
                  wine cmd /c "call Z:\\home\\msvc2002\\bin\\vcvars32.bat >NUL && %CMAKE_EXE_WIN% --build Z:\\work\\build-msvc"
              '
          '');
        };

        msvc-build-debug = {
          type = "app";
          program = toString (pkgs.writeShellScript "msvc-build-debug" ''
            set -euo pipefail

            docker="${pkgs.docker}/bin/docker"
            uid="$(id -u)"
            gid="$(id -g)"

            rootless_docker=false
            if [ -z "''${DOCKER_HOST:-}" ]; then
              if [ -n "''${XDG_RUNTIME_DIR:-}" ] && [ -S "''${XDG_RUNTIME_DIR}/docker.sock" ]; then
                export DOCKER_HOST="unix://''${XDG_RUNTIME_DIR}/docker.sock"
                rootless_docker=true
              elif [ -S "/run/user/$uid/docker.sock" ]; then
                export DOCKER_HOST="unix:///run/user/$uid/docker.sock"
                rootless_docker=true
              fi
            else
              case "''${DOCKER_HOST}" in
                "unix://''${XDG_RUNTIME_DIR:-}/docker.sock"|"unix:///run/user/$uid/docker.sock")
                  rootless_docker=true
                  ;;
              esac
            fi

            prep_user_arg="--user 0:0"
            docker_user_arg="--user $uid:$gid"

            if ! "$docker" info >/dev/null 2>&1; then
              cat >&2 <<EOF
Docker daemon not reachable.

You're using rootless Docker on NixOS. Try:
  systemctl --user enable --now docker

If needed, set:
  export DOCKER_HOST=unix:///run/user/$uid/docker.sock

Then re-run:
  nix run .#msvc-build-debug
EOF
              exit 1
            fi

            curl="${pkgs.curl}/bin/curl"
            unzip="${pkgs.unzip}/bin/unzip"

            cmakeVersion="3.24.4"
            cmakeDir="build/tools/cmake-''${cmakeVersion}-windows-i386"
            cmakeExe="$PWD/$cmakeDir/bin/cmake.exe"

            if [ ! -x "$cmakeExe" ]; then
              "${pkgs.coreutils}/bin/mkdir" -p "build/tools"
              tmpZip="build/tools/cmake-''${cmakeVersion}-windows-i386.zip"
              "$curl" -L -o "$tmpZip" \
                "https://github.com/Kitware/CMake/releases/download/v$cmakeVersion/cmake-$cmakeVersion-windows-i386.zip"
              "$unzip" -q "$tmpZip" -d "build/tools"
            fi

            cmakeExeWin="Z:\\work\\build\\tools\\cmake-$cmakeVersion-windows-i386\\bin\\cmake.exe"

            "$docker" pull ${msvcImage} >/dev/null

            chown_cmd=""
            if [ "$rootless_docker" != true ]; then
              chown_cmd=" && chown -R $uid:$gid /work/build-msvc /work/obj /work/build /work/build/wineprefix"
            fi

            "$docker" run --rm -t \
              $prep_user_arg \
              -v "$PWD:/work" -w /work \
              ${msvcImage} \
              -lc "mkdir -p /work/build-msvc /work/obj /work/build /work/build/wineprefix && if [ ! -f /work/build/wineprefix/system.reg ]; then cp -a --no-preserve=ownership /home/msvc2002/.wine32/. /work/build/wineprefix/; fi && chmod -R a+rwX /work/build-msvc /work/obj /work/build /work/build/wineprefix$chown_cmd"

            "$docker" run --rm -t \
              $docker_user_arg \
              -e CMAKE_EXE_WIN="$cmakeExeWin" \
              -v "$PWD:/work" -w /work \
              ${msvcImage} \
              -lc '
                WINEPREFIX=/work/build/wineprefix \
                  wine cmd /c "call Z:\\home\\msvc2002\\bin\\vcvars32.bat >NUL && %CMAKE_EXE_WIN% -S Z:\\work -B Z:\\work\\build-msvc -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY"
                WINEPREFIX=/work/build/wineprefix \
                  wine cmd /c "call Z:\\home\\msvc2002\\bin\\vcvars32.bat >NUL && %CMAKE_EXE_WIN% --build Z:\\work\\build-msvc"
              '
          '');
        };

        msvc-build-tracing = {
          type = "app";
          program = toString (pkgs.writeShellScript "msvc-build-tracing" ''
            set -euo pipefail

            docker="${pkgs.docker}/bin/docker"
            uid="$(id -u)"
            gid="$(id -g)"

            rootless_docker=false
            if [ -z "''${DOCKER_HOST:-}" ]; then
              if [ -n "''${XDG_RUNTIME_DIR:-}" ] && [ -S "''${XDG_RUNTIME_DIR}/docker.sock" ]; then
                export DOCKER_HOST="unix://''${XDG_RUNTIME_DIR}/docker.sock"
                rootless_docker=true
              elif [ -S "/run/user/$uid/docker.sock" ]; then
                export DOCKER_HOST="unix:///run/user/$uid/docker.sock"
                rootless_docker=true
              fi
            else
              case "''${DOCKER_HOST}" in
                "unix://''${XDG_RUNTIME_DIR:-}/docker.sock"|"unix:///run/user/$uid/docker.sock")
                  rootless_docker=true
                  ;;
              esac
            fi

            prep_user_arg="--user 0:0"
            docker_user_arg="--user $uid:$gid"

            if ! "$docker" info >/dev/null 2>&1; then
              cat >&2 <<EOF
Docker daemon not reachable.

You're using rootless Docker on NixOS. Try:
  systemctl --user enable --now docker

If needed, set:
  export DOCKER_HOST=unix:///run/user/$uid/docker.sock

Then re-run:
  nix run .#msvc-build-tracing
EOF

              exit 1
            fi

            curl="${pkgs.curl}/bin/curl"
            unzip="${pkgs.unzip}/bin/unzip"

            cmakeVersion="3.24.4"
            cmakeDir="build/tools/cmake-''${cmakeVersion}-windows-i386"
            cmakeExe="$PWD/$cmakeDir/bin/cmake.exe"

            if [ ! -x "$cmakeExe" ]; then
              "${pkgs.coreutils}/bin/mkdir" -p "build/tools"
              tmpZip="build/tools/cmake-''${cmakeVersion}-windows-i386.zip"
              "$curl" -L -o "$tmpZip" \
                "https://github.com/Kitware/CMake/releases/download/v$cmakeVersion/cmake-$cmakeVersion-windows-i386.zip"
              "$unzip" -q "$tmpZip" -d "build/tools"
            fi

            cmakeExeWin="Z:\\work\\build\\tools\\cmake-$cmakeVersion-windows-i386\\bin\\cmake.exe"

            "$docker" pull ${msvcImage} >/dev/null

            chown_cmd=""
            if [ "$rootless_docker" != true ]; then
              chown_cmd=" && chown -R $uid:$gid /work/build-msvc-tracing /work/obj /work/build /work/build/wineprefix"
            fi

            "$docker" run --rm -t \
              $prep_user_arg \
              -v "$PWD:/work" -w /work \
              ${msvcImage} \
              -lc "mkdir -p /work/build-msvc-tracing /work/obj /work/build /work/build/wineprefix && if [ ! -f /work/build/wineprefix/system.reg ]; then cp -a --no-preserve=ownership /home/msvc2002/.wine32/. /work/build/wineprefix/; fi && chmod -R a+rwX /work/build-msvc-tracing /work/obj /work/build /work/build/wineprefix$chown_cmd"

            "$docker" run --rm -t \
              $docker_user_arg \
              -e CMAKE_EXE_WIN="$cmakeExeWin" \
              -v "$PWD:/work" -w /work \
              ${msvcImage} \
              -lc '
                WINEPREFIX=/work/build/wineprefix \
                  wine cmd /c "call Z:\\home\\msvc2002\\bin\\vcvars32.bat >NUL && %CMAKE_EXE_WIN% -S Z:\\work -B Z:\\work\\build-msvc-tracing -DCMAKE_BUILD_TYPE=Release -DOPENVB_TRACING=ON -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY"
                WINEPREFIX=/work/build/wineprefix \
                  wine cmd /c "call Z:\\home\\msvc2002\\bin\\vcvars32.bat >NUL && %CMAKE_EXE_WIN% --build Z:\\work\\build-msvc-tracing"
              '
          '');
        };

        f3-run = {
          type = "app";
          program = toString (pkgs.writeShellScript "f3-run" ''
            set -euo pipefail

            wine="${pkgs.wineWowPackages.stable}/bin/wine"
            wineboot="${pkgs.wineWowPackages.stable}/bin/wineboot"

            exePath="$PWD/build/F3.exe"
            if [ ! -f "$exePath" ]; then
              cat >&2 <<EOF
Missing build/F3.exe.

Build first with:
  nix run .#msvc-build
EOF
              exit 1
            fi

            if [ -z "''${WINEPREFIX:-}" ]; then
              if [ -n "''${XDG_DATA_HOME:-}" ]; then
                export WINEPREFIX="''${XDG_DATA_HOME}/openvb/wineprefix"
              else
                export WINEPREFIX="$HOME/.local/share/openvb/wineprefix"
              fi
            fi

            "${pkgs.coreutils}/bin/mkdir" -p "$WINEPREFIX"

            if [ -z "''${WINEARCH:-}" ]; then
              export WINEARCH=win32
            fi

            if [ -z "''${WINEDLLOVERRIDES:-}" ]; then
              export WINEDLLOVERRIDES="mscoree,mshtml="
            fi

            if [ -z "''${WINEDEBUG:-}" ]; then
              export WINEDEBUG=-all,+err
            fi

            if [ ! -f "$WINEPREFIX/system.reg" ]; then
              "$wineboot" -u >/dev/null
            fi

            exec "$wine" "$exePath"
          '');
        };

        f3-run-debug = {
          type = "app";
          program = toString (pkgs.writeShellScript "f3-run-debug" ''
            set -euo pipefail

            wine="${pkgs.wineWowPackages.stable}/bin/wine"
            wineboot="${pkgs.wineWowPackages.stable}/bin/wineboot"

            exePath="$PWD/build/F3-trace.exe"
            if [ ! -f "$exePath" ]; then
              cat >&2 <<EOF
Missing build/F3-trace.exe.

Build first with:
  nix run .#msvc-build-tracing
EOF
              exit 1
            fi

            if [ -z "''${WINEPREFIX:-}" ]; then
              if [ -n "''${XDG_DATA_HOME:-}" ]; then
                export WINEPREFIX="''${XDG_DATA_HOME}/openvb/wineprefix"
              else
                export WINEPREFIX="$HOME/.local/share/openvb/wineprefix"
              fi
            fi

            "${pkgs.coreutils}/bin/mkdir" -p "$WINEPREFIX"

            if [ -z "''${WINEARCH:-}" ]; then
              export WINEARCH=win32
            fi

            if [ -z "''${WINEDLLOVERRIDES:-}" ]; then
              export WINEDLLOVERRIDES="mscoree,mshtml="
            fi

            if [ -z "''${WINEDEBUG:-}" ]; then
              export WINEDEBUG=-all,+err
            fi

            if [ ! -f "$WINEPREFIX/system.reg" ]; then
              "$wineboot" -u >/dev/null
            fi

            exec "$wine" "$exePath"
          '');
        };
      };
    };
  };
}
