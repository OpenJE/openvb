// Copyright 2024 OpenJE

#include "f3.hpp"
#include "f3/global.hpp"
#include "f3/Common/Chunk.hpp"
#include "f3/Re"

namespace f3 {
    namespace Common {
        // 0x4a1ca0
        Chunk::Chunk() {
            this->mbr_0x4 = 0;
            this->mbr_0x8 = 0;
            this->mbr_0xc = 0;
            this->mbr_0x10 = 0;
            this->mbr_0x14 = 0;
            this->mbr_0x18 = 0;
            this->mbr_0x1c = 0;
        }

        // 0x409270
        //Chunk::~Chunk() {}

        // 0x409280
        Chunk::~Chunk() {}

        // 0x4a1cc0
        undefined4 Chunk::Read( undefined4 param_1, int param_2 ) {
            dword dVar1;
            unsigned int uVar2;

            if (this->mbr_0x4 != 0) {
                FailWithError("Chunk::Read() called when not in proper mode.\n");
            }
            this->mbr_0x4 = 1;
            this->mbr_0x18 = param_2;
            dVar1 = FUN_004981d0(param_2);
            this->mbr_0x1c = dVar1;
            uVar2 = ResourceSystem::MemStreamRead(&this->mbr_0x8,4,param_2);
            if ((char)uVar2 != '\0') {
                uVar2 = ResourceSystem::MemStreamRead(&this->mbr_0xc,4,param_2);
                if ((char)uVar2 != '\0') {
                uVar2 = ResourceSystem::MemStreamRead(&this->mbr_0x10,4,param_2);
                if ((char)uVar2 != '\0') {
                    return CONCAT31((int3)(uVar2 >> 8),1);
                }
                }
            }
            this->mbr_0x4 = 0;
            return uVar2 & 0xffffff00;
        }

        // 0x4a1d50
        void Chunk::SkipToEnd() {

        }
    } // namespace Common
} // namespace f3
