# ./devenv/scripts/run-all-tests.nix

{ pkgs, ... }:
let
  wine = "${pkgs.wine}/bin/wine";
  mingxCxx = "${pkgs.pkgsCross.mingw32.stdenv.cc}/bin/i686-w64-mingw32-g++";
  statBin = "${pkgs.coreutils}/bin/stat";
in {
  scripts.run-all-tests = {
    description = "Build and run every host test suite plus the app boot check (exit 124 = boot pass); one line per suite, nonzero exit on any failure";
    exec = /* bash */ ''
      set -uo pipefail

      # Build/run every host suite in modules/openje/tests/je. Source lists live HERE (not in the
      # test headers, which are stale/missing for 3 of 6 suites — see tasks/T-001 Notes and
      # docs/notes/implementer-B.md §T-001). Exes go to .devenv/state/tests/, so this script never
      # touches the shared tests/je/<Suite>/<Suite>.[AB].exe names (chat/from-A.md C-4).
      #
      # usage: run-all-tests [--only Suite[,Suite]] [--no-app] [--keep] [--timeout N] [-v]

      root="$PWD"
      if [ ! -d "modules/openje/tests/je" ]; then
        printf 'run-all-tests: run me from the repo root.\n' >&2
        exit 1
      fi

      outDir="$root/.devenv/state/tests"
      # State paths are overridable so the tool can be pointed at a git worktree of HEAD (to prove
      # green on committed code while the main checkout is mid-edit by the other implementer).
      wineprefix="''${OPENVB_TEST_WINEPREFIX:-$root/.devenv/state/f3-reimpl-wineprefix}"
      dataDir="''${OPENVB_TEST_DATA_DIR:-$root/.devenv/state/f3-reimpl-data}"
      dataArg='C:\f3data'          # C:\f3data inside the prefix (staged below if missing)
      timeout_s=6
      only=""
      want_app=1
      keep=0
      verbose=0

      while [ $# -gt 0 ]; do
        case "$1" in
          --only)   only="''${2:-}"; shift 2 ;;
          --no-app) want_app=0; shift ;;
          --keep)   keep=1; shift ;;
          --timeout) timeout_s="''${2:-6}"; shift 2 ;;
          -v|--verbose) verbose=1; shift ;;
          -h|--help) sed -n '2,12p' "$0"; exit 0 ;;
          *) printf 'run-all-tests: unknown argument: %s\n' "$1" >&2; exit 2 ;;
        esac
      done

      if [ ! -f "$wineprefix/system.reg" ]; then
        printf 'run-all-tests: wine prefix missing (%s). Run f3-reimpl-run-data once.\n' "$wineprefix" >&2
        exit 1
      fi

      # The data-based suites read their resource root through wine, so C:\f3data must exist in
      # the prefix. Stage it (read-only symlinks to the demo root) exactly like
      # f3-reimpl-run-data does, so this script works even if that script never ran here before.
      staging="$wineprefix/drive_c/f3data"
      demoRoot="''${OPENVB_F3DEMO_ROOT:-}"
      if [ ! -d "$staging" ] && [ -d "$dataDir" ] && [ -n "$demoRoot" ]; then
        "${pkgs.coreutils}/bin/mkdir" -p "$staging"
        for entry in "$demoRoot"/*; do
          "${pkgs.coreutils}/bin/ln" -sfn "$entry" "$staging/$(basename "$entry")"
        done
        printf 'run-all-tests: staged %s from the demo root\n' "$staging"
      fi
      if [ ! -d "$staging" ]; then
        printf 'run-all-tests: no data root (missing %s). Run f3-reimpl-run-data once.\n' "$staging" >&2
        exit 1
      fi

      export WINEPREFIX="$wineprefix"
      export WINEARCH="''${WINEARCH:-win32}"
      export WINEDLLOVERRIDES="''${WINEDLLOVERRIDES:-mscoree,mshtml=}"
      export WINEDEBUG="''${WINEDEBUG:--all}"

      JE="modules/openje/src/JE"
      SRC="modules/openje/src/JE.cpp"

      # name | test source | impl sources (space separated)
      # SpatialGrid.cpp is listed for every suite that links Map.cpp; the runner skips listed
      # sources that are not on disk yet, so the table stays valid whether or not that file has
      # landed (it is A-lane and Map.cpp references it). MapEntitySpawn.cpp is required by the suites
      # that link GamePathManager.cpp: F3::GetActiveMap() lives there (openje 18a8e00).
      suites=(
        "cls_0x4d8d70|modules/openje/tests/je/cls_0x4d8d70/cls_0x4d8d70.cpp|$JE/cls_0x4d8d70.cpp"
        "PathManager|modules/openje/tests/je/PathManager/PathManager.cpp|$JE/PathManager.cpp $JE/Map.cpp $JE/SpatialGrid.cpp $JE/ResourceSystem.cpp $JE/Common/Chunk.cpp $JE/Resources.cpp $JE/VFX_Parameter.cpp $JE/Subsystem.cpp $SRC $JE/World.cpp $JE/Action.cpp src/F3/GamePathManager.cpp src/F3/MapEntitySpawn.cpp"
        "ResourceSystem|modules/openje/tests/je/ResourceSystem/ResourceSystem.cpp|$JE/ResourceSystem.cpp $JE/Resources.cpp $JE/Map.cpp $JE/SpatialGrid.cpp $JE/VFX_Parameter.cpp $JE/Common/Chunk.cpp $JE/Subsystem.cpp $SRC"
        "StringSystem|modules/openje/tests/je/StringSystem/StringSystem.cpp|$JE/SS/StringSystem.cpp $JE/SS/EngineStringSystem.cpp $JE/ResourceSystem.cpp $JE/Resources.cpp $JE/Strings.cpp $JE/VFX_Parameter.cpp $JE/World.cpp $JE/Subsystem.cpp $SRC $JE/Common/Chunk.cpp $JE/PathManager.cpp $JE/Map.cpp $JE/SpatialGrid.cpp $JE/Action.cpp"
        "ActionMoveTo|modules/openje/tests/je/ActionMoveTo/ActionMoveTo.cpp|$JE/Action.cpp $JE/ActionMoveTo.cpp $JE/PathManager.cpp $JE/Map.cpp $JE/SpatialGrid.cpp $JE/ResourceSystem.cpp $JE/Common/Chunk.cpp $JE/VFX_Parameter.cpp $JE/Resources.cpp $JE/Subsystem.cpp $SRC $JE/World.cpp src/F3/GamePathManager.cpp src/F3/MapEntitySpawn.cpp"
        "SpawnMove|modules/openje/tests/je/ActionMoveTo/SpawnMove.cpp|src/F3/MapEntitySpawn.cpp src/F3/GameActionMoveTo.cpp $JE/Action.cpp $JE/ActionMoveTo.cpp $JE/PathManager.cpp $JE/Map.cpp $JE/SpatialGrid.cpp $JE/ResourceSystem.cpp $JE/Common/Chunk.cpp $JE/VFX_Parameter.cpp $JE/Resources.cpp $JE/Subsystem.cpp $SRC $JE/World.cpp src/F3/GamePathManager.cpp"
      )

      mkdir -p "$outDir"
      fails=0
      passed=0
      summary=""

      for entry in "''${suites[@]}"; do
        name="''${entry%%|*}"
        rest="''${entry#*|}"
        test_src="''${rest%%|*}"
        impl_srcs="''${rest#*|}"

        if [ -n "$only" ] && ! printf '%s' ",$only," | grep -qi ",$name,"; then
          continue
        fi

        exe="$outDir/$name.exe"
        # Drop listed sources that are not on disk yet (see the SpatialGrid note above).
        srcs=""
        for s in $test_src $impl_srcs; do
          if [ -f "$s" ]; then srcs="$srcs $s"; else printf 'note: %s: source not present, skipped: %s\n' "$name" "$s"; fi
        done

        # Stale-exe invariant: remove the binary and never run what a failed compile left behind.
        rm -f "$exe"

        # shellcheck disable=SC2086
        log="$("${mingxCxx}" -std=c++98 -static -w -Isrc -Imodules/openje/src -Imodules/tracing/src \
              $srcs -o "$exe" 2>&1)"
        cstat=$?
        if [ $cstat -ne 0 ] || [ ! -f "$exe" ]; then
          printf '%-15s BUILD-FAIL (exit %d)\n' "$name" "$cstat"
          [ $verbose -eq 1 ] && printf '%s\n' "$log"
          summary="$summary$name=BUILD-FAIL "
          fails=$((fails + 1))
          continue
        fi

        runlog="$(cd "$root" && "${wine}" "$exe" "$dataArg" 2>&1)"
        rstat=$?
        line="$(printf '%s' "$runlog" | grep -E '^(OK|FAIL|[0-9]+ failures)' | head -3 | tr '\n' ' ')"
        if [ $rstat -eq 0 ] && printf '%s' "$line" | grep -q '^OK'; then
          printf '%-15s OK   %s\n' "$name" "$line"
          summary="$summary$name=OK "
          passed=$((passed + 1))
        else
          printf '%-15s FAIL (exit %d) %s\n' "$name" "$rstat" "''${line:-<no OK/FAIL line>}"
          [ $verbose -eq 1 ] && printf '%s\n' "$runlog"
          summary="$summary$name=FAIL "
          fails=$((fails + 1))
        fi
        [ $keep -eq 1 ] || rm -f "$exe"
      done

      if [ $want_app -eq 1 ]; then
        appexe="''${OPENVB_TEST_EXE:-$root/build/F3.exe}"
        if [ ! -f "$appexe" ]; then
          printf '%-15s FAIL (missing %s — run msvc-build)\n' "app-boot" "$appexe"
          summary="$summary app=BUILD-MISSING "
          fails=$((fails + 1))
        else
          # Race-proof boot check (chat/from-A.md C-10): build/F3.exe is shared and is sometimes
          # mid-link when this runs. Copy it under an agent-tagged name INTO the staged data dir
          # (the engine resolves granny2/binkw32 next to the exe, so /tmp would break DLL lookup),
          # and detect a concurrent rebuild by comparing the source stat before/after the copy.
          before="$("${statBin}" -c '%Y %s' "$appexe")"
          boot_copy="$dataDir/F3.run-all-tests.exe"
          rm -f "$boot_copy"
          cp "$appexe" "$boot_copy"
          after="$("${statBin}" -c '%Y %s' "$appexe")"
          moved=""
          [ "$before" != "$after" ] && moved=" (WARNING: source exe changed during copy - another build is running)"

          applog="$(cd "$dataDir" && timeout "$timeout_s" "${wine}" "$boot_copy" 2>&1)"
          rstat=$?
          [ $keep -eq 1 ] || rm -f "$boot_copy"
          statline="$("${statBin}" -c '%y %s' "$appexe")"
          if [ $rstat -eq 124 ]; then
            printf 'app-boot        OK   exit 124 (alive past %ss) exe: %s%s\n' "$timeout_s" "$statline" "$moved"
            summary="$summary app=124 "
            passed=$((passed + 1))
          else
            printf 'app-boot        FAIL exit %d exe: %s%s\n' "$rstat" "$statline" "$moved"
            [ $verbose -eq 1 ] && printf '%s\n' "$applog" | tail -20
            summary="$summary app=$rstat "
            fails=$((fails + 1))
          fi
        fi
      fi

      printf '\nsummary: %s => %d pass, %d fail\n' "$summary" "$passed" "$fails"
      [ $fails -eq 0 ] || exit 1
      exit 0
    '';
  };
}
