# ./devenv/scripts/f3-wine-dxvk.nix

{ config, pkgs, ... }:
let
  wine = "${pkgs.wine}/bin/wine";
  dxvkSrc = config.env.OPENVB_DXVK_X32;
  wineSrc = "${pkgs.wine}/lib/wine/i386-windows";
in {
  scripts.f3-wine-dxvk = {
    description = "Report/arm/disarm the DXVK x32 render override in a Wine prefix (f3-wine-dxvk status|arm|disarm [prefix])";
    exec = /* bash */ ''
      set -euo pipefail

      # WHY THIS EXISTS. M-001/T-067 armed the render lane by hand: 3 DXVK x32 dlls into
      # system32 + HKCU DllOverrides d3d8=n,d3d9=n (dxgi deliberately WITHOUT a reg line, matching
      # the working demo prefix). Hand-armed state does not survive a prefix recreation, and a
      # PREFIX CONTAINS d3d8/d3d9/dxgi.dll COPIES IN THE PRISTINE STATE TOO - measured 2026-10-22:
      # a fresh `wineboot -u` ships system32 copies byte-identical to Wine's own i386-windows
      # builtins. So file PRESENCE is never evidence of an install, and the only two signals that
      # mean anything are (a) the hash against a DECLARED source and (b) the registry override.
      # The declared source is env.OPENVB_DXVK_X32 (''${pkgs.dxvk.bin}/x32), not a pasted /nix/store
      # path: the ad-hoc path T-067 recorded had been GC'd by 2026-10-22 and `nix build
      # nixpkgs#dxvk` re-fetched it byte-identical.
      #
      # SELF-DEFECTS FIXED IN THIS VERSION (caught by JadeBear 2026-10-22, root cause corrected by
      # measurement - see docs/notes/harness-items.md item 68):
      #  1. A RELATIVE prefix argument was accepted verbatim. `[ -d "$sys32" ]` resolves against the
      #     CWD and PASSES, while wine refuses WINEPREFIX=relative and exits - every registry read
      #     died, `|| true` swallowed it, the overrides printed "none", and the tool FABRICATED an
      #     INERT verdict off a wine exit code on a genuinely armed prefix. Now: absolute-ize with
      #     realpath, and require system.reg (which also rejects fake directories).
      #  2. `arm` copied its three dlls BEFORE touching the registry, so a registry failure plus
      #     set -e left dlls-without-overrides installed - the tool manufactured the exact INERT
      #     state it exists to detect, on precisely the CLEAN prefix someone re-arms after a
      #     recreation. Now: registry first, each write read back, dlls copied last and hash-checked.
      #  3. "absent" and "query failed" were the same string, so a broken read could not be told from
      #     an empty key. Now one cached query keeps its exit status; REG-ERROR refuses a verdict and
      #     exits nonzero (loud, and the run scripts pipe through `|| true` so it can never abort a boot).
      #  4. Read-back compares died on Wine's CRLF: `got` held "n\r" and != "n" while PRINTING "n"
      #     (live-caught on the scratch prefix). CR is stripped where the table is built.
      #
      # usage: f3-wine-dxvk status            # $WINEPREFIX, else the shared reimpl prefix
      #        f3-wine-dxvk arm   [prefix]    # install + overrides, then re-report
      #        f3-wine-dxvk disarm [prefix]   # remove both (the law-55 clean-revert state)

      cmd="''${1:-status}"
      case "$cmd" in
        status|arm|disarm) ;;
        -h|--help) sed -n '12,44p' "$0"; exit 0 ;;
        *) printf 'f3-wine-dxvk: unknown command: %s (status|arm|disarm)\n' "$cmd" >&2; exit 2 ;;
      esac

      stateDir="$PWD/.devenv/state"
      prefix="''${2:-''${WINEPREFIX:-$stateDir/f3-reimpl-wineprefix}}"
      prefix="''${prefix:+$("${pkgs.coreutils}/bin/realpath" -m -- "$prefix")}"
      sys32="$prefix/drive_c/windows/system32"
      if [ ! -d "$sys32" ] || [ ! -f "$prefix/system.reg" ]; then
        printf 'f3-wine-dxvk: no initialized prefix at %s (run an f3-*-run script first)\n' "$prefix" >&2
        exit 1
      fi

      # One bash variable per declared source: the braces are NIX's, so the body never
      # writes a dollar-brace at all, so a path can never silently become an unbound bash var
      DXVK_SRC="${dxvkSrc}"
      WINE_SRC="${wineSrc}"

      export WINEPREFIX="$prefix"
      export WINEARCH="''${WINEARCH:-win32}"
      export WINEDLLOVERRIDES="''${WINEDLLOVERRIDES:-mscoree,mshtml=}"
      export WINEDEBUG="''${WINEDEBUG:--all}"

      DLLS=(d3d8.dll d3d9.dll dxgi.dll)
      OVERRIDES=(d3d8 d3d9)   # dxgi stays unregistered ON PURPOSE (the working demo prefix has no dxgi line)

      md5of() { "${pkgs.coreutils}/bin/md5sum" "$1" 2>/dev/null | cut -d' ' -f1 || true; }
      src_md5() { md5of ""$DXVK_SRC"/$1"; }
      builtin_md5() { md5of ""$WINE_SRC"/$1"; }

      # Classify a system32 copy by HASH against the declared sources, never by name.
      dll_kind() {
        local here
        here="$(md5of "$sys32/$1")"
        if [ -z "$here" ]; then
          printf 'ABSENT'
        elif [ "$here" = "$(src_md5 "$1")" ]; then
          printf 'DXVK'
        elif [ "$here" = "$(builtin_md5 "$1")" ]; then
          printf 'WINE-BUILTIN'
        else
          printf 'UNKNOWN(%s)' "$here"
        fi
      }

      # One cached read of the whole key, WITH its exit status: a failed query is not an empty key.
      REG_OK=1
      REG_TABLE=""
      reg_refresh() {
        local out
        if out="$("${wine}" reg query 'HKCU\Software\Wine\DllOverrides' 2>&1)"; then
          REG_OK=0
        else
          REG_OK=1
        fi
        REG_TABLE="$(printf '%s\n' "$out" | tr -d '\r' | grep 'REG_SZ' || true)"
      }

      override_of() {
        if [ "$REG_OK" -ne 0 ]; then
          printf 'QUERY-FAILED'
          return
        fi
        local hit
        hit="$(printf '%s\n' "$REG_TABLE" | awk -v n="$1" '$1==n {print $3; exit}')"
        if [ -z "$hit" ]; then
          printf 'absent'
        else
          printf '%s' "$hit"
        fi
      }

      report() {
        local d kinds=0 overrides=0 builtins=0 unknowns=0 present=0
        printf 'prefix: %s\n' "$prefix"
        printf 'dxvk source: %s\n' ""$DXVK_SRC""
        for d in "''${DLLS[@]}"; do
          printf '  %-9s %-22s dxvk=%s winebuiltin=%s\n' "$d" "$(dll_kind "$d")" "$(src_md5 "$d")" "$(builtin_md5 "$d")"
        done
        reg_refresh
        for d in "''${OVERRIDES[@]}"; do
          printf '  override %-5s = %s\n' "$d" "$(override_of "$d")"
        done
        if [ "$REG_OK" -ne 0 ]; then
          printf 'verdict: REG-ERROR - wine could not read DllOverrides; no verdict is safe, refusing\n' >&2
          return 1
        fi
        for d in "''${DLLS[@]}"; do
          case "$(dll_kind "$d")" in
            DXVK) kinds=$((kinds+1)); present=$((present+1)) ;;
            WINE-BUILTIN) builtins=$((builtins+1)); present=$((present+1)) ;;
            ABSENT) ;;
            *) unknowns=$((unknowns+1)); present=$((present+1)) ;;
          esac
        done
        for d in "''${OVERRIDES[@]}"; do
          if [ "$(override_of "$d")" = n ]; then overrides=$((overrides+1)); fi
        done
        if [ "$kinds" = 3 ] && [ "$overrides" = 2 ]; then
          printf 'verdict: ARMED (DXVK d3d8/d3d9/dxgi + {d3d8=n,d3d9=n})\n'
        elif [ "$kinds" = 3 ]; then
          printf 'verdict: INERT - dlls hash-match DXVK but the overrides are missing: Wine builtin wins, DXVK never loads\n'
        elif [ "$unknowns" != 0 ]; then
          printf 'verdict: UNKNOWN - %s copy/copies match NEITHER the declared DXVK source nor Wine builtins; refusing to guess\n' "$unknowns"
        elif [ "$builtins" != 0 ] && [ "$overrides" = 0 ]; then
          printf 'verdict: DEFAULT-BUILTINS - pristine wineboot state: Wine ships these copies itself, so presence proves nothing; no DXVK, no overrides\n'
        elif [ "$builtins" != 0 ]; then
          printf 'verdict: HALF-ARMED - only Wine builtin copies present but override(s) set (%s builtin dlls, %s overrides): the native load cannot come from these files\n' "$builtins" "$overrides"
        elif [ "$present" = 0 ] && [ "$overrides" = 0 ]; then
          printf 'verdict: CLEAN (no dll copies at all, no overrides)\n'
        else
          printf 'verdict: PARTIAL - %s/3 DXVK dlls, %s builtin dlls, %s/2 overrides\n' "$kinds" "$builtins" "$overrides"
        fi
        return 0
      }

      do_arm() {
        local d got
        for d in "''${DLLS[@]}"; do
          if [ ! -f ""$DXVK_SRC"/$d" ]; then
            printf 'f3-wine-dxvk: DXVK source missing for %s (nix build nixpkgs#dxvk)\n' "$d" >&2
            exit 1
          fi
        done
        # Registry FIRST (see defect 2): a failure here leaves the prefix exactly as found.
        for d in "''${OVERRIDES[@]}"; do
          if ! "${wine}" reg add 'HKCU\Software\Wine\DllOverrides' /v "$d" /t REG_SZ /d n /f >/dev/null 2>&1; then
            printf 'f3-wine-dxvk: reg add %s=n FAILED - aborting BEFORE copying dlls, prefix left as found\n' "$d" >&2
            exit 1
          fi
        done
        reg_refresh
        if [ "$REG_OK" -ne 0 ]; then
          printf 'f3-wine-dxvk: overrides written but unreadable - aborting, dlls NOT copied\n' >&2
          exit 1
        fi
        for d in "''${OVERRIDES[@]}"; do
          got="$(override_of "$d")"
          if [ "$got" != n ]; then
            printf 'f3-wine-dxvk: read-back of %s gave "%s", expected "n" - aborting, dlls NOT copied\n' "$d" "$got" >&2
            exit 1
          fi
          printf 'override %s=n (read back)\n' "$d"
        done
        for d in "''${DLLS[@]}"; do
          "${pkgs.coreutils}/bin/cp" -f ""$DXVK_SRC"/$d" "$sys32/$d"
          if [ "$(md5of "$sys32/$d")" != "$(src_md5 "$d")" ]; then
            printf 'f3-wine-dxvk: post-copy hash mismatch for %s\n' "$d" >&2
            exit 1
          fi
          printf 'installed %-9s md5 %s\n' "$d" "$(md5of "$sys32/$d")"
        done
        printf 'dxgi deliberately left UNREGISTERED (matches the working demo prefix)\n'
      }

      do_disarm() {
        local d kind
        for d in "''${OVERRIDES[@]}"; do
          if ! "${wine}" reg delete 'HKCU\Software\Wine\DllOverrides' /v "$d" /f >/dev/null 2>&1; then
            printf 'f3-wine-dxvk: reg delete %s FAILED - aborting, dlls left in place\n' "$d" >&2
            exit 1
          fi
          printf 'override %s deleted\n' "$d"
        done
        reg_refresh
        for d in "''${OVERRIDES[@]}"; do
          if [ "$REG_OK" -eq 0 ] && [ "$(override_of "$d")" != absent ]; then
            printf 'f3-wine-dxvk: %s still present after delete\n' "$d" >&2
            exit 1
          fi
        done
        for d in "''${DLLS[@]}"; do
          kind="$(dll_kind "$d")"
          if [ "$kind" != ABSENT ]; then
            # Only copies this tool's model can explain are removed; a pristine prefix's OWN builtin
            # copies are Wine's layout, so disarm leaves them (it reverts OUR install, not Wine).
            case "$kind" in
              DXVK|UNKNOWN*)
                "${pkgs.coreutils}/bin/rm" -f "$sys32/$d"; printf 'removed %-9s (%s)\n' "$d" "$kind" ;;
              WINE-BUILTIN)
                printf 'left %-11s (Wine ships this copy itself; disarm reverts OUR install, not Wine)\n' "$d" ;;
              *)
                printf 'left %-11s (%s)\n' "$d" "$kind" ;;
            esac
          fi
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
