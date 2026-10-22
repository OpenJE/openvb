# ./devenv/scripts/f3-wine-dxvk.nix

{ config, pkgs, ... }:
let
  wine = "${pkgs.wine}/bin/wine";
  dxvkX32 = config.env.OPENVB_DXVK_X32;
  wineX32 = "${pkgs.wine}/lib/wine/i386-windows";
in {
  scripts.f3-wine-dxvk = {
    description = "Report/arm/disarm the DXVK x32 render override in a Wine prefix (f3-wine-dxvk status|arm|disarm [prefix])";
    exec = /* bash */ ''
      set -euo pipefail

      # WHY THIS EXISTS. M-001/T-067 armed the render lane by hand: 3 DXVK x32 dlls into
      # system32 + HKCU DllOverrides d3d8=n,d3d9=n (dxgi deliberately WITHOUT a reg line, matching
      # the working demo prefix). Hand-armed state does not survive a prefix recreation: the
      # 2026-10-21 rig rebuild left a prefix whose DllOverrides key was EMPTY while three dlls
      # named d3d8/d3d9/dxgi.dll sat in system32 - so the prefix LOOKED armed and was not. Worse,
      # those files were not DXVK at all: their md5 equals Wine's own builtin i386-windows proxies,
      # i.e. the "DXVK install" had become a no-op copy of the builtin. A file named d3d8.dll
      # proves nothing; only a hash against a declared source does - and the source has to be
      # DECLARED, because the ad-hoc /nix/store path the rig recorded was GC'd by 2026-10-22
      # (`nix build nixpkgs#dxvk` re-fetched it byte-identical). Hence OPENVB_DXVK_X32.
      #
      # Verdicts: ARMED (3 dlls hash-match DXVK + both overrides =n) / INERT (dlls present,
      # override missing - builtin wins, DXVK never loads) / IMPOSTOR (system32 holds Wine
      # builtins) / CLEAN (nothing installed) / PARTIAL. status always exits 0.
      #
      # usage: f3-wine-dxvk status            # $WINEPREFIX, else the shared reimpl prefix
      #        f3-wine-dxvk arm   [prefix]    # install dlls + overrides, then re-report
      #        f3-wine-dxvk disarm [prefix]   # remove both (the law-55 clean-revert state)

      cmd="''${1:-status}"
      case "$cmd" in
        status|arm|disarm) ;;
        -h|--help) sed -n '12,33p' "$0"; exit 0 ;;
        *) printf 'f3-wine-dxvk: unknown command: %s (status|arm|disarm)\n' "$cmd" >&2; exit 2 ;;
      esac

      stateDir="$PWD/.devenv/state"
      prefix="''${2:-''${WINEPREFIX:-$stateDir/f3-reimpl-wineprefix}}"
      sys32="$prefix/drive_c/windows/system32"
      if [ ! -d "$sys32" ]; then
        printf 'f3-wine-dxvk: no prefix at %s (run an f3-*-run script first)\n' "$prefix" >&2
        exit 1
      fi

      export WINEPREFIX="$prefix"
      export WINEARCH="''${WINEARCH:-win32}"
      export WINEDLLOVERRIDES="''${WINEDLLOVERRIDES:-mscoree,mshtml=}"
      export WINEDEBUG="''${WINEDEBUG:--all}"

      DLLS=(d3d8.dll d3d9.dll dxgi.dll)
      OVERRIDES=(d3d8 d3d9)   # dxgi stays unregistered ON PURPOSE (the demo prefix has no dxgi line)

      md5of() { "${pkgs.coreutils}/bin/md5sum" "$1" 2>/dev/null | cut -d' ' -f1 || true; }
      src_md5() { md5of "${dxvkX32}/$1"; }
      builtin_md5() { md5of "${wineX32}/$1"; }

      # Classify a system32 copy by HASH against the declared sources, never by name.
      dll_kind() {
        local here
        here="$(md5of "$sys32/$1")"
        if [ -z "$here" ]; then
          printf 'ABSENT'
        elif [ "$here" = "$(src_md5 "$1")" ]; then
          printf 'DXVK'
        elif [ "$here" = "$(builtin_md5 "$1")" ]; then
          printf 'WINE-BUILTIN(impostor)'
        else
          printf 'UNKNOWN(%s)' "$here"
        fi
      }

      override_of() {
        local v
        v="$("${wine}" reg query 'HKCU\Software\Wine\DllOverrides' /v "$1" 2>/dev/null | grep 'REG_SZ' | sed 's/.*REG_SZ[[:space:]]*//' | tr -d '[:space:]' || true)"
        if [ -z "$v" ]; then
          printf 'none'
        else
          printf '%s' "$v"
        fi
      }

      report() {
        local d kinds=0 overrides=0 any=0
        printf 'prefix: %s\n' "$prefix"
        printf 'dxvk source: %s\n' "${dxvkX32}"
        for d in "''${DLLS[@]}"; do
          printf '  %-9s %-22s dxvk=%s winebuiltin=%s\n' "$d" "$(dll_kind "$d")" "$(src_md5 "$d")" "$(builtin_md5 "$d")"
        done
        for d in "''${OVERRIDES[@]}"; do
          printf '  override %-5s = %s\n' "$d" "$(override_of "$d")"
        done
        for d in "''${DLLS[@]}"; do
          if [ "$(dll_kind "$d")" = DXVK ]; then kinds=$((kinds+1)); fi
          if [ "$(dll_kind "$d")" != ABSENT ]; then any=1; fi
        done
        for d in "''${OVERRIDES[@]}"; do
          if [ "$(override_of "$d")" = n ]; then overrides=$((overrides+1)); fi
        done
        if [ "$kinds" = 3 ] && [ "$overrides" = 2 ]; then
          printf 'verdict: ARMED (DXVK d3d8/d3d9/dxgi + {d3d8=n,d3d9=n})\n'
        elif [ "$kinds" = 3 ]; then
          printf 'verdict: INERT - dlls are DXVK but DllOverrides is missing: Wine builtin wins and DXVK NEVER loads\n'
        elif [ "$kinds" = 0 ] && [ "$overrides" = 0 ]; then
          if [ "$any" = 1 ]; then
            printf 'verdict: IMPOSTOR - system32 holds copies that are not DXVK (looks armed, is not)\n'
          else
            printf 'verdict: CLEAN (no DXVK dlls, no overrides)\n'
          fi
        else
          printf 'verdict: PARTIAL - %s/3 DXVK dlls, %s/2 overrides\n' "$kinds" "$overrides"
        fi
      }

      do_arm() {
        local d
        for d in "''${DLLS[@]}"; do
          if [ ! -f "${dxvkX32}/$d" ]; then
            printf 'f3-wine-dxvk: DXVK source missing for %s (nix build nixpkgs#dxvk)\n' "$d" >&2
            exit 1
          fi
          "${pkgs.coreutils}/bin/cp" -f "${dxvkX32}/$d" "$sys32/$d"
          printf 'installed %-9s md5 %s\n' "$d" "$(md5of "$sys32/$d")"
        done
        for d in "''${OVERRIDES[@]}"; do
          "${wine}" reg add 'HKCU\Software\Wine\DllOverrides' /v "$d" /t REG_SZ /d n /f >/dev/null
          printf 'override %s=n\n' "$d"
        done
        printf 'dxgi deliberately left UNREGISTERED (matches the working demo prefix)\n'
      }

      do_disarm() {
        local d kind
        for d in "''${OVERRIDES[@]}"; do
          "${wine}" reg delete 'HKCU\Software\Wine\DllOverrides' /v "$d" /f >/dev/null 2>&1 || true
          printf 'override %s deleted\n' "$d"
        done
        for d in "''${DLLS[@]}"; do
          kind="$(dll_kind "$d")"
          case "$kind" in
            ABSENT) ;;
            *) "${pkgs.coreutils}/bin/rm" -f "$sys32/$d"; printf 'removed %-9s (%s)\n' "$d" "$kind" ;;
          esac
        done
      }

      case "$cmd" in
        arm) do_arm; printf '\n'; report ;;
        disarm) do_disarm; printf '\n'; report ;;
        *) report ;;
      esac
    '';
  };
}
