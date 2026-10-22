// Copyright 2024 OpenJE

#ifndef F3_GAMEWORLD_HPP_
#define F3_GAMEWORLD_HPP_

#include "F3.hpp"
#include "JE/World.hpp"
#include "JE/VFX_Parameter.hpp"

namespace JE { class Encyclopedia; class StoreManager; class PathManager; class VFX_Parameter; }
namespace F3 { struct GameParty; struct GameJournal; }

namespace F3 {
	class CombatManager;

	struct GameWorld : public JE::World {
		//: Members
			dword mbr_0x10;
			dword mbr_0x164;
			dword mbr_0x4;
			dword mbr_0x8;
			dword mbr_0xc;
		//: Members (subsystem pointers created by the Initialize cascade; original offsets shown)
		public:
			JE::PathManager*      m_pathManager;    // +0xF0  vft[7] CreatePathManager_0x46f9e0
			F3::GameParty*        m_party;          // +0xF4  vft[8] CreateParty_0x46f820
			F3::GameJournal*      m_journal;        // +0xF8  vft[9] CreateJournal_0x46f880
			JE::Encyclopedia*     m_encyclopedia;   // +0xFC  vft[10] CreateGameEncyclopedia (vft -> GameEncyclopedia 0x668430)
			JE::StoreManager*     m_storeManager;   // +0x100 vft[11] CreateGameStoreManager  (vft -> GameStoreManager 0x668448)
			CombatManager*        m_combat;         // +0x8C  cls_0x5a9330 initiative manager
			void*                 m_mapRequestMgr;  // +0x104 vft[13] JE::World::CreateMapRequestMgr_0x545430
			                                        //        (cls_0x5a42b0, 0x14; unmodeled - stored NULL)
			// Combat seams (JE::World): tick the turn manager before job processing and
			// expose its mid-turn flag to gate the entity loop (0x545E0D/0x545E23).
			virtual void CombatTick( int dtMs, int ms );
			virtual bool CombatMidTurn() const;   // impl in GameWorld.cpp (needs the full type)

			// JE::World seam: vft[16] ServerUpdate pumps this manager's job queue
			// (world+0xF0 per original 0x545E18 evidence).
			virtual JE::PathManager* GetPathManager() { return m_pathManager; }

			// T-026 digest seam: the turn queue needs to be READ by the digest in queue order.
			// Accessor rather than public member, so nothing outside the game layer can reorder it.
			CombatManager* GetCombat() const { return m_combat; }

			// T-026 digest: the same value GameWorld::Tick feeds to CombatManager::Tick, i.e. the
			// world clock in whole seconds, so a divergence can be attributed to sim time rather
			// than to frame counting. Declared here and DEFINED in GameWorld.cpp: World.hpp only
			// forward-declares JE::Gregorian, and an inline body here compiled under mingw purely
			// because each test TU happens to include Gregorian.hpp first - MSVC saw through that
			// and failed with C2027, which is the correct verdict for an order-dependent header.
			unsigned ClockSeconds() const;

			// vft[12] 0x46F980 CreateGameWorldMap (JE::WorldMap 0x80 + vft -> GameWorldMap
			// 0x668480) is NOT part of the Initialize cascade; caller/location to be found.
		//: Methods
		public:
			// vft[1] = JE::World::Initialize_0x546690 (generic subsystem Initialize slot).
			virtual undefined4 Initialize();
			// The game config trees: globals.ini + locals.ini parse into the world-owned
			// parameter tree at world+0x90 (separate from the engine 0x70C2C0 tree).
			// R-FINISH item 1: the pad is now a REAL member - the +0x90 layout was CLOSED by
			// R-VEG-E's value shape {SSO name buf@value+0x04, size@+0x14, cap@+0x18=15, embedded
			// tree at value+0x1C} and the World teardown calls the tree's own clear at the
			// embedded slot: sub_5AA340(this+144) @0x54852f inside the World destroy chain
			// (0x5aa340-0x5aa3b1: tree-clear via VFX_Parameter::meth_0x45d1d0 on value+0x1C,
			// then SSO reset cap=15 @0x5aa3aa / size=0 @0x5aa3a7 / NUL @0x5aa3b1 - the SAME
			// field triple this port's ctor now byte-pins per item 2). The port embeds the
			// behavioral tree directly (port classes are behavior-first on layout; the shipped
			// region is world+0x90..+0xD4 per the IDB member census).
		private:
			JE::VFX_Parameter m_worldParams;   // +0x90 embedded parameter tree (was: void* pad)
		public:
			void dtor_0x46f190();
			undefined4 virt_meth_0x46f820();
			undefined4 virt_meth_0x46f880();
			undefined4 virt_meth_0x46f8e0();
			undefined4 virt_meth_0x46f930();
			undefined4 virt_meth_0x46f980();
			undefined4 virt_meth_0x46f9e0();
			undefined4 virt_meth_0x46fa40();
			undefined4 virt_meth_0x46fd20();
			undefined4 virt_meth_0x46fd40();
			undefined4 virt_meth_0x46fd60();
			undefined4 virt_meth_0x46fd80();
			undefined4 virt_meth_0x46fda0();
			undefined4 virt_meth_0x46fdb0();
			undefined4 virt_meth_0x46fdc0();
			undefined4 virt_meth_0x46fdd0();
			undefined4 virt_meth_0x46fde0();
			undefined4 virt_meth_0x46fe10();
			undefined4 virt_meth_0x46fe40();
			undefined4 virt_meth_0x46fe70();
			undefined4 virt_meth_0x46fe90();
			undefined4 virt_meth_0x46fea0();
			undefined4 virt_meth_0x46fed0();
			undefined4 virt_meth_0x46ff00();
			undefined4 virt_meth_0x46ff20();
			undefined4 virt_meth_0x46ff50();
			undefined4 virt_meth_0x46ff80();
			undefined4 virt_meth_0x46ffa0();
			undefined4 virt_meth_0x46ffb0();
			undefined4 virt_meth_0x46ffc0();
			undefined4 virt_meth_0x46ffd0();
			undefined4 virt_meth_0x46ffe0();
			undefined4 virt_meth_0x46fff0();
			undefined4 virt_meth_0x470010();
			undefined4 virt_meth_0x470030();
			undefined4 virt_meth_0x470050();
			undefined4 virt_meth_0x470070();
			undefined4 virt_meth_0x470090();
			undefined4 virt_meth_0x4700b0();
			undefined4 virt_meth_0x4700d0();
			undefined4 virt_meth_0x4700f0();
			undefined4 virt_meth_0x470110();
			undefined4 virt_meth_0x470130();
			undefined4 virt_meth_0x470140();
			undefined4 virt_meth_0x470150();
			undefined4 virt_meth_0x470160();
			undefined4 virt_meth_0x470180();
			undefined4 virt_meth_0x4701a0();
			undefined4 virt_meth_0x4701b0();
			undefined4 virt_meth_0x4701c0();
			undefined4 virt_meth_0x4701e0();
			undefined4 virt_meth_0x470200();
			undefined4 meth_0x470220();
			undefined4 virt_meth_0x470330();
			void ctor_0x470340();
	}; // GameWorld
} // namespace F3

#endif // F3_GAMEWORLD_HPP_
