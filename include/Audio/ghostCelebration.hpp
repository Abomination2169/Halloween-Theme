#pragma once

namespace GlobalNamespace {
    class FireworkItemController;
}

namespace Halloween::ghostCelebration {
    void LoadSounds();
    bool PlaySound(GlobalNamespace::FireworkItemController* controller);
    void ApplyGhostParticles(GlobalNamespace::FireworkItemController* controller);
}
