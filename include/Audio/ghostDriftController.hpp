#pragma once

#include "custom-types/shared/macros.hpp"
#include "UnityEngine/MonoBehaviour.hpp"

namespace GlobalNamespace { class FireworkItemController; }

DECLARE_CLASS_CODEGEN(Halloween, GhostDriftController, UnityEngine::MonoBehaviour) {
    DECLARE_INSTANCE_METHOD(void, Update);
    DECLARE_INSTANCE_METHOD(void, OnDestroy);
    DECLARE_INSTANCE_FIELD(float, elapsed);
    DECLARE_INSTANCE_FIELD(float, untilSpawn);
    DECLARE_INSTANCE_FIELD(int, sequence);
};

namespace Halloween::ghostDrift {
    // Called from the existing 1.40.8 results-firework initialization hook.
    // Returns false if no camera/sprites are ready, allowing legacy fallback.
    bool Begin(GlobalNamespace::FireworkItemController* firework);
}
