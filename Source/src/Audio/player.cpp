#include "Audio/player.hpp"
#include "main.hpp"
#include "logging.hpp"

#include "GlobalNamespace/MediaAsyncLoader.hpp"
#include "System/Threading/Tasks/Tasks.hpp"
#include "custom-types/shared/delegate.hpp"

#include <string>

// The supplied track is copied to this path by the QMOD installer.
static const std::string menuSongPath =
    "/sdcard/ModData/com.beatgames.beatsaber/Mods/Halloween/Ghostly_Horror_Menu.ogg";

void PlayRandomSong(GlobalNamespace::SongPreviewPlayer* songPreviewPlayer) {
    if (!songPreviewPlayer || !fileexists(menuSongPath)) {
        PaperLogger.info("Halloween menu track not found; keeping the default menu audio.");
        return;
    }

    auto loadSongTask = GlobalNamespace::MediaAsyncLoader::New_ctor()->LoadAudioClipFromFilePathAsync(menuSongPath);
    loadSongTask->GetAwaiter().OnCompleted(custom_types::MakeDelegate<System::Action*>(std::function([songPreviewPlayer, loadSongTask]() {
        auto clip = loadSongTask->get_Result();
        if (songPreviewPlayer && clip) {
            songPreviewPlayer->_defaultAudioClip = clip;
            PaperLogger.info("Halloween menu track loaded.");
        }
    })));
}
