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
#include <utility>

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
        // Five recordings included with the v1.0.1 public QMOD; details and
        // attribution are in GHOST_AUDIO_CREDITS.txt / PIXABAY_AUDIO_CREDITS.txt.
        const std::array<std::string, 5> voicePaths = {{
            "/sdcard/ModData/com.beatgames.beatsaber/Mods/Halloween/ghost_ogrebane.ogg",
            "/sdcard/ModData/com.beatgames.beatsaber/Mods/Halloween/ghost_scary_1.ogg",
            "/sdcard/ModData/com.beatgames.beatsaber/Mods/Halloween/ghost_breath.ogg",
            "/sdcard/ModData/com.beatgames.beatsaber/Mods/Halloween/ghost_pixabay_creepy.ogg",
            "/sdcard/ModData/com.beatgames.beatsaber/Mods/Halloween/ghost_pixabay_wind.ogg"
        }};
        std::array<UnityEngine::AudioClip*, 5> voiceClips = {nullptr, nullptr, nullptr, nullptr, nullptr};
        bool voiceLoadingStarted = false;

        // Shuffle-bag rotation: each loaded recording plays once per cycle
        // before any track repeats. Keep the last choice across results screens
        // to avoid immediately repeating a sound at the cycle boundary.
        std::array<std::size_t, 5> voiceOrder{};
        std::size_t voiceOrderLength = 0;
        std::size_t voiceOrderCursor = 0;
        int lastVoiceIndex = -1;

        int NextVoiceIndex() {
            if (voiceOrderCursor >= voiceOrderLength) {
                voiceOrderLength = 0;
                voiceOrderCursor = 0;
                for (std::size_t i = 0; i < voiceClips.size(); ++i) {
                    if (voiceClips[i]) voiceOrder[voiceOrderLength++] = i;
                }
                if (!voiceOrderLength) return -1;
                // Fisher-Yates shuffle, then avoid a consecutive repeat
                // across cycles if more than one recording is available.
                for (std::size_t i = voiceOrderLength; i > 1; --i) {
                    const std::size_t j = static_cast<std::size_t>(std::rand()) % i;
                    std::swap(voiceOrder[i - 1], voiceOrder[j]);
                }
                if (voiceOrderLength > 1 &&
                    static_cast<int>(voiceOrder[0]) == lastVoiceIndex) {
                    std::swap(voiceOrder[0], voiceOrder[1]);
                }
            }
            const int selected = static_cast<int>(voiceOrder[voiceOrderCursor++]);
            lastVoiceIndex = selected;
            return selected;
        }

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
        // Approximately 42% of spawned ghosts produce a sound.
        // The other 58% remain silent to prevent a crowded soundscape.
        if (std::rand() % 100 >= 42) return;
        auto* src = ghost->AddComponent<UnityEngine::AudioSource*>();
        if (!src) return;
        const int index = NextVoiceIndex();
        if (index < 0) {
            UnityEngine::Object::Destroy(src);
            return;  // No clips loaded yet; never invent a sound.
        }
        auto* clip = voiceClips[static_cast<std::size_t>(index)];
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
