// Copyright 2024 OpenJE

#include "tracing.hpp"
#include "F3/GameWorld.hpp"

namespace F3 {
    // 0x470340
    void GameWorld::ctor_0x470340() {
        tracing::instrument( tracing::LOCATION, "" );
    } // ctor_0x470340
} // namespace F3
