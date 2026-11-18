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
