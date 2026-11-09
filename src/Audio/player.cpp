#include "Audio/player.hpp"
#include "main.hpp"
#include "logging.hpp"
#include "c.hpp"

#include "GlobalNamespace/MediaAsyncLoader.hpp"
#include "System/Threading/Tasks/Tasks.hpp"
#include "custom-types/shared/delegate.hpp"
#include "beatsaber-hook/shared/safeptr.hpp"

#include <functional>
#include <string>

// Installer-provided OGG. Leave Beat Saber's audio alone if the file is missing.
static const std::string menuSongPath =
    "/sdcard/ModData/com.beatgames.beatsaber/Mods/Halloween/Ghostly_Horror_Menu.ogg";

namespace {
    // Hold a managed reference across menu transitions, and prevent starting
    // overlapping async loads when OnEnable and CrossfadeTo fire close together.
    safe_ptr<UnityEngine::AudioClip*> cachedMenuClip;
    bool loadingMenuClip = false;
}

void PlayRandomSong(GlobalNamespace::SongPreviewPlayer* songPreviewPlayer) {
    if (!songPreviewPlayer || !getModConfig().EnableMenuMusic.GetValue()) return;

    if (cachedMenuClip) {
        songPreviewPlayer->_defaultAudioClip = cachedMenuClip.ptr();
        return;
    }

    if (loadingMenuClip) return;

    if (!fileexists(menuSongPath)) {
        PaperLogger.info("Halloween menu track not found; keeping the default menu audio.");
        return;
    }

    auto loader = GlobalNamespace::MediaAsyncLoader::New_ctor();
    if (!loader) {
        PaperLogger.info("Halloween audio loader unavailable; keeping the default menu audio.");
        return;
    }

    auto loadTask = loader->LoadAudioClipFromFilePathAsync(menuSongPath);
    if (!loadTask) {
        PaperLogger.info("Halloween audio load task unavailable; keeping the default menu audio.");
        return;
    }

    loadingMenuClip = true;
    try {
        // Both objects must remain GC-reachable until the task finishes.
        safe_ptr<GlobalNamespace::SongPreviewPlayer*> playerHandle(songPreviewPlayer);
        safe_ptr<decltype(loadTask)> taskHandle(loadTask);
        loadTask->GetAwaiter().OnCompleted(
            custom_types::MakeDelegate<System::Action*>(std::function<void()>(
                [playerHandle, taskHandle]() mutable {
                    loadingMenuClip = false;
                    try {
                        auto clip = taskHandle->get_Result();
                        if (!clip) {
                            PaperLogger.info("Halloween audio load returned no clip; keeping the default menu audio.");
                            return;
                        }
                        cachedMenuClip = clip;
                        if (playerHandle && getModConfig().EnableMenuMusic.GetValue()) {
                            playerHandle->_defaultAudioClip = clip;
                            PaperLogger.info("Halloween menu track loaded.");
                        }
                    } catch (...) {
                        PaperLogger.info("Halloween audio load failed; keeping the default menu audio.");
                    }
                }
            ))
        );
    } catch (...) {
        loadingMenuClip = false;
        PaperLogger.info("Halloween audio callback setup failed; keeping the default menu audio.");
    }
}
