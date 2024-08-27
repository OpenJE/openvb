// Copyright 2024 OpenJE

#ifndef F3_GAMEAUDIO_HPP_
#define F3_GAMEAUDIO_HPP_

#include "f3/global.hpp"
#include "f3/Audio.hpp"

namespace f3 {
	struct GameAudio : public Audio {
		//: Members
			dword mbr_0x20;
			dword mbr_0x4;
		//: Methods
			virt_meth_0x45e390;
			virt_meth_0x45e670;
			dtor_0x45e690;
	}; // GameAudio
} // namespace f3

#endif // F3_GAMEAUDIO_HPP_
