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
#include "UnityEngine/LayerMask.hpp"
#include "UnityEngine/ParticleSystemRenderer.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/Color.hpp"
#include "bsml/shared/Helpers/utilities.hpp"

using namespace GlobalNamespace;
using namespace UnityEngine;
using namespace Flair;

namespace Halloween::autumnLeaves {
    namespace {
        safe_ptr<GameObject*> dustGo;
        safe_ptr<GameObject*> autumnLeavesGo;
    }

    void CreateAutumnLeaves() {
        if (autumnLeavesGo) return;

        // Original 1.40.8 design: clone DustPS ONCE and use one atlas.
        if (!dustGo) {
            for (auto particle : Resources::FindObjectsOfTypeAll<ParticleSystem*>()) {
                if (particle->name == "DustPS") {
                    dustGo = particle->gameObject;
                    break;
                }
            }
        }
        if (!dustGo) return;

        auto texture = BSML::Utilities::LoadTextureRaw(
            static_cast<ArrayW<uint8_t>>(IncludedAssets::autumn_leaves_png));
        auto shader = Shader::Find("Sprites/Default");
        if (!texture || !shader) {
            WARNING("Autumn leaves: original atlas or shader unavailable; keeping menu dust");
            return;
        }

        auto clone = GameObject::Instantiate(dustGo->gameObject);
        if (!clone) return;
        autumnLeavesGo = clone;
        clone->name = "AutumnLeavesPS";
        clone->active = false;
        clone->transform->parent = nullptr;
        clone->transform->position = Vector3(0.0f, 7.0f, 0.0f);
        clone->transform->rotation = Quaternion::Euler(Vector3(0.0f, 0.0f, 0.0f));

        auto system = clone->GetComponent<ParticleSystem*>();
        auto renderer = clone->GetComponent<ParticleSystemRenderer*>();
        if (!system || !renderer) {
            GameObject::Destroy(clone);
            autumnLeavesGo = nullptr;
            return;
        }
        auto material = renderer->get_material();
        if (!material) {
            GameObject::Destroy(clone);
            autumnLeavesGo = nullptr;
            return;
        }
        material->set_shader(shader);
        material->set_mainTexture(texture);
        material->SetTexture(StringW("_MainTex"), texture);
        material->SetColor(StringW("_Color"), Color(1, 1, 1, 1));

        // Match original 1.40.8 values, with native setters for properties
        // available in the 1.45.3 generated Unity API.
        ParticleSystem::MinMaxCurve lifetime{};
        lifetime._ctor(14.0f);
        ParticleSystem::MinMaxGradient leafColor{};
        leafColor._ctor(Color(1, 1, 1, 1));
        auto mainModule = system->main;
        mainModule.set_startLifetime(lifetime);
        mainModule.set_startSizeMultiplier(0.18f);
        mainModule.set_maxParticles(150);
        mainModule.set_gravityModifierMultiplier(0.025f);
        mainModule.set_startColor(leafColor);

        ParticleController::ColorOverLifetimeModule::set_enabled(system, false);
        ParticleController::ColorBySpeedModule::set_enabled(system, false);
        ParticleController::EmissionModule::set_enabled(system, true);
        ParticleController::EmissionModule::set_rateOverTime(system, 5.0f);
        ParticleController::ShapeModule::set_shapeType(system, ParticleController::ShapeType::Box);
        ParticleController::ShapeModule::set_position(system, Vector3(0, 0, 0));
        ParticleController::ShapeModule::set_rotation(system, Vector3(0, 0, 0));
        ParticleController::ShapeModule::set_scale(system, Vector3(20, 1, 20));

        ParticleController::CollisionModule::set_enabled(system, true);
        ParticleController::CollisionModule::set_type(system, ParticleController::CollisionType::World);
        LayerMask allLayers{};
        allLayers.m_Mask = -1;
        ParticleController::CollisionModule::set_collidesWith(system, allLayers);
        ParticleController::CollisionModule::set_dampen(system, ParticleController::MinMaxCurve(0.08f));
        ParticleController::CollisionModule::set_bounce(system, ParticleController::MinMaxCurve(0.0f));
        ParticleController::CollisionModule::set_lifetimeLoss(system, ParticleController::MinMaxCurve(0.02f));

        // Original single-emitter, five-column atlas animation.
        constexpr int numLeafSprites = 5;
        ParticleController::TextureSheetAnimationModule::set_enabled(system, true);
        ParticleController::TextureSheetAnimationModule::set_mode(system, ParticleController::AnimationMode::Grid);
        ParticleController::TextureSheetAnimationModule::set_numTilesX(system, numLeafSprites);
        ParticleController::TextureSheetAnimationModule::set_numTilesY(system, 1);
        ParticleController::TextureSheetAnimationModule::set_frameOverTime(
            system, ParticleController::MinMaxCurve(0.0f, numLeafSprites));
        ParticleController::TextureSheetAnimationModule::set_startFrame(
            system, ParticleController::MinMaxCurve(0.0f, numLeafSprites));
        ParticleController::TextureSheetAnimationModule::set_cycleCount(system, 0);

        system->Stop(true);
        system->Clear(true);
        dustGo->active = false;
        clone->active = true;
        system->Play();
        INFO("Autumn leaves: original single-emitter 1.40.8 atlas system active");
    }

    // Retained for the 1.45.3 menu update hook; the original 1.40.8
    // implementation requires no per-frame manual emission or floor cutoff.
    void UpdateLeafFloor() {}

    void DestroyAutumnLeaves() {
        if (autumnLeavesGo) {
            GameObject::Destroy(autumnLeavesGo->gameObject);
            autumnLeavesGo = nullptr;
        }
        if (dustGo) dustGo->active = true;
    }
}
