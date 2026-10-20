// Copyright 2024 OpenJE

#include "tracing.hpp"
#include <stdlib.h>
#include "F3/GameWorld.hpp"
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

    // World-owned config tree (original at world+0x90; kept file-static until the 0x39C-byte
    // GameWorld layout is reconstructed). Read-only consumers: [World] Year/Month seed the game
    // calendar; [Globals] SKILLDIFF*/Greeting feed difficulty checks.
    static JE::VFX_Parameter g_worldParams;

    static void LoadWorldIni( const char* resourceName ) {
        if ( !JE::g_resource_system ) {
            return;
        }
        void* buf = NULL;
        int len = 0;
        if ( JE::g_resource_system->ReadResource( resourceName, &buf, &len ) ) {
            g_worldParams.ParseIniText( (char*)buf, len );   // same routine as 0x5A9AB0 wrapper
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
    undefined4 GameWorld::Initialize() {
        tracing::instrument( tracing::LOCATION, "" );

        LoadWorldIni( "globals.ini" );   // strings at 0x6918B4 / 0x6918A8
        LoadWorldIni( "locals.ini" );

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

        // vft[4] GameWorld::InitInterfaces_0x46FB10 -> JE::World::InitInterfaces_0x5460D0
        // (dialogue/console/menu interface table; DEFERRED - interfaces unmodeled)
        // [World] Year / Month calendar wiring: deferred with GameTime.
        return 0;
    } // Initialize
} // namespace F3
