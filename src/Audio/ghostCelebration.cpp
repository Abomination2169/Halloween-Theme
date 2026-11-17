#include "main.hpp"
#include "Audio/ghostCelebration.hpp"
#include "logging.hpp"
#include "assets.hpp"
#include "bsml/shared/Helpers/utilities.hpp"
#include "AutumnLeaves/particleController.hpp"

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

using namespace UnityEngine;\nusing namespace Flair;\n\nnamespace Halloween::ghostCelebration {
    namespace {
        const std::array<std::string, 3> soundPaths = {
            "/sdcard/ModData/com.beatgames.beatsaber/Mods/Halloween/ghost_moan_01.ogg",
            "/sdcard/ModData/com.beatgames.beatsaber/Mods/Halloween/ghost_moan_02.ogg",
            "/sdcard/ModData/com.beatgames.beatsaber/Mods/Halloween/ghost_breath.ogg"
        };

        std::vector<UnityEngine::AudioClip*> ghostClips;
        bool loadStarted = false;
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
        if (!controller || !controller->_audioSource || ghostClips.empty()) return false;

        const auto index = static_cast<std::size_t>(std::rand()) % ghostClips.size();
        auto clip = ghostClips[index];
        if (!clip) return false;

        controller->_audioSource->set_clip(clip);
        controller->_audioSource->Play();
        return true;
    }
}
