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
#include "UnityEngine/Vector3.hpp"
#include "System/Threading/Tasks/Tasks.hpp"
#include "custom-types/shared/delegate.hpp"

#include <array>
#include <cstdlib>
#include <string>
#include <vector>

namespace Halloween::ghostCelebration {
    namespace {
        // Only user-installed recordings are used: these Pixabay tracks are
        // not uploaded to or redistributed by the public source repository.
        const std::array<std::string, 2> cartoonSoundPaths = {
            "/sdcard/ModData/com.beatgames.beatsaber/Mods/Halloween/ghost_cartoon_wooo.mp3",
            "/sdcard/ModData/com.beatgames.beatsaber/Mods/Halloween/ghost_woo.mp3"
        };
        const std::array<std::string, 3> fallbackSoundPaths = {
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

        const bool useCartoon =
            fileexists(cartoonSoundPaths[0]) && fileexists(cartoonSoundPaths[1]);
        PaperLogger.info(useCartoon
            ? "Using both user-supplied cartoon ghost recordings"
            : "Cartoon ghost audio not yet installed; using previous ghost sounds");

        const auto loadOne = [](const std::string& path) {
            if (!fileexists(path)) {
                PaperLogger.info("Ghost celebration audio missing: {}", path);
                return;
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
        };
        if (useCartoon) {
            for (const auto& path : cartoonSoundPaths) loadOne(path);
        } else {
            for (const auto& path : fallbackSoundPaths) loadOne(path);
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

        // The previous 32-centimeter circular particle looked like a blue bubble.
        // The atlas now has four complete, alpha-transparent ghost designs.
        if (!textureLoadAttempted) {
            textureLoadAttempted = true;
            ghostTexture = BSML::Utilities::LoadTextureRaw(
                static_cast<ArrayW<uint8_t>>(IncludedAssets::ghost_sprites_png)
            );
        }
        if (!ghostTexture) {
            PaperLogger.info("Ghost celebration atlas could not be loaded");
            return;
        }

        auto ghostShader = UnityEngine::Shader::Find("Sprites/Default");
        int updated = 0;

        for (auto* entry : controller->_particleSystems) {
            if (!entry || !entry->_particleSystem) continue;

            auto* particleSystem = entry->_particleSystem.ptr();
            auto* renderer = particleSystem->GetComponent<UnityEngine::ParticleSystemRenderer*>();
            if (renderer && ghostShader) {
                auto material = renderer->get_material();
                if (material) {
                    material->set_shader(ghostShader);
                    material->SetTexture(StringW("_MainTex"), ghostTexture);
                    material->SetColor(
                        StringW("_Color"),
                        UnityEngine::Color(1.0f, 1.0f, 1.0f, 0.70f)
                    );
                }
            }

            using Particles = Flair::ParticleController;
            // Four columns, one ghost per cell. Choose a random cell once at
            // birth; do NOT animate through all four faces within one particle.
            Particles::TextureSheetAnimationModule::set_enabled(particleSystem, true);
            Particles::TextureSheetAnimationModule::set_mode(
                particleSystem, Particles::AnimationMode::Grid);
            Particles::TextureSheetAnimationModule::set_numTilesX(particleSystem, 4);
            Particles::TextureSheetAnimationModule::set_numTilesY(particleSystem, 1);
            Particles::TextureSheetAnimationModule::set_frameOverTime(
                particleSystem, Particles::MinMaxCurve(0.0f));
            Particles::TextureSheetAnimationModule::set_startFrame(
                particleSystem, Particles::MinMaxCurve(0.0f, 3.99f));
            Particles::TextureSheetAnimationModule::set_cycleCount(particleSystem, 1);

            // IMPORTANT: Keep Beat Saber's native firework burst scheduling.
            // Results celebrations are short-lived. The previous test replaced
            // native burst emission with a 0.45/sec continuous stream and zero
            // bursts, so the controller could stop before any ghost appeared.
            // Retaining the original burst gives an immediate visible effect.
            // v2.2: preserve the visible native burst trigger, but allow only
            // two ghosts from each firework emitter. This prevents the bright
            // overlapping clusters observed in the Quest screenshots.
            Particles::MainModule::set_maxParticles(particleSystem, 2);
            Particles::MainModule::set_startLifetime(
                particleSystem, Particles::MinMaxCurve(5.0f, 7.0f));
            Particles::MainModule::set_startSpeed(
                particleSystem, Particles::MinMaxCurve(0.10f, 0.22f));
            Particles::MainModule::set_startSize(
                particleSystem, Particles::MinMaxCurve(1.45f, 1.90f));
            Particles::MainModule::set_gravityModifier(
                particleSystem, Particles::MinMaxCurve(0.0f));
            Particles::MainModule::set_startRotation3D(particleSystem, false);
            Particles::MainModule::set_startRotation(
                particleSystem, Particles::MinMaxCurve(-0.10f, 0.10f));

            // Keep the original firework burst count and activation timing.
            // Apply gentle world-space travel away from the player, with
            // a little horizontal variation between individual ghosts.
            Particles::VelocityOverLifetimeModule::set_enabled(particleSystem, true);
            Particles::VelocityOverLifetimeModule::set_space(
                particleSystem, Particles::SimulationSpace::World);
            Particles::VelocityOverLifetimeModule::set_x(
                particleSystem, Particles::MinMaxCurve(-0.07f, 0.07f));
            Particles::VelocityOverLifetimeModule::set_y(
                particleSystem, Particles::MinMaxCurve(0.30f, 0.43f));
            Particles::VelocityOverLifetimeModule::set_z(
                particleSystem, Particles::MinMaxCurve(0.10f, 0.18f));

            // Remove inherited firework tumble/physics. Keep the existing
            // color-over-lifetime fade and the four random sprite cells.
            Particles::CollisionModule::set_enabled(particleSystem, false);
            Particles::RotationOverLifetimeModule::set_enabled(particleSystem, false);
            Particles::SizeOverLifetimeModule::set_enabled(particleSystem, false);
            Particles::NoiseModule::set_enabled(particleSystem, false);

            Particles::MinMaxGradient tint{};
            tint.mode = Particles::GradientMode::Color;
            tint.colorMin = UnityEngine::Color(1.0f, 1.0f, 1.0f, 0.70f);
            tint.colorMax = tint.colorMin;
            Particles::MainModule::set_startColor(particleSystem, tint);
            // Preserve the firework's existing fade-over-lifetime gradient.
            ++updated;
        }

        PaperLogger.info("Configured {} results celebration particle systems with sparse rising ghosts (v2.2)", updated);
    }
}
