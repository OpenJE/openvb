// Copyright 2024 OpenJE

#ifndef F3_GAMENETWORK_HPP_
#define F3_GAMENETWORK_HPP_

#include "JE/Network.hpp"

namespace F3 {
	// Game-side network object. The original factory F3::InitializeNetwork (0x45E170) allocates
	// sizeof 0x30, runs JE::Network::ctor_0x543890, then patches the vftptr to
	// F3::GameNetwork::vftable_0x663efc - the C++ equivalent is this derived class (adds no data).
	//
	// vftable 0x663efc (8 slots):
	//   [0] 0x45E0B0 scalar deleting dtor (game) - compiler-generated from the virtual dtor;
	//       completing dtor is 0x45E0D0
	//   [1] 0x543A40 JE::Network::Initialize (inherited)
	//   [2] 0x543730 JE::Network::Terminate  (inherited)
	//   [3] 0x543770 JE::Network::Tick       (inherited - the real netcode pump)
	//   [4] 0x45E040 GameNetwork override (engine slot 4; body not reconstructed)
	//   [5] 0x45DFD0 GameNetwork override (engine slot 5; body not reconstructed)
	//   [6] 0x543AC0 inherited
	//   [7] 0x5441F0 inherited
	// GameNetwork adds NO data (factory allocates the same 0x30); the database's word fields at
	// +0x08/+0x1C are re-typed aliases of JE::Network's port words (documented in Network.hpp).
	// NOTE: overrides MUST reuse the base virtual names/signatures (C++ keeps the slot), hence
	// virt_meth_0x5437b0/0x543820 here map to the game bodies 0x45E040/0x45DFD0.
	class GameNetwork : public JE::Network {
		//: Methods
		public:
			// Subsystem tick (slot 4 of the table). Parity note: the original reaches
			// World::Update_0x544a90 through the LOCAL Server object's frame pump
			// (JE::Server::virt_meth_0x552b60 = socket pump + IWorldServer+0x18);
			// no server-connection object is modeled in the demo yet, so GameWorld's
			// update frame is driven here when a world server instance exists.
			virtual undefined4 Tick();

			virtual undefined4 virt_meth_0x5437b0();  // vft[4] = 0x45E040
			virtual undefined4 virt_meth_0x543820();  // vft[5] = 0x45DFD0
	}; // GameNetwork
} // namespace F3

#endif // F3_GAMENETWORK_HPP_
