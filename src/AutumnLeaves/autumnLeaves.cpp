#include "AutumnLeaves/autumnLeaves.hpp"
#include "AutumnLeaves/particleController.hpp"
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
#include "UnityEngine/LayerMask.hpp"
#include "UnityEngine/ParticleSystemRenderer.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/Time.hpp"
#include "bsml/shared/Helpers/utilities.hpp"

#include <array>

using namespace GlobalNamespace;
using namespace UnityEngine;

namespace Halloween::autumnLeaves {
    namespace {
        constexpr int LeafVariants = 5;
        std::array<safe_ptr<GameObject*>, LeafVariants> leafObjects{};
        safe_ptr<GameObject*> dustGo;
        float emissionAccumulator = 0.0f;
        int nextLeafVariant = 0;
        float diagnosticsElapsed = 0.0f;
        int diagnosticsEmitted = 0;


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
        emissionAccumulator = 0.0f;
        nextLeafVariant = 0;
        diagnosticsElapsed = 0.0f;
        diagnosticsEmitted = 0;

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
                white._ctor(Color(1, 1, 1, 1.0f));

                auto mainModule = system->main;
                mainModule.set_startLifetime(lifetime);
                mainModule.set_startSizeMultiplier(0.18f);
                mainModule.set_maxParticles(150);
                mainModule.set_gravityModifierMultiplier(0.025f);
                mainModule.set_startColor(white);
                mainModule.set_simulationSpace(ParticleSystemSimulationSpace::World);

                // Mirror the original 1.40.8 particle configuration rather
                // than inheriting DustPS color fades or speed-driven tint.
                Flair::ParticleController::ColorOverLifetimeModule::set_enabled(system, false);
                Flair::ParticleController::ColorBySpeedModule::set_enabled(system, false);
                Flair::ParticleController::ShapeModule::set_enabled(system, true);
                Flair::ParticleController::ShapeModule::set_shapeType(
                    system, Flair::ParticleController::ShapeType::Box);
                Flair::ParticleController::ShapeModule::set_position(
                    system, Vector3(0.0f, 0.0f, 0.0f));
                Flair::ParticleController::ShapeModule::set_rotation(
                    system, Vector3(0.0f, 0.0f, 0.0f));
                Flair::ParticleController::ShapeModule::set_scale(
                    system, Vector3(20.0f, 1.0f, 20.0f));

                // Restore the original 1.40.8 world-collision settings.
                // Generated 1.45.3 CollisionModule exposes no public setters.
                // Use the existing guarded Unity particle ICALL compatibility layer.
                Flair::ParticleController::CollisionModule::set_enabled(system, true);
                Flair::ParticleController::CollisionModule::set_type(
                    system, Flair::ParticleController::CollisionType::World);
                LayerMask allLayers{};
                allLayers.m_Mask = -1;
                Flair::ParticleController::CollisionModule::set_collidesWith(system, allLayers);
                Flair::ParticleController::CollisionModule::set_dampen(
                    system, Flair::ParticleController::MinMaxCurve(0.08f));
                Flair::ParticleController::CollisionModule::set_bounce(
                    system, Flair::ParticleController::MinMaxCurve(0.0f));
                Flair::ParticleController::CollisionModule::set_lifetimeLoss(
                    system, Flair::ParticleController::MinMaxCurve(0.02f));

                // A cloned DustPS may contain already-simulated particles.
                // Reset it so five clones do not multiply the initial burst.
                system->Stop(true);
                system->Clear(true);

                auto emission = system->emission;
                // Disable the DustPS automatic rate and any inherited bursts.
                // UpdateLeafFloor emits exactly five leaves per second across
                // all five variants, preserving the original single-emitter
                // cadence rather than multiplying inherited dust emissions.
                emission.set_rateOverTime(emissionRate);
                emission.set_enabled(false);

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
        if (!leafObjects[0]) return;

        // The original 1.40.8 emitter produces five leaves per second.
        // One shared clock and round-robin selection reproduce that timing
        // without allowing five cloned DustPS burst schedules to stack.
        emissionAccumulator += Time::get_deltaTime() * 5.0f;
        int emittedThisFrame = 0;
        while (emissionAccumulator >= 1.0f && emittedThisFrame < 3) {
            auto& leaf = leafObjects[nextLeafVariant];
            if (leaf) {
                auto system = leaf->GetComponent<ParticleSystem*>();
                if (system) {
                    // Restore the last Quest-visible emission path.
                    // Explicit EmitParams positions made leaves invisible on device.
                    system->Emit(1);
                }
            }
            nextLeafVariant = (nextLeafVariant + 1) % LeafVariants;
            emissionAccumulator -= 1.0f;
            ++emittedThisFrame;
            ++diagnosticsEmitted;
        }
        // Prevent a paused menu from accumulating an emission backlog.
        if (emissionAccumulator > 3.0f) emissionAccumulator = 0.0f;

        // World collision handles floor contact; do not delete particles midair.
        // Keep the proven direct Emit(1) path unchanged.
        // Quest diagnostics: sample observed particles rather than assuming
        // five cloned DustPS emitters follow the 1.40.8 behavior.
        diagnosticsElapsed += Time::get_deltaTime();
        if (diagnosticsElapsed >= 5.0f) {
            int alive = 0;
            for (auto& leaf : leafObjects) {
                if (!leaf) continue;
                auto ps = leaf->GetComponent<ParticleSystem*>();
                if (ps) alive += ps->get_particleCount();
            }
            INFO("AutumnLeaves1453 DIAG seconds={} emitted={} alive={} removedByCutoff={}",
                 diagnosticsElapsed, diagnosticsEmitted, alive, 0);
            diagnosticsElapsed = 0.0f;
            diagnosticsEmitted = 0;

        }


    }

    void DestroyAutumnLeaves() {
        emissionAccumulator = 0.0f;
        nextLeafVariant = 0;
        RemoveLeafClones();
        if (dustGo) dustGo->active = true;
    }
}
