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

      # Build/run every host suite in modules/openje/tests/je (seven as of openje 66cd256:
      # eight as of T-005: cls_0x4d8d70 PathManager ResourceSystem StringSystem ActionMoveTo
      # Scripting SpawnMove GUILoading). Engine-side deps the headers never mention live in the
      # rows below: JE/Action.cpp, JE/SpatialGrid.cpp, JE/Gregorian.cpp, F3/MapEntitySpawn.cpp
      # (F3::GetActiveMap), F3/CombatManager.cpp + F3/CombatMath.cpp + F3/GameActionAttack.cpp
      # (combat seams). When a row prints kind=undefined-reference, add the defining .cpp there -
      # never weaken the test. Grows with the engine; that is expected, not drift.
      # Source lists live HERE (not in the test headers, which are stale/missing for most — see
      # tasks/T-001 Notes and docs/notes/implementer-B.md §T-001). Exes go to .devenv/state/tests/,
      # so this script never
      # touches the shared tests/je/<Suite>/<Suite>.[AB].exe names (chat/from-A.md C-4).
      #
      # usage: run-all-tests [--only Suite[,Suite]] [--no-app] [--msvc-build] [--keep]
      #                      [--timeout N] [-v]
      # Environment: OPENVB_TEST_FLOOR=Suite=N[,Suite=N] raises an assertion floor for ONE run.
      #   That is this gate's own falsification switch (rule 2): a floor that cannot be made to fail
      #   is decoration. Rows now carry a 4th field, name|test_src|impl_srcs|min_checks, and the
      #   floor counts assertions in SOURCE - it catches assertions being removed or a suite going
      #   vacuous, and prints that boundary itself (rule 3). It is NOT a runtime execution count.

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
      want_msvc=0
      keep=0
      verbose=0

      while [ $# -gt 0 ]; do
        case "$1" in
          --only)   only="''${2:-}"; shift 2 ;;
          --no-app) want_app=0; shift ;;
          --msvc-build) want_msvc=1; shift ;;
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

      # The app-boot suite boots the exe under this prefix; dangling user-dir links there make
      # F3::SetupSaveDirectory fatal, so heal them before running anything.
      f3-wine-userdirs-fix "$wineprefix"

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

      # name | test source | impl sources (space separated) | min_checks (T-024 assertion floor)
      # Sources the test headers do NOT mention are needed transitively (docs/notes/implementer-B.md):
      # JE/Action.cpp (JE::Action base vft + dtor), JE/SpatialGrid.cpp (JE/Map.cpp references it),
      # src/F3/MapEntitySpawn.cpp (F3::GetActiveMap, used by GamePathManager). Listed sources that
      # are not on disk yet are skipped with a note so the table survives A-lane churn.
      # If a suite suddenly BUILD-FAILs with kind=LNK after an engine commit, that is a NEW
      # cross-module reference: add the defining .cpp here rather than weakening the test.
      suites=(
        "cls_0x4d8d70|modules/openje/tests/je/cls_0x4d8d70/cls_0x4d8d70.cpp|$JE/cls_0x4d8d70.cpp|11"
        "PathManager|modules/openje/tests/je/PathManager/PathManager.cpp|$JE/PathManager.cpp $JE/Map.cpp $JE/SpatialGrid.cpp $JE/ResourceSystem.cpp $JE/Common/Chunk.cpp $JE/Resources.cpp $JE/VFX_Parameter.cpp $JE/Subsystem.cpp $SRC $JE/World.cpp $JE/Gregorian.cpp $JE/Action.cpp src/F3/GamePathManager.cpp src/F3/MapEntitySpawn.cpp src/F3/CombatManager.cpp src/F3/GameActionAttack.cpp src/F3/CombatMath.cpp $JE/Scripting.cpp|117"
        "ResourceSystem|modules/openje/tests/je/ResourceSystem/ResourceSystem.cpp|$JE/ResourceSystem.cpp $JE/Resources.cpp $JE/Map.cpp $JE/SpatialGrid.cpp $JE/VFX_Parameter.cpp $JE/Common/Chunk.cpp $JE/Subsystem.cpp $SRC|111"
        "StringSystem|modules/openje/tests/je/StringSystem/StringSystem.cpp|$JE/SS/StringSystem.cpp $JE/SS/EngineStringSystem.cpp $JE/SS/StringFormat.cpp $JE/ResourceSystem.cpp $JE/Resources.cpp $JE/Strings.cpp $JE/VFX_Parameter.cpp $JE/World.cpp $JE/Gregorian.cpp $JE/Subsystem.cpp $SRC $JE/Common/Chunk.cpp $JE/PathManager.cpp $JE/Map.cpp $JE/SpatialGrid.cpp $JE/Action.cpp|20"
        "ActionMoveTo|modules/openje/tests/je/ActionMoveTo/ActionMoveTo.cpp|$JE/Action.cpp $JE/ActionMoveTo.cpp $JE/PathManager.cpp $JE/Map.cpp $JE/SpatialGrid.cpp $JE/ResourceSystem.cpp $JE/Common/Chunk.cpp $JE/VFX_Parameter.cpp $JE/Resources.cpp $JE/Subsystem.cpp $SRC $JE/World.cpp $JE/Gregorian.cpp src/F3/GamePathManager.cpp src/F3/MapEntitySpawn.cpp src/F3/GameActionAttack.cpp src/F3/CombatMath.cpp $JE/Scripting.cpp|13"
        "Scripting|modules/openje/tests/je/Scripting/Scripting.cpp|$JE/Scripting.cpp $JE/ActionMoveTo.cpp $JE/Action.cpp $JE/PathManager.cpp $JE/Map.cpp $JE/SpatialGrid.cpp $JE/ResourceSystem.cpp $JE/Common/Chunk.cpp $JE/VFX_Parameter.cpp $JE/Resources.cpp $JE/Subsystem.cpp $JE/World.cpp $JE/Gregorian.cpp $SRC|6"
        "GUILoading|modules/openje/tests/je/GUILoading/GUILoading.cpp|$JE/GUI/InterfaceFile.cpp $JE/GUI/SkinDefaults.cpp $JE/GUI/SkinFragments.cpp $JE/GUI/SkinTable.cpp $JE/ResourceSystem.cpp $JE/Resources.cpp $JE/Map.cpp $JE/SpatialGrid.cpp $JE/VFX_Parameter.cpp $JE/Common/Chunk.cpp $JE/Subsystem.cpp $JE/World.cpp $JE/PathManager.cpp $JE/Gregorian.cpp $JE/Action.cpp $SRC|116"
        "SpawnMove|modules/openje/tests/je/ActionMoveTo/SpawnMove.cpp|src/F3/MapEntitySpawn.cpp src/F3/GameActionMoveTo.cpp src/F3/GameActionAttack.cpp src/F3/CombatMath.cpp $JE/Scripting.cpp $JE/Action.cpp $JE/ActionMoveTo.cpp $JE/PathManager.cpp $JE/Map.cpp $JE/SpatialGrid.cpp $JE/ResourceSystem.cpp $JE/Common/Chunk.cpp $JE/VFX_Parameter.cpp $JE/Resources.cpp $JE/Subsystem.cpp $SRC $JE/World.cpp $JE/Gregorian.cpp src/F3/GamePathManager.cpp|80"
        "CombatMath|modules/openje/tests/je/CombatMath/CombatMath.cpp|src/F3/CombatMath.cpp $SRC|43"
        "ScriptNatives|modules/openje/tests/je/ScriptNatives/ScriptNatives.cpp|src/F3/ScriptNatives.cpp $SRC|43"
        "Commands|modules/openje/tests/je/Commands/Commands.cpp|src/F3/Commands.cpp src/F3/CommandRegistry.cpp src/F3/GameClient.cpp $JE/DialogueManager.cpp $JE/cls_0x4d8d70.cpp $JE/cls_0x50db20.cpp $JE/Network.cpp $JE/Client.cpp $JE/CameraControl.cpp $JE/Subsystem.cpp $JE/VFX_Parameter.cpp $SRC|16"
      )

      # ---- stale-table guard ---------------------------------------------------------------
      # The script you are RUNNING is a copy baked into /nix/store when the devenv shell was
      # built; the table I maintain lives in devenv/scripts/run-all-tests.nix. If the shell has
      # not been rebuilt (or the env was sourced from a capture that had devenv's progress chatter
      # merged in with 2>&1, which corrupts the export lines and silently leaves the OLD PATH),
      # then a peer's brand-new table edits are invisible here and the failures look like engine
      # bugs. That cost a false "4 suites red" report on 2026-10-20: the committed tree was 9/9
      # green, but PATH pointed at a payload with 6 rows and no CombatManager/GameActionAttack in
      # them, producing kind=undefined-reference (chat/from-A.md C-40/C-41, coordinator dispatch).
      # Compare the rows this process actually holds against the rows in the working tree.
      src_table="$root/devenv/scripts/run-all-tests.nix"
      if [ -f "$src_table" ] && [ -f "$0" ]; then
        # A suite row is "Name|<test src>|<impl srcs>" and always mentions a .cpp, which anchors
        # the match to the table itself. Rows use the plain dollar-name form ($JE, $SRC) rather than
        # the nix dollar-brace escape, so installed and working-tree rows compare as text as-is.
        held=$(grep -oE '"[A-Za-z0-9_]+\|[^"]*\.cpp[^"]*"' "$0")
        want=$(grep -oE '"[A-Za-z0-9_]+\|[^"]*\.cpp[^"]*"' "$src_table")
        if [ "$held" != "$want" ]; then
          printf 'WARNING the installed run-all-tests is STALE vs %s\n' "$src_table"
          printf '  it holds %d suite rows, the tree has %d; differing rows:\n' \
            "$(printf '%s\n' "$held" | grep -c .)" "$(printf '%s\n' "$want" | grep -c .)"
          diff <(printf '%s\n' "$held") <(printf '%s\n' "$want") \
            | grep -E '^[<>]' | cut -c1-60 | sed 's/^/    /' | head -12
          printf '  results below measure the INSTALLED table. Reload first: devenv-reload\n' >&2
        fi
      fi

      mkdir -p "$outDir"

      # ---- T-024 integrity stamp -----------------------------------------------------------
      # Everything printed below measures THIS payload against THIS tree at THIS instant. A result
      # quoted without this block is not evidence: on 2026-10-20 alone, a stale installed payload
      # and a status read through a subshell each looked exactly like a green (harness items
      # 8b, 14, 17).
      gitq() { (cd "$root" && git "$@" 2>/dev/null) || printf 'unknown'; }
      stamp_rev=$(gitq rev-parse --short HEAD)
      stamp_sub=$( (cd "$root/modules/openje" && git rev-parse --short HEAD 2>/dev/null) || printf 'unknown')
      [ -n "$stamp_sub" ] || stamp_sub=unknown
      stamp_dirty=$(gitq status --porcelain | grep -c . || true)
      stamp_build_dirty=$(gitq status --porcelain -- cmake.toml CMakeLists.txt src devenv modules | grep -c . || true)
      stamp_hash=$(grep -oE '"[A-Za-z0-9_]+\|[^"]*\.cpp[^"]*"' "$0" | cksum | cut -d' ' -f1)
      printf 'GATE STAMP\n'
      printf '  time     : %s\n' "$(date -u '+%Y-%m-%dT%H:%M:%SZ')"
      printf '  HEAD     : %s   openje: %s\n' "$stamp_rev" "$stamp_sub"
      printf '  dirty    : %s files total, %s build-affecting (cmake/src/devenv/modules)\n' \
        "''${stamp_dirty:-0}" "''${stamp_build_dirty:-0}"
      printf '  payload  : %s\n  table    : %s rows, hash %s\n' \
        "$0" "''${#suites[@]}" "$stamp_hash"
      printf '  compiler : %s\n' "$(${mingxCxx} --version 2>/dev/null | head -1)"
      printf '  exe      : %s\n\n' "$(${statBin} -c '%y %s' "''${OPENVB_TEST_EXE:-$root/build/F3.exe}" 2>/dev/null || printf absent)"

      fails=0
      passed=0
      summary=""

      if [ $want_msvc -eq 1 ]; then
        # Opt-in full rebuild with correct attribution + error classification: an up-to-date tree
        # makes msvc-build a silent no-op (the exe mtime stays the other implementer's), and LNK
        # failures are not 'error C' (chat/from-A.md C-10 item 2, C-11 item 3).
        rm -f "$root/build/F3.exe"
        blog=$(msvc-build 2>&1); bstat=$?
        cerr=$(printf '%s' "$blog" | grep -oE 'error C[0-9]+' | wc -l)
        lnk=$(printf '%s' "$blog" | grep -oE 'LNK[0-9]+' | sort -u | tr '\n' ' ')
        if [ $bstat -eq 0 ] && [ "$cerr" -eq 0 ] && [ -z "$lnk" ]; then
          printf 'msvc-build      OK   exit 0, error C=%s, no LNK\n' "$cerr"
          summary="$summary msvc=OK "
          passed=$((passed + 1))
        else
          printf 'msvc-build      FAIL exit %d (error C=%s, LNK=%s)\n' "$bstat" "$cerr" "''${lnk:-none}"
          [ $verbose -eq 1 ] && printf '%s\n' "$blog" | tail -30
          summary="$summary msvc=FAIL "
          fails=$((fails + 1))
        fi
      fi


      for entry in "''${suites[@]}"; do
        name="''${entry%%|*}"
        rest="''${entry#*|}"
        test_src="''${rest%%|*}"
        rest="''${rest#*|}"
        impl_srcs="''${rest%|*}"
        min_checks="''${rest##*|}"
        for pair in $(printf '%s' "''${OPENVB_TEST_FLOOR:-}" | tr ',' ' '); do
          case "$pair" in "$name="*) min_checks="''${pair#*=}" ;; esac
        done
        got_checks=$(grep -cE '^[[:space:]]*(if[^(]*\)?[[:space:]]*)?(check|CHECK|expect|assert)\(' "$test_src" 2>/dev/null || true)
        [ -n "$got_checks" ] || got_checks=0

        if [ -n "$only" ] && ! printf '%s' ",$only," | grep -qi ",$name,"; then
          continue
        fi

        exe="$outDir/$name.exe"
        # Drop listed sources that are not on disk yet (see the SpatialGrid note above).
        srcs=""
        # Extra engine sources the table cannot know about yet (a lane-A file that is still
        # untracked/uncommitted, e.g. JE/Gregorian.cpp while the clock lands). Space separated,
        # relative to the repo root. This is a gate-keeping hook, NOT a place to hide mistakes:
        # if you use it, say so in chat so the table gets the file once it is committed.
        for s in $test_src $impl_srcs ''${OPENVB_TEST_EXTRA_SOURCES:-}; do
          if [ -f "$s" ]; then srcs="$srcs $s"; else printf 'note: %s: source not present, skipped: %s\n' "$name" "$s"; fi
        done

        # Stale-exe invariant: remove the binary and never run what a failed compile left behind.
        rm -f "$exe"

        # shellcheck disable=SC2086
        log="$("${mingxCxx}" -std=c++98 -static -w -Isrc -Imodules/openje/src -Imodules/tracing/src \
              $srcs -o "$exe" 2>&1)"
        cstat=$?
        if [ $cstat -ne 0 ] || [ ! -f "$exe" ]; then
          # Classify, do not just count: an 'error C' grep misses link failures (LNK2019 /
          # undefined reference), which is exactly what a missing source looks like
          # (docs/DEFERRED-NOTES.md "Tooling traps", chat/from-A.md C-11 item 3).
          kind=$(printf '%s' "$log" | grep -oE 'error [A-Z]+[0-9]+|LNK[0-9]+|undefined reference|error:' | head -1 | tr -d ':' | tr ' ' '-')
          # Name the missing symbols right on the summary line: a new engine file that is not in
          # the table below shows up as exactly this, and the fix is to add that .cpp (or to pass
          # OPENVB_TEST_EXTRA_SOURCES while the author is still mid-edit), never to weaken a test.
          syms=$(printf '%s' "$log" | tr '`' ' ' | sed -n 's/.*undefined reference to //p' \
                | tr -d "'" | tr -s ' ' | sort -u | head -3 | tr '\n' ' ')
          tail=""
          [ -n "$syms" ] && tail=" missing: $syms"
          printf '%-15s BUILD-FAIL (exit %d) kind=%s%s\n' "$name" "$cstat" "''${kind:-unknown}" "$tail"
          # Diagnostics surface on their own (implementer-B's T-024 scope ask): a red row that
          # printed only kind=error forced a manual re-run of that row's g++ line to read anything.
          printf '%s\n' "$log" > "$outDir/$name.build.log"
          printf '  diagnostics (full: %s/%s.build.log):\n' "$outDir" "$name"
          printf '%s\n' "$log" | grep -E 'error|undefined reference|LNK' | head -6 | sed 's/^/    /'
          [ $verbose -eq 1 ] && printf '%s\n' "$log"
          summary="$summary$name=BUILD-FAIL(''${kind:-unknown}) "
          fails=$((fails + 1))
          continue
        fi

        runlog="$(cd "$root" && "${wine}" "$exe" "$dataArg" 2>&1)"
        rstat=$?
        line="$(printf '%s' "$runlog" | grep -E '^(OK|FAIL|[0-9]+ failures)' | head -3 | tr '\n' ' ')"
        if [ $rstat -eq 0 ] && printf '%s' "$line" | grep -q '^OK'; then
          if [ "$got_checks" -lt "$min_checks" ]; then
            # A green run that lost its assertions is not a green run. Stated boundary: this
            # counts check() calls in the SOURCE, so it catches removal and vacuity, not a runtime
            # path that silently skipped them - that needs the suite itself to report a count.
            printf '%-15s ASSERT-FAIL (%s checks in %s, floor %s)\n' "$name" "$got_checks" "$test_src" "$min_checks"
            printf '  floor enforced by source count; prove it bites with OPENVB_TEST_FLOOR=%s=99999\n' "$name"
            summary="$summary$name=ASSERT-FAIL "
            fails=$((fails + 1))
          else
            printf '%-15s OK   %s [%s asserts >= %s]\n' "$name" "$line" "$got_checks" "$min_checks"
            summary="$summary$name=OK "
            passed=$((passed + 1))
          fi
        else
          printf '%-15s FAIL (exit %d) %s\n' "$name" "$rstat" "''${line:-<no OK/FAIL line>}"
          printf '%s\n' "$runlog" > "$outDir/$name.run.log"
          printf '  run output (full: %s/%s.run.log):\n' "$outDir" "$name"
          printf '%s\n' "$runlog" | grep -E '^FAIL|failures|error' | head -6 | sed 's/^/    /'
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
          # Wine-prefix lock contention (two implementers booting into one prefix) makes the
          # loader answer 126/1 occasionally. Retry once after 3s before calling it a
          # regression (chat/from-A.md C-13); a real crash reproduces and still fails.
          if [ $rstat -ne 124 ]; then
            sleep 3
            applog2="$(cd "$dataDir" && timeout "$timeout_s" "${wine}" "$boot_copy" 2>&1)"
            rstat2=$?
            if [ $rstat2 -eq 124 ]; then
              printf 'note: app-boot retried after %d (prefix contention) -> 124\n' "$rstat"
              rstat=124; applog="$applog2"
            fi
          fi
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

      # ---- item 57: incremental annotation gate (graduated at R-ANNOT close 2026-10-22) ----
      # Ports must cite like the research does: only files touched vs HEAD are policed, so this
      # gates NEW/EDITED defs while the 9-stock tail (R-FINISH fenced) clears at its completion.
      annot_out="$(python3 "$root/tools/port-annot-check.py" --gate 2>&1)"; annot_rc=$?
      if [ $annot_rc -ne 0 ]; then
        printf 'port-annot      FAIL (item 57: uncited defs in touched files)\n%s\n' "$(printf '%s\n' "$annot_out" | tail -8)"
        summary="$summary annot=FAIL "
        fails=$((fails + 1))
      else
        printf 'port-annot      OK   (%s)\n' "$(printf '%s' "$annot_out" | tail -1)"
        passed=$((passed + 1))
      fi

      printf '\nsummary: %s => %d pass, %d fail\n' "$summary" "$passed" "$fails"
      [ $fails -eq 0 ] || exit 1
      exit 0
    '';
  };
}
