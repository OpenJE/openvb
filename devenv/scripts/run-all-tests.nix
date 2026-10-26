# ./devenv/scripts/run-all-tests.nix

{ pkgs, ... }:
let
  wine = "${pkgs.wine}/bin/wine";
  # The suites are built by MSVC 2002 cl in $MSVC_IMAGE via tools/msvc-test-compile.sh - the same compiler
  # that builds F3.exe. The mingw g++ binding and its OPENVB_TEST_TOOLCHAIN=gcc escape hatch are removed
  # (they were kept for one transition to distinguish newly-broken-under-cl from already-broken; that
  # transition is done: 14/14 gates green under cl with every floor identical, plus one real C++98 defect in
  # SpawnMove that gcc had been accepting as an extension). A second toolchain here would be a second set of
  # bugs to keep in agreement, and every green would again be evidence about a compiler the product is not.
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
        "cls_0x4d8d70|modules/openje/tests/je/cls_0x4d8d70/cls_0x4d8d70.cpp|$JE/cls_0x4d8d70.cpp $JE/cls_0x50db20.cpp $JE/cls_0x40a220.cpp|11"
        # The release + destroy range of the 0x1C-stride SSO-string array (0x432510/0x4310A0).
        # Small source list on purpose: the suite drives the container on raw storage, so it needs
        # no Map/ResourceSystem/VFX tower. Floor 29 = the check() calls the runner counts in the
        # source; the run itself reports 39 (the helper bodies assert several fields per call).
        "cls_0x50db20|modules/openje/tests/je/cls_0x50db20/cls_0x50db20.cpp|$JE/cls_0x50db20.cpp $JE/net/exception.cpp src/HostHooks.cpp $SRC|313"
        # The 0x10-byte container of 4-byte elements (0x41FE40 _Buy, 0x420140 copy ctor, 0x421230 the
        # ASSIGNMENT operator) plus its three element primitives (0x5456A0 _Ucopy / 0x41F520 / 0x5457B0
        # _Ufill). One impl source on purpose: the class only needs <new>/<stdexcept>, and the suite
        # drives it on raw heap blocks, so no Map/Resource tower. Floor 66 = the check() lines in the
        # source; the run reports 66 (the checkFresh helper adds its one call per invocation).
        "cls_0x420140|modules/openje/tests/je/cls_0x420140/cls_0x420140.cpp|$JE/cls_0x420140.cpp|66"
        # The 0x54-byte line container embedded in GUI::LineSystem (+0x2C) and GUI::Label (+0xD78):
        # meth_0x41f200 the flat teardown and meth_0x41fdf0 the destructor (0x41f200 + the embedded
        # cls_0x4abc60's full teardown). The suite existed since 2f2252b but was never in this list,
        # so its checks only ran when someone built it by hand. It carries its own link-only stubs
        # (sub_5A48E0 / FileStreamHost_ctor_0x497bb0 / cls_0x4b5d00::ctor, plus a faithful
        # RbTree_InitSentinel_0x56e0b0) and a g_unreached counter that fails the run if any of them
        # is actually reached - which is why JE.cpp is NOT in this source list: it defines the first
        # two again (LNK2005, measured), and it is not needed by any path the suite takes.
        # Floor 42 = the ck()/ck_null() calls that ran; the suite now reports the count itself.
        # cls_0x49cc40::meth_0x49cc40 - the two-child element accessor. Seven checks: the independent
        # guards and the legal 0 result, the model-mesh wrapper+0xC indirection, the skin-mesh direct
        # pointer, index scaling by the 0x244 stride on both, the LAST-call-wins result when both
        # children are present, and the absence of a bounds check.
        "cls_0x49cc40|modules/openje/tests/je/cls_0x49cc40/cls_0x49cc40.cpp|$JE/cls_0x49cc40.cpp $JE/cls_0x40a220.cpp $JE/cls_0x4abc60.cpp $JE/cls_0x4abe60.cpp $JE/cls_0x4ac210.cpp $JE/cls_0x4b5d00.cpp $JE/cls_0x4b60b0.cpp $JE/cls_0x401b00.cpp $JE/cls_0x50db20.cpp $JE/cls_0x420140.cpp $JE/cls_0x4d3d90.cpp $JE/GUI/Interface.cpp $JE/net/exception.cpp $JE.cpp|8"
        # cls_0x40cc80::meth_0x431220 - the erase/close-gap primitive of the begin/end container. The
        # requirement is what a caller can rely on afterwards (which elements moved, where the new end
        # points, which cases are no-ops), because all three interesting failure modes are about the
        # END: caching it once, not publishing it on first==last, and always publishing it otherwise.
        "cls_0x40cc80|modules/openje/tests/je/cls_0x40cc80/cls_0x40cc80.cpp|$JE/cls_0x40cc80.cpp $JE/cls_0x420140.cpp $JE/cls_0x4d3d90.cpp $JE/net/exception.cpp|51"
        # cls_0x4a5be0::meth_0x4a5d70 - the AnimMeshInstance teardown. The requirement is the object's
        # state AFTER teardown, and the shipped code is deliberately asymmetric: the malloc'd buffers
        # are freed and nulled, while the four Granny handles and the bare +0xBC block are freed and
        # left dangling. The Granny frees and the +0xB0 sub-object teardown have no reimplementation
        # (SDK imports / unported method), so the port routes them through installable hooks - which is
        # what lets the test count the forwards and check the arguments instead of asserting on a no-op.
        # Floor 23 = the check() calls this runner's regex counts in the source; the run reports 33
        # (check_ptr is a helper, so its calls add assertions without matching the floor regex).
        "cls_0x4a5be0|modules/openje/tests/je/cls_0x4a5be0/cls_0x4a5be0.cpp|$JE/cls_0x4a5be0.cpp $JE/cls_0x40cc80.cpp $JE/cls_0x420140.cpp $JE/cls_0x4d3d90.cpp|31"
        "cls_0x515040|modules/openje/tests/je/cls_0x515040/cls_0x515040.cpp|$JE/cls_0x515040.cpp $JE/cls_0x4abc60.cpp $JE/cls_0x4abe60.cpp $JE/cls_0x4ac210.cpp $JE/cls_0x4b60b0.cpp $JE/cls_0x401b00.cpp $JE/cls_0x50db20.cpp $JE/cls_0x420140.cpp $JE/cls_0x4d3d90.cpp $JE/GUI/Interface.cpp $JE/net/exception.cpp|42"
        # The enclosing object's destructor 0x505CD0, whose four parts are pinned here: the +0x28
        # scalar delete through its own vtable slot 0 (flag 1), the +0x2c release-then-free, the
        # render-manager user count at 0x7089BC (2->1 no teardown, 1->0 teardown, 0 wraps to
        # 0xFFFFFFFF and no teardown), and the base element destructor last. Same tower as
        # cls_0x515040 above plus cls_0x420270/cls_0x41cff0; the suite carries the five link-only
        # stubs itself, with a g_unreached counter, so JE.cpp stays out (it redefines two of them).
        # Floor 28 = the check() lines in the source; the run reports 28.
        "cls_0x420270|modules/openje/tests/je/cls_0x420270/cls_0x420270.cpp|$JE/cls_0x420270.cpp $JE/cls_0x41cff0.cpp $JE/cls_0x515040.cpp $JE/cls_0x4abc60.cpp $JE/cls_0x4abe60.cpp $JE/cls_0x4ac210.cpp $JE/cls_0x4b60b0.cpp $JE/cls_0x401b00.cpp $JE/cls_0x50db20.cpp $JE/cls_0x420140.cpp $JE/cls_0x4d3d90.cpp $JE/GUI/Interface.cpp $JE/net/exception.cpp|28"
        # The shipped composite copy ctor (0x565EB0): one 0x1C SSO string + one 0x10 vector holder.
        # Small source list on purpose - it drives the holder family on raw storage, no tower needed.
        "cls_0x565eb0|modules/openje/tests/je/cls_0x565eb0/cls_0x565eb0.cpp|$JE/cls_0x565eb0.cpp $JE/cls_0x50db20.cpp $JE/net/exception.cpp|32"
        # The 0x2C map-key composite: the TWO-SOURCE ctor (0x566030, its holder pointer is an
        # argument, not src+0x1C) and the first destructor in this family (0x5655B0). Small source
        # list on purpose - raw storage, no tower needed.
        "cls_0x566030|modules/openje/tests/je/cls_0x566030/cls_0x566030.cpp|$JE/cls_0x566030.cpp $JE/cls_0x50db20.cpp $JE/net/exception.cpp|37"
        # The 0x6C-byte mesh-attribute head record: TWO 0x1C strings in an ARRAY plus one standalone
        # string and one 0x10 holder. First record in the family whose elements come through the
        # shipped `_eh vector copy constructor iterator' (count 2), so it pins element ORDER and the
        # per-element construct (not assign) path. Small source list on purpose - raw storage, no tower.
        # It also carries the record's destructor (0x4d6c70), the body the owner's SEH calls on
        # unwind: the pinned defects there are the BEGIN-only holder guard, the resets that happen on
        # both arms of the standalone string's free test, and the element destructor's BYTE store.
        # And it carries the string-C material-name extraction (0x4ce790) plus its LoadMesh wrapper
        # (0x4ce820): the pinned defect is reading string A or B (+0x08/+0x24) instead of C (+0x40),
        # with the 0xCD-filled destination proving the empty-SSO triple runs before the assign.
        "cls_0x4d8b70|modules/openje/tests/je/cls_0x4d8b70/cls_0x4d8b70.cpp|$JE/cls_0x4d8b70.cpp $JE/cls_0x50db20.cpp $JE/net/exception.cpp|100"
        # The 0x90-byte mesh-attribute record and the two 0x10 container instantiations it embeds. The
        # pinned defect is a sibling copy-paste: the 4-byte and 1-byte copy ctors are otherwise identical
        # text, so the CAPACITY checks (3 elements -> 12 bytes vs 5 bytes -> 5 bytes) are what break if the
        # element stride is swapped. Also the record's deep-copy proof: release the destination, read the
        # source across all four sub-objects. Section 6 covers the record's teardown @0x4d7ac0: release
        # ORDER (+0x84 before +0x74, then the tail-called head dtor), the two container proxy words that
        # must SURVIVE it, and the head teardown the same call performs. Section 7 covers the 4-byte
        # container's ASSIGNMENT operator @0x421230: arm A keeps the block and its capacity and leaves the
        # stale tail past the new end, arm C re-buys exactly n (not 1.5x), self-assignment returns before
        # freeing, an empty source releases and zeroes. Floor 173: measured by running the row, and every
        # claim above mutation-tested (swap the two containers / drop the proxies / drop the head dtor /
        # grow arm C by half / drop the self-assignment test / trim capacity in arm A - each turns the
        # row red on the named check). The self-assignment guard is only observable when size == 0 with a
        # bought block (`_Buy` leaves exactly that state), so that case is checked separately.
        "cls_0x4d91d0|modules/openje/tests/je/cls_0x4d91d0/cls_0x4d91d0.cpp|$JE/cls_0x4d91d0.cpp $JE/cls_0x4d8b70.cpp $JE/cls_0x420140.cpp $JE/cls_0x4d6ce0.cpp $JE/cls_0x401d60.cpp $JE/cls_0x50db20.cpp $JE/net/exception.cpp|178"
        # The 0x10-byte 4-byte-element holder: its layout (the +0x00 proxy word the DB's member list
        # omits), the two element primitives only its grow path uses (_Copy_backward 0x545780 and the
        # value fill 0x4016b0), and the container bodies themselves - _Insert_n 0x4d3d90 and push_back
        # 0x4d7df0. The primitives are direction/bound pins, not arithmetic: a FORWARD copy_backward port
        # fails the right-shift case, a memmove-shaped one fails the left-shift case (the shipped loop
        # reads words it already overwrote), and a fill that runs past `last` or stops at a count fails
        # the sentinels. The container sections pin the parts a plausible port flattens: the count==0
        # arm returns CAPACITY (not _Where), the 1.5x growth (5 -> 7, not 6), which arms keep the block
        # IDENTITY (shift-right and middle-insert must not realloc) and which replace it, and the
        # length_error at 0x3FFFFFFF. cls_0x420140.cpp is here for _Ucopy/_Ufill (0x5456a0/0x5457b0).
        # Floor 116: measured by running the row. Four mutations were injected and each turned it red on
        # the named check: `return _Where` for count==0 ("returns CAPACITY"), capacity = count+size
        # instead of 1.5x ("capacity = 7 elements" + "push_back grow: capacity = 4 + 2"), and a
        # shift-right arm that moves the begin pointer ("block identity kept"). The two primitive
        # families were mutation-tested the same way when the row was created (forward copy_backward /
        # memmove copy_backward / fill off by one).
        "cls_0x4d3d90|modules/openje/tests/je/cls_0x4d3d90/cls_0x4d3d90.cpp|$JE/cls_0x4d3d90.cpp $JE/cls_0x420140.cpp|116"
        # The 1-BYTE sibling of the 4-byte holder above: layout (the same +0x00 proxy word), the four
        # byte element primitives its grow path calls (_Fill 0x4308d0, _Copy_backward 0x430920, _Ufill
        # 0x430950 and its 0x431040 tail), and _Insert_n itself (@0x432550). Small source list on purpose:
        # cls_0x4d6ce0.cpp supplies _Ucopy for 1-byte elements (0x4308f0) and nothing else is needed.
        # Floor 42 = the check() calls the runner counts in the source (the check_bytes/check_span
        # helpers assert several bytes per call, so the run reports more). The row exists because every
        # arm of this instantiation returns something DIFFERENT from its 4-byte sibling - count==0 returns
        # _Myfirst (not the capacity), the realloc arm returns the NEW CAPACITY (not _Where) and the
        # middle-insert arm returns the OLD _Mylast - so a port copied from the sibling looks right and
        # fails the named checks. The primitive sections pin directions and bounds: the right-overlap
        # copy must come out CORRECT (a forward loop smears it) and the left-overlap copy must come out
        # CORRUPTED (a memmove-shaped port silently changes behaviour), _Ufill must return 0 for a zero
        # count, and the null-destination cases must burn the count without writing.
        "cls_0x431000|modules/openje/tests/je/cls_0x431000/cls_0x431000.cpp|$JE/cls_0x431000.cpp $JE/cls_0x4d6ce0.cpp|110"
        "PathManager|modules/openje/tests/je/PathManager/PathManager.cpp|$JE/PathManager.cpp $JE/Map.cpp $JE/SpatialGrid.cpp $JE/ResourceSystem.cpp $JE/Common/Chunk.cpp $JE/Resources.cpp $JE/Common/Exception.cpp $JE/VFX_Parameter.cpp $JE/Subsystem.cpp $JE/net/exception.cpp $JE/GUI/Window.cpp $JE/AreaMapIcon.cpp $JE/CSoundSystem.cpp $JE/cls_0x50ca00.cpp $JE/GUI/Interface.cpp $JE/cls_0x52d080.cpp $JE/cls_0x539d90.cpp $JE/cls_0x53a1e0.cpp $JE/VFX_Sound.cpp $JE/cls_0x401b00.cpp $JE/cls_0x50db20.cpp $JE/CSoundFile.cpp $JE/CSoundGroup.cpp $JE/CMasterGrouplist.cpp $JE/CMasterJumplist.cpp $JE/cls_0x5392c0.cpp $JE/CJumplist.cpp $JE/Gfx_Engine.cpp $JE/cls_0x53a510.cpp $JE/CSound.cpp $JE/CMusic.cpp $JE/ogg_tables.cpp $JE/cls_0x605e28.cpp $JE/cls_0x40a220.cpp $JE/cls_0x50dbb0.cpp src/HostHooks.cpp $JE/DialogueManager.cpp $JE/cls_0x4d8d70.cpp $JE/cls_0x48d750.cpp $JE/cls_0x48d7b0.cpp $JE/cls_0x537ca0.cpp $SRC $JE/World.cpp $JE/Gregorian.cpp $JE/Action.cpp src/F3/GamePathManager.cpp src/F3/MapEntitySpawn.cpp src/F3/CombatManager.cpp src/F3/GameActionAttack.cpp src/F3/CombatMath.cpp $JE/Scripting.cpp|117"
        "ResourceSystem|modules/openje/tests/je/ResourceSystem/ResourceSystem.cpp|$JE/ResourceSystem.cpp $JE/Resources.cpp $JE/Common/Exception.cpp $JE/Map.cpp $JE/SpatialGrid.cpp $JE/VFX_Parameter.cpp $JE/Common/Chunk.cpp $JE/Subsystem.cpp $JE/net/exception.cpp $JE/GUI/Window.cpp $JE/AreaMapIcon.cpp $JE/CSoundSystem.cpp $JE/cls_0x50ca00.cpp $JE/GUI/Interface.cpp $JE/cls_0x52d080.cpp $JE/cls_0x539d90.cpp $JE/cls_0x53a1e0.cpp $JE/VFX_Sound.cpp $JE/cls_0x401b00.cpp $JE/cls_0x50db20.cpp $JE/CSoundFile.cpp $JE/CSoundGroup.cpp $JE/CMasterGrouplist.cpp $JE/CMasterJumplist.cpp $JE/cls_0x5392c0.cpp $JE/CJumplist.cpp $JE/Gfx_Engine.cpp $JE/cls_0x53a510.cpp $JE/CSound.cpp $JE/CMusic.cpp $JE/ogg_tables.cpp $JE/cls_0x605e28.cpp $JE/cls_0x40a220.cpp $JE/cls_0x50dbb0.cpp src/HostHooks.cpp $JE/DialogueManager.cpp $JE/cls_0x4d8d70.cpp $JE/cls_0x48d750.cpp $JE/cls_0x48d7b0.cpp $JE/cls_0x537ca0.cpp $SRC|111"
        "StringSystem|modules/openje/tests/je/StringSystem/StringSystem.cpp|$JE/SS/StringSystem.cpp $JE/SS/EngineStringSystem.cpp $JE/SS/StringFormat.cpp $JE/ResourceSystem.cpp $JE/Resources.cpp $JE/Strings.cpp $JE/Common/Exception.cpp $JE/VFX_Parameter.cpp $JE/World.cpp $JE/Gregorian.cpp $JE/Subsystem.cpp $JE/net/exception.cpp $JE/GUI/Window.cpp $JE/AreaMapIcon.cpp $JE/CSoundSystem.cpp $JE/cls_0x50ca00.cpp $JE/GUI/Interface.cpp $JE/cls_0x52d080.cpp $JE/cls_0x539d90.cpp $JE/cls_0x53a1e0.cpp $JE/VFX_Sound.cpp $JE/cls_0x401b00.cpp $JE/cls_0x50db20.cpp $JE/CSoundFile.cpp $JE/CSoundGroup.cpp $JE/CMasterGrouplist.cpp $JE/CMasterJumplist.cpp $JE/cls_0x5392c0.cpp $JE/CJumplist.cpp $JE/Gfx_Engine.cpp $JE/cls_0x53a510.cpp $JE/CSound.cpp $JE/CMusic.cpp $JE/ogg_tables.cpp $JE/cls_0x605e28.cpp $JE/cls_0x40a220.cpp $JE/cls_0x50dbb0.cpp src/HostHooks.cpp $JE/DialogueManager.cpp $JE/cls_0x4d8d70.cpp $JE/cls_0x48d750.cpp $JE/cls_0x48d7b0.cpp $JE/cls_0x537ca0.cpp $SRC $JE/Common/Chunk.cpp $JE/PathManager.cpp $JE/Map.cpp $JE/SpatialGrid.cpp $JE/Action.cpp|20"
        "ActionMoveTo|modules/openje/tests/je/ActionMoveTo/ActionMoveTo.cpp|$JE/Action.cpp $JE/ActionMoveTo.cpp $JE/PathManager.cpp $JE/Map.cpp $JE/SpatialGrid.cpp $JE/ResourceSystem.cpp $JE/Common/Chunk.cpp $JE/VFX_Parameter.cpp $JE/Resources.cpp $JE/Common/Exception.cpp $JE/Subsystem.cpp $JE/net/exception.cpp $JE/GUI/Window.cpp $JE/AreaMapIcon.cpp $JE/CSoundSystem.cpp $JE/cls_0x50ca00.cpp $JE/GUI/Interface.cpp $JE/cls_0x52d080.cpp $JE/cls_0x539d90.cpp $JE/cls_0x53a1e0.cpp $JE/VFX_Sound.cpp $JE/cls_0x401b00.cpp $JE/cls_0x50db20.cpp $JE/CSoundFile.cpp $JE/CSoundGroup.cpp $JE/CMasterGrouplist.cpp $JE/CMasterJumplist.cpp $JE/cls_0x5392c0.cpp $JE/CJumplist.cpp $JE/Gfx_Engine.cpp $JE/cls_0x53a510.cpp $JE/CSound.cpp $JE/CMusic.cpp $JE/ogg_tables.cpp $JE/cls_0x605e28.cpp $JE/cls_0x40a220.cpp $JE/cls_0x50dbb0.cpp src/HostHooks.cpp $JE/DialogueManager.cpp $JE/cls_0x4d8d70.cpp $JE/cls_0x48d750.cpp $JE/cls_0x48d7b0.cpp $JE/cls_0x537ca0.cpp $SRC $JE/World.cpp $JE/Gregorian.cpp src/F3/GamePathManager.cpp src/F3/MapEntitySpawn.cpp src/F3/GameActionAttack.cpp src/F3/CombatMath.cpp $JE/Scripting.cpp|13"
        "Scripting|modules/openje/tests/je/Scripting/Scripting.cpp|$JE/Scripting.cpp $JE/ActionMoveTo.cpp $JE/Action.cpp $JE/PathManager.cpp $JE/Map.cpp $JE/SpatialGrid.cpp $JE/ResourceSystem.cpp $JE/Common/Chunk.cpp $JE/VFX_Parameter.cpp $JE/Resources.cpp $JE/Common/Exception.cpp $JE/Subsystem.cpp $JE/World.cpp $JE/Gregorian.cpp $JE/net/exception.cpp $JE/GUI/Window.cpp $JE/AreaMapIcon.cpp $JE/CSoundSystem.cpp $JE/cls_0x50ca00.cpp $JE/GUI/Interface.cpp $JE/cls_0x52d080.cpp $JE/cls_0x539d90.cpp $JE/cls_0x53a1e0.cpp $JE/VFX_Sound.cpp $JE/cls_0x401b00.cpp $JE/cls_0x50db20.cpp $JE/CSoundFile.cpp $JE/CSoundGroup.cpp $JE/CMasterGrouplist.cpp $JE/CMasterJumplist.cpp $JE/cls_0x5392c0.cpp $JE/CJumplist.cpp $JE/Gfx_Engine.cpp $JE/cls_0x53a510.cpp $JE/CSound.cpp $JE/CMusic.cpp $JE/ogg_tables.cpp $JE/cls_0x605e28.cpp $JE/cls_0x40a220.cpp $JE/cls_0x50dbb0.cpp src/HostHooks.cpp $JE/DialogueManager.cpp $JE/cls_0x4d8d70.cpp $JE/cls_0x48d750.cpp $JE/cls_0x48d7b0.cpp $JE/cls_0x537ca0.cpp $SRC|6"
        "GUILoading|modules/openje/tests/je/GUILoading/GUILoading.cpp|$JE/GUI/InterfaceFile.cpp $JE/GUI/SkinDefaults.cpp $JE/GUI/SkinFragments.cpp $JE/GUI/SkinTable.cpp $JE/ResourceSystem.cpp $JE/Resources.cpp $JE/Common/Exception.cpp $JE/Map.cpp $JE/SpatialGrid.cpp $JE/VFX_Parameter.cpp $JE/Common/Chunk.cpp $JE/Subsystem.cpp $JE/World.cpp $JE/PathManager.cpp $JE/Gregorian.cpp $JE/Action.cpp $JE/net/exception.cpp $JE/GUI/Window.cpp $JE/AreaMapIcon.cpp $JE/CSoundSystem.cpp $JE/cls_0x50ca00.cpp $JE/GUI/Interface.cpp $JE/cls_0x52d080.cpp $JE/cls_0x539d90.cpp $JE/cls_0x53a1e0.cpp $JE/VFX_Sound.cpp $JE/cls_0x401b00.cpp $JE/cls_0x50db20.cpp $JE/CSoundFile.cpp $JE/CSoundGroup.cpp $JE/CMasterGrouplist.cpp $JE/CMasterJumplist.cpp $JE/cls_0x5392c0.cpp $JE/CJumplist.cpp $JE/Gfx_Engine.cpp $JE/cls_0x53a510.cpp $JE/CSound.cpp $JE/CMusic.cpp $JE/ogg_tables.cpp $JE/cls_0x605e28.cpp $JE/cls_0x40a220.cpp $JE/cls_0x50dbb0.cpp src/HostHooks.cpp $JE/DialogueManager.cpp $JE/cls_0x4d8d70.cpp $JE/cls_0x48d750.cpp $JE/cls_0x48d7b0.cpp $JE/cls_0x537ca0.cpp $SRC|116"
        "SpawnMove|modules/openje/tests/je/ActionMoveTo/SpawnMove.cpp|src/F3/MapEntitySpawn.cpp src/F3/GameActionMoveTo.cpp src/F3/GameActionAttack.cpp src/F3/CombatMath.cpp $JE/Scripting.cpp $JE/Action.cpp $JE/ActionMoveTo.cpp $JE/PathManager.cpp $JE/Map.cpp $JE/SpatialGrid.cpp $JE/ResourceSystem.cpp $JE/Common/Chunk.cpp $JE/VFX_Parameter.cpp $JE/Resources.cpp $JE/Common/Exception.cpp $JE/Subsystem.cpp $JE/net/exception.cpp $JE/GUI/Window.cpp $JE/AreaMapIcon.cpp $JE/CSoundSystem.cpp $JE/cls_0x50ca00.cpp $JE/GUI/Interface.cpp $JE/cls_0x52d080.cpp $JE/cls_0x539d90.cpp $JE/cls_0x53a1e0.cpp $JE/VFX_Sound.cpp $JE/cls_0x401b00.cpp $JE/cls_0x50db20.cpp $JE/CSoundFile.cpp $JE/CSoundGroup.cpp $JE/CMasterGrouplist.cpp $JE/CMasterJumplist.cpp $JE/cls_0x5392c0.cpp $JE/CJumplist.cpp $JE/Gfx_Engine.cpp $JE/cls_0x53a510.cpp $JE/CSound.cpp $JE/CMusic.cpp $JE/ogg_tables.cpp $JE/cls_0x605e28.cpp $JE/cls_0x40a220.cpp $JE/cls_0x50dbb0.cpp src/HostHooks.cpp $JE/DialogueManager.cpp $JE/cls_0x4d8d70.cpp $JE/cls_0x48d750.cpp $JE/cls_0x48d7b0.cpp $JE/cls_0x537ca0.cpp $SRC $JE/World.cpp $JE/Gregorian.cpp src/F3/GamePathManager.cpp|175"
        "CombatMath|modules/openje/tests/je/CombatMath/CombatMath.cpp|src/F3/CombatMath.cpp $JE/VFX_Parameter.cpp $JE/ResourceSystem.cpp $JE/net/exception.cpp $JE/GUI/Window.cpp $JE/AreaMapIcon.cpp $JE/CSoundSystem.cpp $JE/cls_0x50ca00.cpp $JE/GUI/Interface.cpp $JE/cls_0x52d080.cpp $JE/cls_0x539d90.cpp $JE/cls_0x53a1e0.cpp $JE/VFX_Sound.cpp $JE/cls_0x401b00.cpp $JE/cls_0x50db20.cpp $JE/CSoundFile.cpp $JE/CSoundGroup.cpp $JE/CMasterGrouplist.cpp $JE/CMasterJumplist.cpp $JE/cls_0x5392c0.cpp $JE/CJumplist.cpp $JE/Gfx_Engine.cpp $JE/cls_0x53a510.cpp $JE/CSound.cpp $JE/CMusic.cpp $JE/ogg_tables.cpp $JE/cls_0x605e28.cpp $JE/cls_0x40a220.cpp $JE/cls_0x50dbb0.cpp src/HostHooks.cpp $JE/DialogueManager.cpp $JE/cls_0x4d8d70.cpp $JE/cls_0x48d750.cpp $JE/cls_0x48d7b0.cpp $JE/cls_0x537ca0.cpp $SRC|43"
        "ScriptNatives|modules/openje/tests/je/ScriptNatives/ScriptNatives.cpp|src/F3/ScriptNatives.cpp $JE/VFX_Parameter.cpp $JE/ResourceSystem.cpp $JE/net/exception.cpp $JE/GUI/Window.cpp $JE/AreaMapIcon.cpp $JE/CSoundSystem.cpp $JE/cls_0x50ca00.cpp $JE/GUI/Interface.cpp $JE/cls_0x52d080.cpp $JE/cls_0x539d90.cpp $JE/cls_0x53a1e0.cpp $JE/VFX_Sound.cpp $JE/cls_0x401b00.cpp $JE/cls_0x50db20.cpp $JE/CSoundFile.cpp $JE/CSoundGroup.cpp $JE/CMasterGrouplist.cpp $JE/CMasterJumplist.cpp $JE/cls_0x5392c0.cpp $JE/CJumplist.cpp $JE/Gfx_Engine.cpp $JE/cls_0x53a510.cpp $JE/CSound.cpp $JE/CMusic.cpp $JE/ogg_tables.cpp $JE/cls_0x605e28.cpp $JE/cls_0x40a220.cpp $JE/cls_0x50dbb0.cpp src/HostHooks.cpp $JE/DialogueManager.cpp $JE/cls_0x4d8d70.cpp $JE/cls_0x48d750.cpp $JE/cls_0x48d7b0.cpp $JE/cls_0x537ca0.cpp $SRC|243"
        "GameWeapon|modules/openje/tests/je/GameWeapon/GameWeapon.cpp|src/F3/GameWeapon.cpp $JE/ResourceSystem.cpp $JE/Common/Chunk.cpp $JE/VFX_Parameter.cpp $JE/net/exception.cpp $JE/GUI/Window.cpp $JE/AreaMapIcon.cpp $JE/CSoundSystem.cpp $JE/cls_0x50ca00.cpp $JE/GUI/Interface.cpp $JE/cls_0x52d080.cpp $JE/cls_0x539d90.cpp $JE/cls_0x53a1e0.cpp $JE/VFX_Sound.cpp $JE/cls_0x401b00.cpp $JE/cls_0x50db20.cpp $JE/CSoundFile.cpp $JE/CSoundGroup.cpp $JE/CMasterGrouplist.cpp $JE/CMasterJumplist.cpp $JE/cls_0x5392c0.cpp $JE/CJumplist.cpp $JE/Gfx_Engine.cpp $JE/cls_0x53a510.cpp $JE/CSound.cpp $JE/CMusic.cpp $JE/ogg_tables.cpp $JE/cls_0x605e28.cpp $JE/cls_0x40a220.cpp $JE/cls_0x50dbb0.cpp src/HostHooks.cpp $JE/DialogueManager.cpp $JE/cls_0x4d8d70.cpp $JE/cls_0x48d750.cpp $JE/cls_0x48d7b0.cpp $JE/cls_0x537ca0.cpp $SRC|60"
        "Commands|modules/openje/tests/je/Commands/Commands.cpp|$JE/cls_0x537ca0.cpp $JE/cls_0x48d7b0.cpp src/HostHooks.cpp src/F3/Commands.cpp src/F3/CommandRegistry.cpp src/F3/GameClient.cpp $JE/ResourceSystem.cpp $JE/DialogueManager.cpp $JE/cls_0x4d8d70.cpp $JE/Network.cpp $JE/Common/Exception.cpp $JE/Client.cpp $JE/CameraControl.cpp $JE/Gfx_CamCtrl_Orbit.cpp $JE/cls_0x50dbb0.cpp $JE/net/base.cpp $JE/net/client.cpp $JE/net/critical_section.cpp $JE/net/socket.cpp $JE/net/exception.cpp $JE/GUI/Window.cpp $JE/AreaMapIcon.cpp $JE/CSoundSystem.cpp $JE/cls_0x50ca00.cpp $JE/GUI/Interface.cpp $JE/cls_0x52d080.cpp $JE/cls_0x539d90.cpp $JE/cls_0x53a1e0.cpp $JE/VFX_Sound.cpp $JE/cls_0x401b00.cpp $JE/cls_0x50db20.cpp $JE/CSoundFile.cpp $JE/CSoundGroup.cpp $JE/CMasterGrouplist.cpp $JE/CMasterJumplist.cpp $JE/cls_0x5392c0.cpp $JE/CJumplist.cpp $JE/Gfx_Engine.cpp $JE/cls_0x53a510.cpp $JE/CSound.cpp $JE/CMusic.cpp $JE/ogg_tables.cpp $JE/cls_0x605e28.cpp $JE/cls_0x40a220.cpp $JE/net/address.cpp $JE/net/buffer.cpp $JE/net/connection.cpp $JE/net/packet.cpp $JE/cls_0x5ac5e0.cpp $JE/cls_0x5ac9e0.cpp $JE/cls_0x5ac2b0.cpp $JE/cls_0x45f060.cpp $JE/cls_0x48d750.cpp $JE/cls_0x420140.cpp $JE/Subsystem.cpp $JE/VFX_Parameter.cpp $SRC|16"
        "CSound|modules/openje/tests/je/CSound/CSound.cpp|$JE/VFX_Parameter.cpp $JE/ResourceSystem.cpp $JE/net/exception.cpp $JE/GUI/Window.cpp $JE/AreaMapIcon.cpp $JE/CSoundSystem.cpp $JE/cls_0x50ca00.cpp $JE/GUI/Interface.cpp $JE/cls_0x52d080.cpp $JE/cls_0x539d90.cpp $JE/cls_0x53a1e0.cpp $JE/VFX_Sound.cpp $JE/cls_0x401b00.cpp $JE/cls_0x50db20.cpp $JE/CSoundFile.cpp $JE/CSoundGroup.cpp $JE/CMasterGrouplist.cpp $JE/CMasterJumplist.cpp $JE/cls_0x5392c0.cpp $JE/CJumplist.cpp $JE/Gfx_Engine.cpp $JE/cls_0x53a510.cpp $JE/CSound.cpp $JE/CMusic.cpp $JE/ogg_tables.cpp $JE/cls_0x605e28.cpp $JE/cls_0x40a220.cpp $JE/cls_0x50dbb0.cpp src/HostHooks.cpp $JE/DialogueManager.cpp $JE/cls_0x4d8d70.cpp $JE/cls_0x48d750.cpp $JE/cls_0x48d7b0.cpp $JE/cls_0x537ca0.cpp $SRC|230"
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
        # the match to the table itself. Rows use the plain dollar-name form ($JE, $JE/net/exception.cpp $JE/GUI/Window.cpp $JE/AreaMapIcon.cpp $JE/CSoundSystem.cpp $JE/cls_0x50ca00.cpp $JE/GUI/Interface.cpp $JE/cls_0x52d080.cpp $JE/cls_0x539d90.cpp $JE/cls_0x53a1e0.cpp $JE/VFX_Sound.cpp $JE/cls_0x401b00.cpp $JE/cls_0x50db20.cpp $JE/CSoundFile.cpp $JE/CSoundGroup.cpp $JE/CMasterGrouplist.cpp $JE/CMasterJumplist.cpp $JE/cls_0x5392c0.cpp $JE/CJumplist.cpp $JE/Gfx_Engine.cpp $JE/cls_0x53a510.cpp $JE/CSound.cpp $JE/CMusic.cpp $JE/ogg_tables.cpp $JE/cls_0x605e28.cpp $JE/cls_0x40a220.cpp $SRC) rather than
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
      printf '  compiler : MSVC 2002 cl (docker $MSVC_IMAGE) via tools/msvc-test-compile.sh\n'  
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

        # Compile with the SAME compiler the product uses: MSVC 2002 cl inside $MSVC_IMAGE, bootstrapped
        # exactly as devenv/scripts/msvc-build.nix does it. Until now suites were built by mingw g++, so
        # every green suite was evidence about a compiler the shipped binary does not use.
        #
        # OPENVB_TEST_TOOLCHAIN=gcc keeps the old path for one transition, and only so a suite that newly
        # fails under cl can be told apart from one that was already broken. It is not a supported mode:
        # delete it once the cl path has been green across a wave, because a second toolchain is a second
        # set of bugs to keep in agreement.
        # shellcheck disable=SC2086
        log="$(tools/msvc-test-compile.sh "$exe" $srcs 2>&1)"
        cstat=$?
        if [ $cstat -ne 0 ] || [ ! -f "$exe" ]; then
          # Classify, do not just count: an 'error C' grep misses link failures (LNK2019 /
          # undefined reference), which is exactly what a missing source looks like
          # (docs/DEFERRED-NOTES.md "Tooling traps", chat/from-A.md C-11 item 3).
          kind=$(printf '%s' "$log" | grep -oE 'error [A-Z]+[0-9]+|LNK[0-9]+|undefined reference|error:' | head -1 | tr -d ':' | tr ' ' '-')
          # Name the missing symbols right on the summary line: a new engine file that is not in
          # the table below shows up as exactly this, and the fix is to add that .cpp (or to pass
          # OPENVB_TEST_EXTRA_SOURCES while the author is still mid-edit), never to weaken a test.
          # gcc says "undefined reference to `X'"; cl says "unresolved external symbol X". Capture both,
          # or the row that exists to name the missing source prints no symbol at all.
          syms=$(printf '%s' "$log" | tr '`' ' ' | sed -n 's/.*undefined reference to //p' \
                | sed -n 's/.*unresolved external symbol //p;s/.*unresolved external symbol //p' \
                | tr -d "'" | tr -s ' ' | sort -u | head -3 | tr '\n' ' ')
          syms="$syms$(printf '%s' "$log" | sed -n 's/.*unresolved external symbol //p' \
                | cut -c1-70 | sort -u | head -3 | tr '\n' ' ')"
          syms=$(printf '%s' "$syms" | tr -s ' ' | sort -u | head -3 | tr '\n' ' ')
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

      # ---- member-layout ratchet (see docs/je/MEMBER-OFFSET-AUDIT.md) ----
      # The generated headers are being brought from IDA's EA order into shipped offset order; 3588
      # rows remain in the flattened memory-image structs, which need a composition fix rather than an
      # ordering one. The ratchet makes "no new member sits at an offset its own name contradicts" a
      # gate now, instead of waiting for a backlog nobody should have to clear in one change: a change
      # may reduce the count, never raise it.
      layout_out="$(python3 "$root/tools/check-member-offsets.py" --ratchet 1822 2>&1)"; layout_rc=$?
      if [ $layout_rc -ne 0 ]; then
        printf 'layout-ratchet  FAIL (new member misordered vs its own named offset)\n%s\n' "$(printf '%s\n' "$layout_out" | tail -10)"
        summary="$summary layout=FAIL "
        fails=$((fails + 1))
      else
        printf 'layout-ratchet  OK   (%s)\n' "$(printf '%s' "$layout_out" | tail -1)"
        passed=$((passed + 1))
      fi

      printf '\nsummary: %s => %d pass, %d fail\n' "$summary" "$passed" "$fails"
      [ $fails -eq 0 ] || exit 1
      exit 0
    '';
  };
}
