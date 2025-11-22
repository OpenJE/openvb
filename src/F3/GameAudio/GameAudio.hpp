// Copyright 2024 OpenJE

#ifndef F3_GAMEAUDIO_HPP_
#define F3_GAMEAUDIO_HPP_

#include "f3/global.hpp"
#include "je/Audio.hpp"

namespace F3 {
	struct GameAudio : public JE::Audio {
		//: Members
			dword mbr_0x20;
			dword mbr_0x4;
		//: Methods
			virt_meth_0x45e390;
			virt_meth_0x45e670;
			dtor_0x45e690;
	}; // GameAudio
} // namespace F3

#endif // F3_GAMEAUDIO_HPP_
