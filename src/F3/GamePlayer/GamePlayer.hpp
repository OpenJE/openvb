// Copyright 2024 OpenJE

#ifndef F3_GAMEPLAYER_HPP_
#define F3_GAMEPLAYER_HPP_

#include "f3/global.hpp"
#include "f3/GameCreature.hpp"

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
			ctor_0x411820;
			virt_meth_0x411900;
			virt_meth_0x411910;
			virt_meth_0x411920;
			virt_meth_0x411930;
			virt_meth_0x411940;
			virt_meth_0x411950;
			virt_meth_0x411960;
			virt_meth_0x411970;
			virt_meth_0x411980;
			virt_meth_0x411990;
			virt_meth_0x4119a0;
			virt_meth_0x4119b0;
			virt_meth_0x4119c0;
			virt_meth_0x4119d0;
			virt_meth_0x4119e0;
			virt_meth_0x4119f0;
			virt_meth_0x411a00;
			virt_meth_0x411a10;
			virt_meth_0x411a20;
			virt_meth_0x411a30;
			virt_meth_0x411a40;
			virt_meth_0x411a50;
			virt_meth_0x411a60;
			virt_meth_0x411a70;
			virt_meth_0x411a80;
			virt_meth_0x411a90;
			virt_meth_0x411bf0;
			virt_meth_0x411c50;
	}; // GamePlayer
} // namespace F3

#endif // F3_GAMEPLAYER_HPP_
