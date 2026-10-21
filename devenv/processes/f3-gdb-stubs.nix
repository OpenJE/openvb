# ./devenv/processes/f3-gdb-stubs.nix

{ config, pkgs, ... }:
let
  wineboot = "${pkgs.wine}/bin/wineboot";
  winedbg = "${pkgs.wine}/bin/winedbg";

  # OWNER-VERIFIED readiness probe, shipped as a script with its own shebang so it behaves the
  # same whether or not process-compose runs `ready.exec` through a shell.
  #
  # WHY: the previous form was `nc -z 127.0.0.1 <port>`, which passes against ANY listener on the
  # port. ImHex's Network Interface was configured to the same 31337 as the original's gdb stub, so
  # winedbg lost the bind and exited, while devenv reported "ida-orig-gdb: ready" - a green that
  # manufactured a whole investigation of "cannot attach" (NiceHawk, T-034 handover b77625e).
  #
  # 2026-10-21: the original stub is the side that could actually move (OPENVB_ORIG_GDB_PORT is now
  # 31437). ImHex's listen port is hardcoded to 31337 in the binary, so "move ImHex to 31347" - the
  # first fix - only broke the imhex process; it kept squatting 31337 and starved this stub anyway.
  # See devenv/environment/openvb.nix for the measurement.
  # A readiness probe that cannot tell "someone is listening" from "the thing I started is
  # listening" is worse than no probe.
  # BOUNDARY, measured: this proves the port is held by a WINE process rather than a squatter.
  # It does NOT prove the stub is attachable - after winedbg was killed, wineserver kept holding
  # 31558 and the probe still said ready. So it is strictly better than `nc -z` (which passed for
  # ANY listener, including the ImHex one that hid a dead stub) but is not a liveness test, and a
  # lane that gets "ready" followed by a failed attach should suspect a lingering wineserver, not a
  # misreported port.
  stubReady = pkgs.writeShellScriptBin "gdb-stub-ready" ''
    port="''${1:?usage: gdb-stub-ready <port>}"
    # NOTE (measured, not assumed): the gdb listening socket is reported by `ss` as
    # **wineserver**, not winedbg - winedbg hands the socket to wineserver. A probe grepping only
    # "winedbg" therefore reports NOT READY against a perfectly healthy stub: a false red, which is
    # the exact failure this file exists to prevent. Accept both spellings.
    "${pkgs.iproute2}/bin/ss" -H -ltnp "sport = :''${port}" \
      | "${pkgs.gnugrep}/bin/grep" -Eq "wineserver|winedbg"
  '';
in {
  processes.ida-orig-gdb = {
    exec = /* bash */ ''
      set -euo pipefail

      stateDir="$PWD/.devenv/state"
      prefix="$stateDir/f3demo-wineprefix"
      demoRoot="${config.env.OPENVB_F3DEMO_ROOT}"
      gdbPort=''${OPENVB_ORIG_GDB_PORT:-${config.env.OPENVB_ORIG_GDB_PORT}}

      export WINEPREFIX="$prefix"
      export WINEARCH=''${WINEARCH:-win32}
      export WINEDLLOVERRIDES=''${WINEDLLOVERRIDES:-mscoree,mshtml=}
      export WINEDEBUG=''${WINEDEBUG:--all,+err}

      "${pkgs.coreutils}/bin/mkdir" -p "$prefix"

      if [ ! -f "$prefix/system.reg" ]; then
        "${wineboot}" -u >/dev/null
      fi

      printf '%s\n' \
        "Original demo GDB stub listening on 127.0.0.1:$gdbPort" \
        "Use IDA Remote GDB Debugger against the original demo IDB/database." \
        "The process will wait for the first debugger connection before starting."

      cd "$demoRoot"
      exec "${winedbg}" --gdb --port "$gdbPort" --no-start F3.exe
    '';
    cwd = config.git.root;
    # OWNER CHECK, not an "is the port answering" check. The previous `nc -z` form passed against
    # ANY listener on 31337 - including the ImHex Network Interface that used to share this port -
    # so devenv reported the gdb stub "ready" while winedbg had lost the bind and died. That
    # produced a full investigation of "attach is broken" when the honest signal was "your stub
    # never owned the socket" (NiceHawk, T-034 handover b77625e). A readiness probe that cannot
    # distinguish "someone is listening" from "the thing I started is listening" is worse than no
    # probe: it manufactures a green.
    ready.exec = "${stubReady}/bin/gdb-stub-ready ${config.env.OPENVB_ORIG_GDB_PORT}";
  };

  processes.ida-reimpl-gdb = {
    exec = /* bash */ ''
      set -euo pipefail

      exePath="$PWD/build/F3.exe"
      if [ ! -f "$exePath" ]; then
        printf '%s\n' \
          "Missing build/F3.exe." \
          "" \
          "Build first with:" \
          "  msvc-build" >&2
        exit 1
      fi

      stateDir="$PWD/.devenv/state"
      prefix="$stateDir/f3-reimpl-wineprefix"
      gdbPort=''${OPENVB_REIMPL_GDB_PORT:-${config.env.OPENVB_REIMPL_GDB_PORT}}

      export WINEPREFIX="$prefix"
      export WINEARCH=''${WINEARCH:-win32}
      export WINEDLLOVERRIDES=''${WINEDLLOVERRIDES:-mscoree,mshtml=}
      export WINEDEBUG=''${WINEDEBUG:--all,+err}

      "${pkgs.coreutils}/bin/mkdir" -p "$prefix"

      if [ ! -f "$prefix/system.reg" ]; then
        "${wineboot}" -u >/dev/null
      fi

      printf '%s\n' \
        "Reimplementation GDB stub listening on 127.0.0.1:$gdbPort" \
        "Use IDA Remote GDB Debugger against the rebuilt binary IDB/database." \
        "The process will wait for the first debugger connection before starting."

      exec "${winedbg}" --gdb --port "$gdbPort" --no-start "$exePath"
    '';
    cwd = config.git.root;
    # see the ORIG stub's comment: same owner-verified form (winedbg must hold the socket).
    ready.exec = "${stubReady}/bin/gdb-stub-ready ${config.env.OPENVB_REIMPL_GDB_PORT}";
  };
}