// Copyright 2024 OpenJE

#include "f3/global.hpp"
#include "f3/Action.hpp"
#include "f3/ActionAnim.hpp"
#include "f3/net/critical_section.hpp"

namespace f3 {
    // 0x5a9230
    ActionAnim::ActionAnim() {
        this->mbr_0x10 = 1;
        this->mbr_0x18 = 0;
        this->mbr_0x1c = 0;
        this->mbr_0x20 = 0;
        this->mbr_0x22 = 0;
        this->mbr_0x23 = 0;
        this->mbr_0x24 = 0;
        this->mbr_0x4 = 1; // Action::mbr_0x4
    }

    // 0x405350
    undefined4* ActionAnim::virt_meth_0x405350( byte param_1 ) {

    }

    // 0x5a9260
    byte ActionAnim::Get_mbr_0x23() {
        return this->mbr_0x23;
    }

    // 0x5a9270
    void ActionAnim::virt_meth_0x5a9270() {
        //(this->entity->DynamicObject).field_0x288 = 1;
        this->mbr_0x23 = 1;
        return;
    }

    // 0x5a9280
    undefined4 ActionAnim::virt_meth_0x5a9280( int param1 ) {
    }

    // 0x5a92f0
    undefined4 ActionAnim::virt_meth_0x5a92f0() {

    }

    // 0x5a9310
    undefined4 ActionAnim::virt_meth_0x5a9310() {

    }
} // namespace f3
