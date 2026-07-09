// Copyright 2024 OpenJE

#ifndef F3_GAMEAUDIO_HPP_
#define F3_GAMEAUDIO_HPP_

#include "F3.hpp"
#include "JE/Audio.hpp"

namespace F3 {
	struct GameAudio : public JE::Audio {
		//: Members
			dword mbr_0x20;
			dword mbr_0x4;
		//: Methods
			undefined4 virt_meth_0x45e390();
			undefined4 virt_meth_0x45e670();
			void dtor_0x45e690();
	}; // GameAudio
} // namespace F3

#endif // F3_GAMEAUDIO_HPP_
