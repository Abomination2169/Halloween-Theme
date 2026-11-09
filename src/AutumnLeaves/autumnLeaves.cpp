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
#include "UnityEngine/ParticleSystemRenderer.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Texture2D.hpp"
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

        // The original PNG is a five-column atlas. Since the 1.45.3
        // TextureSheetAnimationModule has no generated setters, crop each
        // column to its own RGBA texture and run five lightweight emitters.
        // This avoids relying on shader UV tiling, which Sprites/Default may
        // ignore, and prevents an entire atlas appearing on each particle.
        auto atlas = BSML::Utilities::LoadTextureRaw(
            static_cast<ArrayW<uint8_t>>(IncludedAssets::autumn_leaves_png));
        if (!atlas) {
            WARNING("Autumn leaves: leaf atlas unavailable; retaining original menu dust");
            return;
        }

        const int atlasWidth = atlas->get_width();
        const int atlasHeight = atlas->get_height();
        if (atlasWidth < LeafVariants || atlasHeight <= 0) {
            WARNING("Autumn leaves: invalid leaf atlas dimensions; retaining original menu dust");
            return;
        }

        auto shader = Shader::Find("Sprites/Default");
        if (!shader) {
            WARNING("Autumn leaves: Sprites/Default shader unavailable; retaining original menu dust");
            return;
        }

        try {
            for (int i = 0; i < LeafVariants; ++i) {
                const int left = atlasWidth * i / LeafVariants;
                const int right = atlasWidth * (i + 1) / LeafVariants;
                const int width = right - left;

                auto pixels = atlas->GetPixels(left, 0, width, atlasHeight);
                auto tile = Texture2D::New_ctor(width, atlasHeight);
                if (!tile || pixels.size() != static_cast<size_t>(width * atlasHeight)) {
                    WARNING("Autumn leaves: could not create an individual leaf texture");
                    RemoveLeafClones();
                    return;
                }
                tile->SetPixels(pixels);
                tile->Apply(false);

                auto clone = GameObject::Instantiate(dustGo->gameObject);
                if (!clone) {
                    WARNING("Autumn leaves: dust clone failed; retaining original menu dust");
                    RemoveLeafClones();
                    return;
                }
                leafObjects[i] = clone;
                clone->name = "AutumnLeavesPS";
                clone->active = true;
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

        // Only remove the original dust once all five emitters are configured.
        dustGo->active = false;
        INFO("Autumn leaves: five individual leaf textures active; original menu dust disabled");
    }

    void DestroyAutumnLeaves() {
        RemoveLeafClones();
        if (dustGo) dustGo->active = true;
    }
}
