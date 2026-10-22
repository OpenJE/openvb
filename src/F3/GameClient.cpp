// Copyright 2024 OpenJE
// F3::GameClient - the net-boot game client. R-USERRE-A slice-7a lands the constructor era
// (0x460b30): base JE::Client init + the GameClient-era command registrations. The registration
// tail is 23 commands (0x460b6f-0x460cbf); FOUR of them resolve to the shipped shared empty
// override JE::DialogueManager::virt_meth_0x404b70 (`retn` @0x404b70) and land NOW, byte-
// faithfully as no-ops. The other nineteen name their handler EAs below as gated rows - each
// waits on the subsystem its shipped body touches (window/GUI manager, cursor manager, fade,
// entity walk, weather flags); the pump (virt_meth_0x45f000) is the next slice-7 checkpoint.

#include "F3/GameClient.hpp"

#include "F3.hpp"
#include "JE/Client.hpp"
#include "JE/DialogueManager.hpp"
#include "tracing.hpp"

namespace F3 {

	// 0x460B30 - F3::GameClient constructor. Shipped flow (read whole this slice):
	//   0x460b52  JE::Client::ctor_0x58e900(this, port)     - base init (the 84-command era
	//                                                       runs inside; port method-call)
	//   0x460b69  vft store GameClient_Client_net_client_net_base_vftable_0x664804 - implicit
	//                                                       in the port's class (Client.cpp
	//                                                       convention: the compiler owns it)
	//   0x460b6f..0x460cbf  23 F3::RegisterCommand sites, listed below in ctor order; live
	//                                                       rows are emitted, gated rows are
	//                                                       comments naming the ctor site.
	void GameClient::ctor_0x460b30( ushort port ) {
		tracing::instrument( tracing::LOCATION, "port=%u", port );

		this->JE::Client::ctor_0x58e900( port );   // @0x460b52

		//   GATED rows (ctor site -> handler body site; per-row recipe: port the handler,
		//   un-comment in ctor order - the same discipline the 84-era uses):
		//   0x460b6f EscapeMenu          @0x45ec00  GameClient method era (no cpp TU yet)
		//   0x460b7e AttackModeCycle     @0x45ee60  GameClient era handler (batch map)
		//   0x460bab FeedbackWindow      @0x45ee00  GUI window manager era
		//   0x460bba GameOptions         @0x45edb0  GUI window era
		//   0x460bc9 HotKeyOptions       @0x45edc0  GUI window era
		//   0x460bd8 InventoryWindow     @0x45eac0  GUI window era
		//   0x460bea LoadGameOptions     @0x45edd0  GUI/options file era
		//   0x460bf9 MultiplayerOptions  @0x45ede0  GUI window era
		//   0x460c08 PipBoy              @0x45ec70  GUI window era (NOTE: the menu-literal
		//                            feeder sub_41CA00 pushes "Pipboy"/"InventoryWindow"
		//                            through the FIFO - these registrations are WHY).
		//   0x460c26 UseSkill            @0x45ee30  entity/skill era
		//   0x460c35 VideoOptions        @0x45edf0  GUI/engine era
		//   0x460c53 WeaponToggle        @0x45eed0  entity/weapon era
		//   0x460c65 ActionbarWindow     @0x45ea50  GUI window era
		//   0x460c74 ForceEntityStep     @0x460720  entity walk era
		//   0x460c83 OptionsWindow       @0x45eb70  GUI window era
		//   0x460c92 TestAnim            @0x460920  animation era
		//   0x460ca1 ToggleRain          @0x4606c0  ERRATUM (7b pre-scan): NOT a member flag -
		//                            the wrapper calls GameClient::meth_0x4601f0, which BUILDS
		//                            a net::packet (ctor 0x45f300), sets type 110 (@0x460242)
		//                            and SENDS via meth_0x59c4f0(this, packet, 1) - a NET
		//                            BROADCAST. Snow @0x4602a0 and Wind @0x460350 are the same
		//                            shape (packet era, gated behind netcode + client boot).

		// The four commands the SHIPPED build registers at the shared empty override (retn
		// @0x404b70) - registered, resolvable, dispatchable, observable NO-OP. Porting them
		// as no-ops IS the byte-faithful port; inventing behavior here would be the defect.
		F3::RegisterCommand( "AttackModeSelect",
		    reinterpret_cast<void ( __cdecl *)()>( &JE::DialogueManager::virt_meth_0x404b70 ) );
		                                                    // @0x460b8d -> retn stub @0x404b70
		F3::RegisterCommand( "CharacterRecordWindow",
		    reinterpret_cast<void ( __cdecl *)()>( &JE::DialogueManager::virt_meth_0x404b70 ) );
		                                                    // @0x460b9c -> retn stub @0x404b70
		F3::RegisterCommand( "SneakToggle",
		    reinterpret_cast<void ( __cdecl *)()>( &JE::DialogueManager::virt_meth_0x404b70 ) );
		                                                    // @0x460c17 -> retn stub @0x404b70
		F3::RegisterCommand( "WeaponLinkToggle",
		    reinterpret_cast<void ( __cdecl *)()>( &JE::DialogueManager::virt_meth_0x404b70 ) );
		                                                    // @0x460c44 -> retn stub @0x404b70
	} // ctor_0x460b30

	// Legacy zero-arg form (Client.cpp precedent): shipped ctor always takes the connect port
	// (F3::GameNetwork::virt_meth_0x45dfd0 era passes network+0x1C); 0 keeps old call shapes.
	void GameClient::ctor_0x460b30() {
		tracing::instrument( tracing::LOCATION, "" );   // @0x460b30 wrapper position
		ctor_0x460b30( (ushort)0 );
	}

} // namespace F3
