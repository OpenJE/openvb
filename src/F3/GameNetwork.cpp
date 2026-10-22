// Copyright 2024 OpenJE

#include "tracing.hpp"
#include "F3/GameNetwork.hpp"
#include "F3.hpp"
#include "F3/GameWorld.hpp"
#include "JE/HostHooks.hpp"

namespace F3 {

	// vft[4] 0x45E040 - SERVER factory, GameNetwork override of engine slot 4 (over
	// JE::Network's 0x5437B0). BODY READ WHOLE slice-7c: new(0x7108) ->
	// F3::GameServer::ctor_0x46ee70(this, network->mbr_0x8 [server port]) @0x45e05e/0x45e07f.
	// The 0x7108-byte GameServer is where the console script-control flag trio lives
	// (+0x70E4/0x70E5/0x70E6 - the slice-6 conn-trio, now owned by a named class). NOT
	// INSTANTIATED: the F3::GameServer class (world/netcode era) has no port TU yet; a
	// construction here would reference its complete virtual surface - named for that era,
	// per the coordinator's 7c fence (P2 netcode stays one season out).
	undefined4 GameNetwork::virt_meth_0x5437b0() {
		tracing::instrument( tracing::LOCATION, "" );
		return 0;
	} // virt_meth_0x5437b0

	// vft[5] 0x45DFD0 - CLIENT factory, GameNetwork override of engine slot 5 (over
	// JE::Network's 0x543820). BODY READ WHOLE slice-7c: new(0x118) ->
	// F3::GameClient::ctor_0x460b30(this, network->mbr_0x1c [client port]) @0x45dfee/0x45e00f;
	// the engine default 0x543820 is the same shape onto base JE::Client (new(0x118) @0x54383e,
	// ctor @0x54385f) - both factories now fully read and cited. NOT INSTANTIATED (named):
	// construction emits the GameClient/Client vftables, referencing ~35 declared-undefined
	// virtuals on both classes (LNK2001 across MSVC+mingw) - the install hub
	// Client::virt_meth_0x58f010 enumerates what that era must complete (see Network.cpp).
	// ApplyNetConfig's port guard makes the 0 result boot-safe (deviation named there:
	// shipped factories never return 0, so the guard is unreachable in shipped terms).
	undefined4 GameNetwork::virt_meth_0x543820() {
		tracing::instrument( tracing::LOCATION, "" );
		return 0;
	} // virt_meth_0x543820


	undefined4 GameNetwork::Tick() {
		tracing::instrument( tracing::LOCATION, "" );
		F3::GameWorld* w = F3::GetWorldServerInstance();
		if ( w ) {
			w->Update_0x544a90( (int)JE::Host::TickMilliseconds() );
		}
		return 0;
	} // Tick
} // namespace F3
