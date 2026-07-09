// Copyright 2024 OpenJE

#ifndef F3_GAMEPLAYER_HPP_
#define F3_GAMEPLAYER_HPP_

#include "F3.hpp"
#include "F3/GameCreature.hpp"

namespace F3 {
	struct GamePlayer : public GameCreature {
		//: Members
			dword mbr_0x4;
			dword mbr_0x800;
			byte mbr_0x808;
			dword mbr_0x818;
			dword mbr_0x81c;
			dword mbr_0x824;
		//: Methods
			void ctor_0x411820();
			undefined4 virt_meth_0x411900();
			undefined4 virt_meth_0x411910();
			undefined4 virt_meth_0x411920();
			undefined4 virt_meth_0x411930();
			undefined4 virt_meth_0x411940();
			undefined4 virt_meth_0x411950();
			undefined4 virt_meth_0x411960();
			undefined4 virt_meth_0x411970();
			undefined4 virt_meth_0x411980();
			undefined4 virt_meth_0x411990();
			undefined4 virt_meth_0x4119a0();
			undefined4 virt_meth_0x4119b0();
			undefined4 virt_meth_0x4119c0();
			undefined4 virt_meth_0x4119d0();
			undefined4 virt_meth_0x4119e0();
			undefined4 virt_meth_0x4119f0();
			undefined4 virt_meth_0x411a00();
			undefined4 virt_meth_0x411a10();
			undefined4 virt_meth_0x411a20();
			undefined4 virt_meth_0x411a30();
			undefined4 virt_meth_0x411a40();
			undefined4 virt_meth_0x411a50();
			undefined4 virt_meth_0x411a60();
			undefined4 virt_meth_0x411a70();
			undefined4 virt_meth_0x411a80();
			undefined4 virt_meth_0x411a90();
			undefined4 virt_meth_0x411bf0();
			undefined4 virt_meth_0x411c50();
	}; // GamePlayer
} // namespace F3

#endif // F3_GAMEPLAYER_HPP_
