#include "AutumnLeaves/autumnLeaves.hpp"
#include "main.hpp"
#include "beatsaber-hook/shared/safeptr.hpp"
#include "assets.hpp"
#include "UnityEngine/Shader.hpp"
#include "logging.hpp"


#include "AutumnLeaves/particleController.hpp"

#include "UnityEngine/Resources.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "UnityEngine/ParticleSystemRenderer.hpp"
#include "UnityEngine/Material.hpp"

#include "bsml/shared/Helpers/utilities.hpp"

#include "UnityEngine/Shader.hpp"
#include "UnityEngine/Color.hpp"

using namespace GlobalNamespace;
using namespace UnityEngine;
using namespace Flair;

namespace Halloween::autumnLeaves {

    safe_ptr<GameObject*> dustGo;
    safe_ptr<GameObject*> autumnLeavesGo;

    void CreateAutumnLeaves() {
        // Don't create if already created
        if(autumnLeavesGo) return;

        // Find the dust GameObject for a template ParticleSystem
        if(!dustGo) for (auto particle : Resources::FindObjectsOfTypeAll<ParticleSystem*>())
            if (particle->name == "DustPS") {
                dustGo = particle->gameObject;
                break;
            }

        // If the template dust wasn't found, don't create the leaves
        if(!dustGo) return;


        // Create the autumnLeaves GameObject from the dust
        autumnLeavesGo = GameObject::Instantiate(dustGo->gameObject);
        if (!autumnLeavesGo) {
            WARNING("Autumn leaves: failed to clone menu dust; retaining original particle system");
            return;
        }
        autumnLeavesGo->name = "AutumnLeavesPS";
        autumnLeavesGo->active = true;
        // GameObject::DontDestroyOnLoad(autumnLeavesGo->gameObject); // This line will enable autumnLeaves in game

        // Get components from the autumnLeaves
        UnityEngine::ParticleSystem* autumnLeaves = autumnLeavesGo->GetComponent<ParticleSystem*>();
        UnityEngine::ParticleSystemRenderer* particleRenderer = autumnLeavesGo->GetComponent<UnityEngine::ParticleSystemRenderer*>();
        if (!autumnLeaves || !particleRenderer) {
            WARNING("Autumn leaves: cloned dust lacks particle system or renderer; removing clone");
            GameObject::Destroy(autumnLeavesGo->gameObject);
            autumnLeavesGo = nullptr;
            return;
        }
        
        // Set up autumnLeavesGo where it needs to be
        autumnLeavesGo->transform->parent = nullptr;
        autumnLeavesGo->transform->position = UnityEngine::Vector3(0.0f, 7.0f, 0.0f);
        autumnLeavesGo->transform->rotation = UnityEngine::Quaternion::Euler(UnityEngine::Vector3(0.0f, 0.0f, 0.0f));

        // Only use methods present in Beat Saber 1.45.3's generated
        // IL2CPP bindings. The older manually-resolved _Injected symbols
        // are not available in this version.
        UnityEngine::ParticleSystem::MinMaxCurve lifetime{};
        lifetime._ctor(14.0f);
        UnityEngine::ParticleSystem::MinMaxCurve emissionRate{};
        emissionRate._ctor(5.0f);
        UnityEngine::ParticleSystem::MinMaxGradient white{};
        white._ctor(UnityEngine::Color(1, 1, 1, 1));

        auto mainModule = autumnLeaves->main;
        mainModule.set_startLifetime(lifetime);
        mainModule.set_startSizeMultiplier(0.18f);
        mainModule.set_maxParticles(150);
        mainModule.set_gravityModifierMultiplier(0.025f);
        mainModule.set_startColor(white);

        auto emission = autumnLeaves->emission;
        emission.set_enabled(true);
        emission.set_rateOverTime(emissionRate);
        auto shape = autumnLeaves->shape;
        shape.set_rotation(UnityEngine::Vector3(0, 0, 0));
        // Other shape, collision and texture-sheet animation setters were
        // stripped from the generated game bindings; do not call them.
        const int numLeafSprites = 5;
        auto leafTexture = BSML::Utilities::LoadTextureRaw(static_cast<ArrayW<uint8_t>>(IncludedAssets::autumn_leaves_png));
        auto leafMaterial = particleRenderer->get_material();
        if (!leafMaterial || !leafTexture) {
            WARNING("Autumn leaves: texture or particle material unavailable; removing clone");
            GameObject::Destroy(autumnLeavesGo->gameObject);
            autumnLeavesGo = nullptr;
            return;
        }
        leafMaterial->set_mainTexture(leafTexture);
// Verified Quest fix: Sprites/Default samples the RGBA leaf spritesheet in full color.
// Do not use Particles/Standard Unlit here: it rendered the leaves white.
auto leafShader = UnityEngine::Shader::Find("Sprites/Default");
if (leafShader && particleRenderer) {
    if (leafMaterial) {
        leafMaterial->set_shader(leafShader);
        leafMaterial->SetTexture(
            StringW("_MainTex"),
            leafTexture
        );
        leafMaterial->SetColor(StringW("_Color"), UnityEngine::Color(1, 1, 1, 1));
    }
} else {
    WARNING("Autumn leaves: Sprites/Default shader not found; using original material");
}
        // TextureSheetAnimationModule has no generated setters in 1.45.3.
        // Keep the inherited emitter properties while using our leaf texture.

        // Keep the template dust active until the leaves have been configured.
        // Only replace the original menu effect after the clone is ready.
        if (dustGo) dustGo->active = false;
    }

    void DestroyAutumnLeaves() {
        if (autumnLeavesGo) {
            GameObject::Destroy(autumnLeavesGo->gameObject);
            autumnLeavesGo = nullptr;
        }
        // Restore the original effect when Halloween leaves are removed.
        if (dustGo) dustGo->active = true;
    }

}