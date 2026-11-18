#include "Audio/ghostSting.hpp"
#include "main.hpp"
#include "logging.hpp"
#include "GlobalNamespace/MediaAsyncLoader.hpp"
#include "UnityEngine/AudioClip.hpp"
#include "UnityEngine/AudioSource.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Object.hpp"
#include "System/Threading/Tasks/Tasks.hpp"
#include "custom-types/shared/delegate.hpp"

#include <functional>
#include <string>
#include <array>
#include <cstdlib>

namespace Halloween::ghostSting {
    namespace {
        // User-installed sound only: never bundle third-party audio in the
        // public source repo without a verified distribution license.
        const std::string path =
            "/sdcard/ModData/com.beatgames.beatsaber/Mods/Halloween/ghost_celebration_sting.ogg";
        UnityEngine::AudioClip* cachedClip = nullptr;
        SafePtrUnity<UnityEngine::GameObject> host;
        bool loading = false;
        bool active = false;
        bool played = false;
        // Three distinct OpenGameArt recordings: Ogrebane (CC0), Berklee (CC BY 3.0), and qubodup (CC0).
        // First three recordings are release-cleared OpenGameArt assets.
        // Final two are OPTIONAL personal Pixabay files: deliberately absent
        // from the public QMOD manifest pending redistribution clearance.
        const std::array<std::string, 5> voicePaths = {{
            "/sdcard/ModData/com.beatgames.beatsaber/Mods/Halloween/ghost_ogrebane.ogg",
            "/sdcard/ModData/com.beatgames.beatsaber/Mods/Halloween/ghost_scary_1.ogg",
            "/sdcard/ModData/com.beatgames.beatsaber/Mods/Halloween/ghost_breath.ogg",
            "/sdcard/ModData/com.beatgames.beatsaber/Mods/Halloween/ghost_pixabay_creepy.ogg",
            "/sdcard/ModData/com.beatgames.beatsaber/Mods/Halloween/ghost_pixabay_wind.ogg"
        }};
        std::array<UnityEngine::AudioClip*, 5> voiceClips = {nullptr, nullptr, nullptr, nullptr, nullptr};
        bool voiceLoadingStarted = false;

        void LoadOriginalVoices() {
            if (voiceLoadingStarted) return;
            voiceLoadingStarted = true;
            for (std::size_t i = 0; i < voicePaths.size(); ++i) {
                if (!fileexists(voicePaths[i])) {
                    PaperLogger.info("Original ghost sound missing: {}", voicePaths[i]);
                    continue;
                }
                auto task = GlobalNamespace::MediaAsyncLoader::New_ctor()
                    ->LoadAudioClipFromFilePathAsync(voicePaths[i]);
                task->GetAwaiter().OnCompleted(
                    custom_types::MakeDelegate<System::Action*>(
                        std::function([task, i]() {
                            auto clip = task->get_Result();
                            if (clip) voiceClips[i] = clip.ptr();
                        }))
                );
            }
        }


        void PlayLoadedOnce() {
            if (!active || played || !host || !cachedClip) return;
            auto* src = host->GetComponent<UnityEngine::AudioSource*>();
            if (!src) return;
            played = true;  // Mark before Play to prevent duplicate callbacks.
            src->set_loop(false);
            src->set_playOnAwake(false);
            src->set_spatialBlend(0.0f);  // Full stereo, screen-wide celebration.
            src->set_volume(0.75f);
            src->set_clip(cachedClip);
            src->Play();
            PaperLogger.info("Playing one-shot ghost celebration sting");
        }

        void RequestClip() {
            if (loading || cachedClip || !fileexists(path)) return;
            loading = true;
            auto task = GlobalNamespace::MediaAsyncLoader::New_ctor()
                ->LoadAudioClipFromFilePathAsync(path);
            task->GetAwaiter().OnCompleted(
                custom_types::MakeDelegate<System::Action*>(std::function([task]() {
                    loading = false;
                    auto clip = task->get_Result();
                    if (!clip) {
                        PaperLogger.info("Unable to load celebration sting");
                        return;
                    }
                    cachedClip = clip.ptr();
                    PlayLoadedOnce();
                }))
            );
        }
    }

    void EnterResults() {
        ExitResults();
        active = true;
        played = false;
        LoadOriginalVoices();
        if (!fileexists(path)) {
            PaperLogger.info("Optional ghost sting not installed; visuals unaffected");
            return;
        }
        auto* obj = UnityEngine::GameObject::New_ctor("HalloweenGhostEntrySting");
        host = obj;
        auto* src = obj->AddComponent<UnityEngine::AudioSource*>();
        if (!src) {
            UnityEngine::Object::Destroy(obj);
            host = nullptr;
            return;
        }
        src->set_playOnAwake(false);
        src->set_loop(false);
        if (cachedClip) PlayLoadedOnce();
        else RequestClip();
    }

    void PlaySpawnVoice(UnityEngine::GameObject* ghost) {
        if (!active || !ghost) return;
        // Not every ghost speaks; avoid overlapping cartoon voices.
        if (std::rand() % 100 >= 42) return;
        // Choose uniformly among successfully loaded recordings. A missing
        // optional clip never reduces the probability of the remaining voices.
        std::array<UnityEngine::AudioClip*, 5> available{};
        std::size_t count = 0;
        for (auto* candidate : voiceClips)
            if (candidate) available[count++] = candidate;
        if (!count) return;
        auto* clip = available[static_cast<std::size_t>(std::rand()) % count];
        if (!clip) return; // First result may still be loading.
        auto* src = ghost->AddComponent<UnityEngine::AudioSource*>();
        if (!src) return;
        src->set_playOnAwake(false);
        src->set_loop(false);
        src->set_spatialBlend(0.0f);
        src->set_volume(0.38f);
        src->set_clip(clip);
        src->Play();
    }

    void ExitResults() {
        active = false;
        played = false;
        if (host) {
            auto* src = host->GetComponent<UnityEngine::AudioSource*>();
            if (src) src->Stop();
            UnityEngine::Object::Destroy(host.ptr());
            host = nullptr;
        }
    }
}
