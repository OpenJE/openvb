// Copyright 2024 OpenJE

#include "tracing.hpp"
#include "F3/GameNetwork.hpp"

namespace F3 {

	// vft[4] 0x45E040 - GameNetwork override of engine slot 4 (over JE::Network's 0x5437B0).
	// Body not reconstructed yet.
	undefined4 GameNetwork::virt_meth_0x5437b0() {
		tracing::instrument( tracing::LOCATION, "" );
		return 0;
	} // virt_meth_0x5437b0

	// vft[5] 0x45DFD0 - GameNetwork override of engine slot 5 (over JE::Network's 0x543820).
	// Body not reconstructed yet.
	undefined4 GameNetwork::virt_meth_0x543820() {
		tracing::instrument( tracing::LOCATION, "" );
		return 0;
	} // virt_meth_0x543820

} // namespace F3
