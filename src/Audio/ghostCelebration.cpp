#include "main.hpp"
#include "Audio/ghostCelebration.hpp"
#include "logging.hpp"
#include "assets.hpp"
#include "AutumnLeaves/particleController.hpp"
#include "bsml/shared/Helpers/utilities.hpp"

#include "GlobalNamespace/FireworkItemController.hpp"
#include "GlobalNamespace/MediaAsyncLoader.hpp"
#include "UnityEngine/AudioSource.hpp"
#include "UnityEngine/AudioClip.hpp"
#include "UnityEngine/ParticleSystem.hpp"
#include "UnityEngine/ParticleSystemRenderer.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/Color.hpp"
#include "System/Threading/Tasks/Tasks.hpp"
#include "custom-types/shared/delegate.hpp"

#include <array>
#include <cstdlib>
#include <string>
#include <vector>

namespace Halloween::ghostCelebration {
    namespace {
        const std::array<std::string, 3> soundPaths = {
            "/sdcard/ModData/com.beatgames.beatsaber/Mods/Halloween/ghost_moan_01.ogg",
            "/sdcard/ModData/com.beatgames.beatsaber/Mods/Halloween/ghost_moan_02.ogg",
            "/sdcard/ModData/com.beatgames.beatsaber/Mods/Halloween/ghost_breath.ogg"
        };

        std::vector<UnityEngine::AudioClip*> ghostClips;
        bool loadStarted = false;
        UnityEngine::Texture2D* ghostTexture = nullptr;
        bool textureLoadAttempted = false;
    }

    void LoadSounds() {
        if (loadStarted) return;
        loadStarted = true;

        for (const auto& path : soundPaths) {
            if (!fileexists(path)) {
                PaperLogger.info("Ghost celebration audio missing: {}", path);
                continue;
            }

            auto loadTask = GlobalNamespace::MediaAsyncLoader::New_ctor()->LoadAudioClipFromFilePathAsync(path);
            loadTask->GetAwaiter().OnCompleted(custom_types::MakeDelegate<System::Action*>(std::function([loadTask, path]() {
                auto clip = loadTask->get_Result();
                if (clip) {
                    ghostClips.push_back(clip);
                    PaperLogger.info("Loaded ghost celebration audio: {}", path);
                } else {
                    PaperLogger.info("Failed to load ghost celebration audio: {}", path);
                }
            })));
        }
    }

    bool PlaySound(GlobalNamespace::FireworkItemController* controller) {
        // Unity objects should not be created from modloader's early startup path.
        // Defer MediaAsyncLoader creation until the results-screen firework hook runs.
        if (!loadStarted) LoadSounds();

        if (!controller || !controller->_audioSource || ghostClips.empty()) return false;

        const auto index = static_cast<std::size_t>(std::rand()) % ghostClips.size();
        auto clip = ghostClips[index];
        if (!clip) return false;

        controller->_audioSource->set_clip(clip);
        controller->_audioSource->Play();
        return true;
    }

    void ApplyGhostParticles(GlobalNamespace::FireworkItemController* controller) {
        if (!controller || !controller->_particleSystems) return;

        if (!textureLoadAttempted) {
            textureLoadAttempted = true;
            ghostTexture = BSML::Utilities::LoadTextureRaw(
                static_cast<ArrayW<uint8_t>>(IncludedAssets::ghost_particle_png)
            );
        }

        auto ghostShader = UnityEngine::Shader::Find("Sprites/Default");
        int updated = 0;

        for (auto* entry : controller->_particleSystems) {
            if (!entry || !entry->_particleSystem) continue;

            auto* particleSystem = entry->_particleSystem.ptr();
            auto* renderer = particleSystem->GetComponent<UnityEngine::ParticleSystemRenderer*>();
            if (renderer && ghostTexture && ghostShader) {
                auto material = renderer->get_material();
                if (material) {
                    material->set_shader(ghostShader);
                    material->SetTexture(StringW("_MainTex"), ghostTexture);
                    material->SetColor(StringW("_Color"), UnityEngine::Color(0.90f, 0.96f, 1.0f, 0.78f));
                }
            }

            Flair::ParticleController::MainModule::set_startLifetime(particleSystem, Flair::ParticleController::MinMaxCurve(3.2f));
            Flair::ParticleController::MainModule::set_startSpeed(particleSystem, Flair::ParticleController::MinMaxCurve(0.55f));
            Flair::ParticleController::MainModule::set_startSize(particleSystem, Flair::ParticleController::MinMaxCurve(0.32f));
            Flair::ParticleController::MainModule::set_gravityModifier(particleSystem, Flair::ParticleController::MinMaxCurve(-0.04f));

            Flair::ParticleController::MinMaxGradient tint{};
            tint.mode = Flair::ParticleController::GradientMode::Color;
            tint.colorMin = UnityEngine::Color(0.90f, 0.96f, 1.0f, 0.78f);
            tint.colorMax = UnityEngine::Color(0.90f, 0.96f, 1.0f, 0.78f);
            Flair::ParticleController::MainModule::set_startColor(particleSystem, tint);

            ++updated;
        }

        PaperLogger.info("Applied ghost appearance to {} results celebration particle systems", updated);
    }
}
