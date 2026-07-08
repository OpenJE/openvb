# ./devenv/scripts/msvc-build.nix

{ config, pkgs, ... }:
let
  cmakeVersion = config.env.OPENVB_CMAKE_VERSION;
  msvcImage = config.env.MSVC_IMAGE;
in {
  scripts.msvc-build = {
    description = "Build OpenVB with MSVC release";
    exec = /* bash */ ''
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

      if [ "$rootless_docker" = true ]; then
        # In rootless Docker, container root maps to the host user.
        # Keep Wine prefix ownership consistent inside the container.
        prep_user_arg="--user 0:0"
        docker_user_arg="--user 0:0"
      else
        # In rootful Docker, avoid root-owned build artifacts on the host.
        prep_user_arg="--user 0:0"
        docker_user_arg="--user $uid:$gid"
      fi

      if ! "$docker" info >/dev/null 2>&1; then
        printf '%s\n' \
          "Docker daemon not reachable." \
          "" \
          "You're using rootless Docker on NixOS. Try:" \
          "  systemctl --user enable --now docker" \
          "" \
          "If needed, set:" \
          "  export DOCKER_HOST=unix:///run/user/$uid/docker.sock" \
          "" \
          "Then re-run:" \
          "  msvc-build" >&2
          exit 1
      fi

      curl="${pkgs.curl}/bin/curl"
      unzip="${pkgs.unzip}/bin/unzip"

      cmakeDir="build/tools/cmake-''${OPENVB_CMAKE_VERSION}-windows-i386"
      cmakeExe="$PWD/$cmakeDir/bin/cmake.exe"

      if [ ! -x "$cmakeExe" ]; then
        "${pkgs.coreutils}/bin/mkdir" -p "build/tools"
        tmpZip="build/tools/cmake-''${OPENVB_CMAKE_VERSION}-windows-i386.zip"
        "$curl" -L -o "$tmpZip" \
          "https://github.com/Kitware/CMake/releases/download/v${cmakeVersion}/cmake-${cmakeVersion}-windows-i386.zip"
        "$unzip" -q "$tmpZip" -d "build/tools"
      fi

      cmakeExeWin="Z:\\work\\build\\tools\\cmake-${cmakeVersion}-windows-i386\\bin\\cmake.exe"

      # Remove stale reimpl IDB so IDA regenerates from fresh EXE
      rm -f "$OPENVB_REIMPL_IDB"

      "$docker" pull "${msvcImage}" >/dev/null

      chown_cmd=""
      if [ "$rootless_docker" != true ]; then
        chown_cmd=" && chown -R $uid:$gid /work/build-msvc /work/obj /work/build /work/build/wineprefix"
      fi

      "$docker" run --rm -t \
        --cap-add=NET_RAW \
        --security-opt seccomp=unconfined \
        $prep_user_arg \
        -v "$PWD:/work" -w /work \
        "${msvcImage}" \
        -lc "mkdir -p /work/build-msvc /work/obj /work/build /work/build/wineprefix && if [ ! -f /work/build/wineprefix/system.reg ]; then cp -a --no-preserve=ownership /home/msvc2002/.wine32/. /work/build/wineprefix/; fi && chmod -R a+rwX /work/build-msvc /work/obj /work/build /work/build/wineprefix$chown_cmd"

      "$docker" run --rm -t \
        --cap-add=NET_RAW \
        --security-opt seccomp=unconfined \
        $docker_user_arg \
        -e CMAKE_EXE_WIN="$cmakeExeWin" \
        -v "$PWD:/work" -w /work \
        "${msvcImage}" \
        -lc '
          WINEPREFIX=/work/build/wineprefix \
            wine cmd /c "call Z:\\home\\msvc2002\\bin\\vcvars32.bat >NUL && %CMAKE_EXE_WIN% -S Z:\\work -B Z:\\work\\build-msvc -DCMAKE_BUILD_TYPE=Release -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY -DCMKR_SKIP_GENERATION=ON"
          WINEPREFIX=/work/build/wineprefix \
            wine cmd /c "call Z:\\home\\msvc2002\\bin\\vcvars32.bat >NUL && %CMAKE_EXE_WIN% --build Z:\\work\\build-msvc"
        '
    '';
  };
}
