// Copyright 2024 OpenJE

#include "f3/global.hpp"
#include "f3/Action.hpp"

namespace f3 {
    // 0x4066f0
    //Action::~Action() {}

    // 0x4010e0
    Action::~Action() {}

    // 0x4010d0
    byte Action::Get_mbr_0x4() {
        return this->mbr_0x4;
    }
} // namespace f3
