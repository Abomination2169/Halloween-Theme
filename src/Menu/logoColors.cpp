#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/SpriteRenderer.hpp"
#include "UnityEngine/Color.hpp"

#include "GlobalNamespace/TubeBloomPrePassLight.hpp"
#include "GlobalNamespace/FlickeringNeonSign.hpp"

namespace Halloween::logoColors {

void ApplyLogoColors() {
    const UnityEngine::Color orange(1.0f, 0.30f, 0.02f, 1.0f);
    const UnityEngine::Color purple(0.65f, 0.08f, 1.0f, 1.0f);

    // BEAT neon lights
    const char* beatLights[] = {
        "BNeon", "ENeon", "ANeon", "TNeon"
    };

    for (const char* name : beatLights) {
        auto obj = UnityEngine::GameObject::Find(name);
        if (!obj) continue;

        auto neon = obj->GetComponent<GlobalNamespace::TubeBloomPrePassLight*>();
        if (neon) neon->set_color(orange);
    }

    // Keep the original flickering E
    auto eObject = UnityEngine::GameObject::Find("EFlickering");
    if (eObject) {
        auto flicker = eObject->GetComponent<GlobalNamespace::FlickeringNeonSign*>();
        if (flicker) {
            flicker->__cordl_internal_set__spriteOnColor(orange);
            flicker->__cordl_internal_set__lightOnColor(orange);
        }
    }

    // SABER neon lighting
    auto saberNeon = UnityEngine::GameObject::Find("SaberNeon");
    if (saberNeon) {
        auto neon = saberNeon->GetComponent<GlobalNamespace::TubeBloomPrePassLight*>();
        if (neon) neon->set_color(purple);
    }

    // SABER visible lettering
    auto saberLogo = UnityEngine::GameObject::Find("SaberLogo");
    if (saberLogo) {
        auto sprite = saberLogo->GetComponent<UnityEngine::SpriteRenderer*>();
        if (sprite) sprite->set_color(purple);
    }
}

}
