// Copyright 2024 OpenJE

#include "tracing.hpp"
#include <stdlib.h>
#include "F3/GameWorld.hpp"
#include "F3/MapEntitySpawn.hpp"
#include "F3/CombatManager.hpp"
#include "JE/Gregorian.hpp"
#include "JE/Resources.hpp"
#include "JE/ResourceSystem.hpp"
#include "JE/VFX_Parameter.hpp"
#include "JE/PathManager.hpp"
#include "F3/GamePathManager.hpp"
#include "F3/GameParty.hpp"
#include "F3/GameJournal.hpp"
#include "JE/Encyclopedia.hpp"
#include "JE/StoreManager.hpp"

namespace F3 {
    // 0x470340
    void GameWorld::ctor_0x470340() {
        tracing::instrument( tracing::LOCATION, "" );
    } // ctor_0x470340

    // World-owned config tree is now a REAL embedded member (F3/GameWorld.hpp m_worldParams,
    // R-FINISH item 1) - the file-static parking lot is retired with the pad.

    static void LoadWorldIni( JE::VFX_Parameter& tree, const char* resourceName ) {
        if ( !JE::g_resource_system ) {
            return;
        }
        void* buf = NULL;
        int len = 0;
        if ( JE::g_resource_system->ReadResource( resourceName, &buf, &len ) ) {
            tree.ParseIniText( (char*)buf, len );   // same routine as 0x5A9AB0 wrapper
            free( buf );
        } else {
            // 0x546xxx: the original logs through the file-manager path; keep parity quietly
        }
    } // LoadWorldIni

    // vft[1] 0x546690 JE::World::Initialize - subset implemented (the full body chains into
    // server/client/net subsystem vfts [4][7][8][9][10][11][13], the +0x8C area-map object and
    // the game calendar from [World] Year/Month - all pending those subsystems' reconstruction;
    // see docs/DEFERRED-NOTES.md):
    //   - parse globals.ini and locals.ini into the world parameter tree (Scripts.grp ftype 800)
    bool GameWorld::CombatMidTurn() const {
        return m_combat && m_combat->MidTurn();
    }

    // Combat seam: turn manager tick ahead of job processing (0x545E03 -> 0x545E18 order).
    // T-011 step 4 CHANGED THE OPERAND and this is why the line is not what it looks like:
    // the shipped consumer at 0x5a95aa reads world+0x40, which is the IMMUTABLE config
    // [World] "Seconds Per Round" (default 6 at 0x546b5b, atol'd to +0x40 at 0x546c8f; the
    // writer census was closed by T-010d) - NOT the world clock. Feeding TotalSeconds() here
    // (the old C-23 placeholder) made accMs advance with wall time, which is exactly what the
    // round-quantum rule forbids. So: secondsPerRound in, per DISPATCHED queue event.
    void GameWorld::CombatTick( int dtMs, int ms ) {
        (void)ms;
        if ( m_combat ) {
            m_combat->Tick( dtMs, (unsigned)( m_secondsPerRound > 0 ? m_secondsPerRound : 0 ) );
        }
    } // CombatTick

    undefined4 GameWorld::Initialize() {
        tracing::instrument( tracing::LOCATION, "" );

        LoadWorldIni( m_worldParams, "globals.ini" );   // strings at 0x6918B4 / 0x6918A8
        LoadWorldIni( m_worldParams, "locals.ini" );

        // Subsystem factory cascade (original GameWorld vft slots; each stores into world+N).
        // vft[7] 0x46F9E0 GamePathManager (0x10) -> +0xF0 - the A* policy object reconstructed
        // earlier this session; steerer/world plumbing pending.
        m_pathManager = new F3::GamePathManager();
        // vft[8] 0x46F820 GameParty (0x18, ctor 0x46CD00) -> +0xF4
        m_party = new F3::GameParty();
        // vft[9] 0x46F880 GameJournal (0x1C, ctor 0x46C410) -> +0xF8
        m_journal = new F3::GameJournal();
        // vft[10] 0x46F8E0 JE::Encyclopedia 0x10 + vftpatch GameEncyclopedia (0x668430) -> +0xFC
        m_encyclopedia = new JE::Encyclopedia();
        // vft[11] 0x46F930 JE::StoreManager 0x10 + vftpatch GameStoreManager (0x668448) -> +0x100
        m_storeManager = new JE::StoreManager();
        // vft[13] 0x545430 JE::World::CreateMapRequestMgr (cls_0x5a42b0, 0x14) -> +0x104.
        // cls_0x5a42b0 is unmodeled (map-load request bookkeeping); store NULL for now.
        m_mapRequestMgr = NULL;
        m_combat = new CombatManager();   // world+0x8C (JE cls_0x5a9330 family)
        // Register the manager as the one attacks join through (0x5490f4 resolves it from the
        // world-server singleton; the seam is in MapEntitySpawn.cpp because GameActionAttack.cpp
        // is compiled into suites that link neither this file nor src/F3.cpp).
        SetActiveCombatManager( m_combat );

        // World clock. Original: GameWorld vb idx4 factory 0x46fda0 -> sub_544D90 -> operator
        // new(0x38) @0x544dab -> Gregorian::ctor_0x544950, which seeds itself with a VERBATIM
        // 6-dword block copy of the calendar fields (`rep movsl`, ecx = 6, @0x54497d) - no
        // conditional, no fallback, no clamp anywhere on that path. So whatever the ctor left in
        // +0x14..+0x28 (0,1,1,0,0,0 - see JE/World.hpp) is what the clock gets when sys.ini has no
        // [World] block, and 2253-10-18 only ever arrives FROM sys.ini.
        // The previous three lines here were `x ? x : 2253/10/18`, i.e. the shipped file's CONTENTS
        // wearing the clothes of a default - and worse, they made the ctor's own 0 unreachable: an
        // authored Year=0 would have been silently retimed to 2253. (T-042.)
        ApplyWorldClockParams();
        m_clock = new JE::Gregorian( m_calendarYear, m_calendarMonth, m_calendarDay,
		                             m_calendarHour, m_calendarMinute, m_calendarSecond );

        // Update-dispatch state (mbr_0x34): 1 selects the per-entity pump path once a
        // world session is live (session-14 §2 switch). Set after subsystem construction.
        mbr_0x34 = 1;

        // Startup map load + entity spawn (shipped flow queues through mapRequestMgr
        // (cls_0x5a42b0 @world+0x104); we load inline - request manager reconstruction pending).
        F3::LoadStartupMaps();

        // vft[4] GameWorld::InitInterfaces_0x46FB10 -> JE::World::InitInterfaces_0x5460D0
        // (dialogue/console/menu interface table; DEFERRED - interfaces unmodeled)
        // [World] Year / Month calendar wiring: deferred with GameTime.
        return 0;
    } // Initialize

	// GameWorld.hpp declares this inline-free on purpose (see the note there).
	unsigned GameWorld::ClockSeconds() const {
		return m_clock ? (unsigned)m_clock->TotalSeconds() : 0u;
	}

} // namespace F3
