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

        // 1.45.3 headset log confirms the legacy _Injected lookups all failed.
        // Use the generated Unity bindings rather than obsolete manually-resolved
        // ParticleSystem module internal calls. These are versioned with the game.
        auto mainModule = autumnLeaves->main;
        mainModule.set_startLifetime(UnityEngine::ParticleSystem::MinMaxCurve(14.0f));
        mainModule.set_startSize(UnityEngine::ParticleSystem::MinMaxCurve(0.18f));
        mainModule.set_maxParticles(150);
        mainModule.set_gravityModifier(UnityEngine::ParticleSystem::MinMaxCurve(0.025f));
        mainModule.set_startColor(UnityEngine::ParticleSystem::MinMaxGradient(UnityEngine::Color(1, 1, 1, 1)));

        auto colorOverLifetime = autumnLeaves->colorOverLifetime;
        colorOverLifetime.set_enabled(false);
        auto colorBySpeed = autumnLeaves->colorBySpeed;
        colorBySpeed.set_enabled(false);
        auto emission = autumnLeaves->emission;
        emission.set_enabled(true);
        emission.set_rateOverTime(UnityEngine::ParticleSystem::MinMaxCurve(5.0f));
        auto shape = autumnLeaves->shape;
        shape.set_shapeType(UnityEngine::ParticleSystemShapeType::Box);
        shape.set_position(UnityEngine::Vector3(0, 0, 0));
        shape.set_rotation(UnityEngine::Vector3(0, 0, 0));
        shape.set_scale(UnityEngine::Vector3(20, 1, 20));
        auto collision = autumnLeaves->collision;
        collision.set_enabled(true);
        collision.set_type(UnityEngine::ParticleSystemCollisionType::World);
        collision.set_collidesWith(UnityEngine::LayerMask(0xffffffff));
        collision.set_dampen(UnityEngine::ParticleSystem::MinMaxCurve(0.08f));
        collision.set_bounce(UnityEngine::ParticleSystem::MinMaxCurve(0.0f));
        collision.set_lifetimeLoss(UnityEngine::ParticleSystem::MinMaxCurve(0.02f));
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
        auto textureSheet = autumnLeaves->textureSheetAnimation;
        textureSheet.set_enabled(true);
        textureSheet.set_mode(UnityEngine::ParticleSystemAnimationMode::Grid);
        textureSheet.set_numTilesX(numLeafSprites);
        textureSheet.set_numTilesY(1);
        textureSheet.set_frameOverTime(UnityEngine::ParticleSystem::MinMaxCurve(0.0f, numLeafSprites));
        textureSheet.set_startFrame(UnityEngine::ParticleSystem::MinMaxCurve(0.0f, numLeafSprites));
        textureSheet.set_cycleCount(0);

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