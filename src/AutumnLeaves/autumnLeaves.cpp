#include "AutumnLeaves/autumnLeaves.hpp"
#include "main.hpp"
#include "beatsaber-hook/shared/safeptr.hpp"
#include "assets.hpp"
#include "logging.hpp"

#include "UnityEngine/Resources.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "UnityEngine/ParticleSystem.hpp"
#include "UnityEngine/ParticleSystemSimulationSpace.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "UnityEngine/ParticleSystemRenderer.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/Color.hpp"
#include "bsml/shared/Helpers/utilities.hpp"

#include <array>

using namespace GlobalNamespace;
using namespace UnityEngine;

namespace Halloween::autumnLeaves {
    namespace {
        constexpr int LeafVariants = 5;
        std::array<safe_ptr<GameObject*>, LeafVariants> leafObjects{};
        safe_ptr<GameObject*> dustGo;

        void RemoveLeafClones() {
            for (auto& leaf : leafObjects) {
                if (leaf) {
                    GameObject::Destroy(leaf->gameObject);
                    leaf = nullptr;
                }
            }
        }
    }

    void CreateAutumnLeaves() {
        if (leafObjects[0]) return;

        // Find the original menu dust emitter, which supplies working motion
        // and geometry on Beat Saber 1.45.3.
        if (!dustGo) {
            for (auto particle : Resources::FindObjectsOfTypeAll<ParticleSystem*>()) {
                if (particle->name == "DustPS") {
                    dustGo = particle->gameObject;
                    break;
                }
            }
        }
        if (!dustGo) return;

        // Build-time split of the original 1.40.8 RGBA atlas avoids
        // Texture2D.GetPixels/SetPixels on-device (which rendered white cubes).
        // Decode each original leaf PNG through the same BSML path that
        // already displayed the complete atlas in color on the headset.
        std::array<ArrayW<uint8_t>, LeafVariants> encodedLeaves = {
            static_cast<ArrayW<uint8_t>>(IncludedAssets::autumn_leaf_0_png),
            static_cast<ArrayW<uint8_t>>(IncludedAssets::autumn_leaf_1_png),
            static_cast<ArrayW<uint8_t>>(IncludedAssets::autumn_leaf_2_png),
            static_cast<ArrayW<uint8_t>>(IncludedAssets::autumn_leaf_3_png),
            static_cast<ArrayW<uint8_t>>(IncludedAssets::autumn_leaf_4_png),
        };

        auto shader = Shader::Find("Sprites/Default");
        if (!shader) {
            WARNING("Autumn leaves: Sprites/Default shader unavailable; retaining original menu dust");
            return;
        }

        try {
            for (int i = 0; i < LeafVariants; ++i) {
                auto tile = BSML::Utilities::LoadTextureRaw(encodedLeaves[i]);
                if (!tile) {
                    WARNING("Autumn leaves: standalone leaf PNG failed to decode");
                    RemoveLeafClones();
                    return;
                }

                auto clone = GameObject::Instantiate(dustGo->gameObject);
                if (!clone) {
                    WARNING("Autumn leaves: dust clone failed; retaining original menu dust");
                    RemoveLeafClones();
                    return;
                }
                leafObjects[i] = clone;
                clone->name = "AutumnLeavesPS";
                // Prevent the cloned DustPS from emitting inherited startup
                // particles before its 1-leaf/sec rate and material are set.
                clone->active = false;
                clone->transform->parent = nullptr;
                clone->transform->position = Vector3(0.0f, 7.0f, 0.0f);
                clone->transform->rotation = Quaternion::Euler(Vector3(0.0f, 0.0f, 0.0f));

                auto system = clone->GetComponent<ParticleSystem*>();
                auto renderer = clone->GetComponent<ParticleSystemRenderer*>();
                if (!system || !renderer) {
                    WARNING("Autumn leaves: cloned emitter missing particle components");
                    RemoveLeafClones();
                    return;
                }

                auto material = renderer->get_material();
                if (!material) {
                    WARNING("Autumn leaves: particle material unavailable");
                    RemoveLeafClones();
                    return;
                }
                material->set_shader(shader);
                material->set_mainTexture(tile);
                material->SetTexture(StringW("_MainTex"), tile);
                material->SetColor(StringW("_Color"), Color(1, 1, 1, 1));

                ParticleSystem::MinMaxCurve lifetime{};
                lifetime._ctor(14.0f);
                ParticleSystem::MinMaxCurve emissionRate{};
                emissionRate._ctor(1.0f); // 5 emitters x 1 = original 5 particles/sec
                ParticleSystem::MinMaxGradient white{};
                white._ctor(Color(1, 1, 1, 1));

                auto mainModule = system->main;
                mainModule.set_startLifetime(lifetime);
                mainModule.set_startSizeMultiplier(0.18f);
                mainModule.set_maxParticles(30);
                mainModule.set_gravityModifierMultiplier(0.025f);
                mainModule.set_startColor(white);
                mainModule.set_simulationSpace(ParticleSystemSimulationSpace::World);

                // A cloned DustPS may contain already-simulated particles.
                // Reset it so five clones do not multiply the initial burst.
                system->Stop(true);
                system->Clear(true);

                auto emission = system->emission;
                emission.set_enabled(true);
                emission.set_rateOverTime(emissionRate);
                auto shape = system->shape;
                shape.set_rotation(Vector3(0, 0, 0));
            }
        } catch (...) {
            WARNING("Autumn leaves: exception while cropping atlas or creating emitters");
            RemoveLeafClones();
            return;
        }

        // Only remove original dust once all five emitters are configured.
        // Start all five clean emitters together, after the old dust is hidden.
        dustGo->active = false;
        for (auto& leaf : leafObjects) {
            if (leaf) {
                leaf->active = true;
                auto system = leaf->GetComponent<ParticleSystem*>();
                if (system) system->Play();
            }
        }
        INFO("Autumn leaves: five individual leaf textures active; original menu dust disabled");
    }

    void UpdateLeafFloor() {
        // CollisionModule setters were stripped in Beat Saber 1.45.3.
        // Instead, remove particles only when they reach the menu floor.
        // This is a visual floor cutoff, not a replacement for physics bounce.
        // Check every third menu frame to limit managed-array allocations.
        static int frameCounter = 0;
        if (++frameCounter % 3 != 0 || !leafObjects[0]) return;

        constexpr float FloorY = 0.0f;
        constexpr int MaxPerEmitter = 30;
        for (auto& leaf : leafObjects) {
            if (!leaf) continue;
            auto system = leaf->GetComponent<ParticleSystem*>();
            if (!system) continue;
            auto particles = il2cpp_utils::NewArray<ParticleSystem_Particle>(MaxPerEmitter);
            if (!particles) continue;
            int count = system->GetParticles(particles);
            bool changed = false;
            for (int j = 0; j < count; ++j) {
                if (particles[j].m_Position.y < FloorY) {
                    particles[j].set_remainingLifetime(0.0f);
                    changed = true;
                }
            }
            if (changed) system->SetParticles(particles, count);
        }
    }

    void DestroyAutumnLeaves() {
        RemoveLeafClones();
        if (dustGo) dustGo->active = true;
    }
}
