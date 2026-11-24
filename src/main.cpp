#include "main.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/MaterialPropertyBlock.hpp"
#include "UnityEngine/MeshRenderer.hpp"
#include "UnityEngine/MonoBehaviour.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Shader.hpp"


#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "GlobalNamespace/GameNoteController.hpp"
#include "GlobalNamespace/FlickeringNeonSign.hpp"
#include "GlobalNamespace/GameServerLobbyFlowCoordinator.hpp"
#include "GlobalNamespace/MultiplayerModeSelectionFlowCoordinator.hpp"
#include "GlobalNamespace/LightWithIdManager.hpp"
#include "GlobalNamespace/MainMenuViewController.hpp"
#include "GlobalNamespace/MediaAsyncLoader.hpp"

#include "logging.hpp"

#include "Menu/ObjectInstances.hpp"

#include "Audio/player.hpp"
#include "Audio/ghostCelebration.hpp"
#include "Audio/ghostDriftController.hpp"
#include "Audio/ghostSting.hpp"
#include "AutumnLeaves/particleController.hpp"
#include "custom-types/shared/register.hpp"
#include "GlobalNamespace/FireworkItemController.hpp"
#include "GlobalNamespace/ResultsViewController.hpp"
#include "GlobalNamespace/LevelCompletionResults.hpp"

#include "GlobalNamespace/SongPreviewPlayer.hpp"
#include "UnityEngine/AudioSource.hpp"
#include "UnityEngine/AudioClip.hpp"
#include "custom-types/shared/macros.hpp"

#include "UnityEngine/Time.hpp"
#include "UnityEngine/Renderer.hpp"
#include "UnityEngine/SceneManagement/SceneManager.hpp"
#include "UnityEngine/SceneManagement/SceneManagement.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/SpriteRenderer.hpp"

#include "System/Threading/Tasks/Tasks.hpp"

#include "Block/colorchanger.hpp"
#include "AutumnLeaves/autumnLeaves.hpp"
#include "Menu/pumpkinAccents.hpp"




#include "c.hpp"

#include "scotland2/shared/modloader.h"


using namespace GlobalNamespace;
using namespace UnityEngine;

namespace {
    constexpr const char* floorDiagnosticBuild = "menu-floor-restoration-v3";
    SafePtrUnity<MeshRenderer> menuFloor;
    Color menuFloorBaseline{};
    bool hasMenuFloorBaseline = false;
    bool floorRestorationActive = false;
    bool loggedFloorRestore = false;

    MeshRenderer* FindMenuFloor(int colorProperty) {
        for (auto* renderer : Resources::FindObjectsOfTypeAll<MeshRenderer*>()) {
            if (!renderer || !renderer->get_enabled()) continue;
            auto go = renderer->get_gameObject();
            if (!go || !go->get_activeInHierarchy() || go->get_name() != "BasicMenuGround") continue;
            const auto materials = renderer->get_sharedMaterials();
            if (materials.size() != 1) continue;
            auto material = materials[0];
            if (material && material->get_name() == "MenuFloor" && material->HasProperty(colorProperty))
                return renderer;
        }
        return nullptr;
    }

}

// Unity copies this block into the renderer. Restore the caller's value after
// submission in case the same block is reused for other environment meshes.
MAKE_HOOK_MATCH(MenuFloorSetPropertyBlock,
    static_cast<void (Renderer::*)(MaterialPropertyBlock*)>(&Renderer::SetPropertyBlock),
    void, Renderer* self, MaterialPropertyBlock* block) {
    if (block && hasMenuFloorBaseline && floorRestorationActive && menuFloor &&
        self == static_cast<Renderer*>(menuFloor.ptr()) && getModConfig().EnableHauntedEnvColors.GetValue()) {
        static const int colorProperty = Shader::PropertyToID("_Color");
        const auto incoming = block->GetColor(colorProperty);
        // Ignore neutral and unset overrides, including the guard's own writes.
        if ((incoming.r != incoming.g || incoming.g != incoming.b) &&
            (incoming.r != menuFloorBaseline.r || incoming.g != menuFloorBaseline.g ||
             incoming.b != menuFloorBaseline.b || incoming.a != menuFloorBaseline.a)) {
            block->SetColor(colorProperty, menuFloorBaseline);
            MenuFloorSetPropertyBlock(self, block);
            block->SetColor(colorProperty, incoming);
            if (!loggedFloorRestore) {
                loggedFloorRestore = true;
                INFO("[MenuFloorRestoration] intercepted BasicMenuGround _Color before submission RGBA=({},{},{},{})",
                     incoming.r, incoming.g, incoming.b, incoming.a);
            }
            return;
        }
    }
    MenuFloorSetPropertyBlock(self, block);
}

DECLARE_CLASS_CODEGEN(Halloween, FloorRendererSample, UnityEngine::MonoBehaviour) {
    DECLARE_INSTANCE_FIELD(float, nextLookupAt);
    DECLARE_INSTANCE_FIELD(UnityEngine::MaterialPropertyBlock*, propertyBlock);
    DECLARE_INSTANCE_METHOD(void, LateUpdate);
};
DEFINE_TYPE(Halloween, FloorRendererSample);

void Halloween::FloorRendererSample::LateUpdate() {
    if (UnityEngine::SceneManagement::SceneManager::GetActiveScene().name == "GameCore") {
        if (!floorRestorationActive) loggedFloorRestore = false;
        floorRestorationActive = true;
        return;
    }
    if (hasMenuFloorBaseline && (!floorRestorationActive || !getModConfig().EnableHauntedEnvColors.GetValue())) return;

    static const int colorProperty = Shader::PropertyToID("_Color");
    if (!menuFloor) {
        const float now = Time::get_realtimeSinceStartup();
        if (now < nextLookupAt) return;
        nextLookupAt = now + 0.5f;
        menuFloor = FindMenuFloor(colorProperty);
        if (!menuFloor) return;
    }

    auto* renderer = menuFloor.ptr();
    auto go = renderer->get_gameObject();
    if (!go || !go->get_activeInHierarchy() || !renderer->get_enabled()) return;
    renderer->GetPropertyBlock(propertyBlock);
    const auto current = propertyBlock->GetColor(colorProperty);

    if (!hasMenuFloorBaseline) {
        if (floorRestorationActive) return;
        // An unset override is reported as zero by the Quest binding.
        const bool unset = current.r == 0 && current.g == 0 && current.b == 0 && current.a == 0;
        menuFloorBaseline = unset ? renderer->get_sharedMaterials()[0]->GetColor(colorProperty) : current;
        hasMenuFloorBaseline = true;
        INFO("[MenuFloorRestoration] baselineRGBA=({},{},{},{})",
             menuFloorBaseline.r, menuFloorBaseline.g, menuFloorBaseline.b, menuFloorBaseline.a);
        return;
    }

    if (current.r == menuFloorBaseline.r && current.g == menuFloorBaseline.g &&
        current.b == menuFloorBaseline.b && current.a == menuFloorBaseline.a) return;
    // Preserve the other properties written by the menu lighting system.
    propertyBlock->SetColor(colorProperty, menuFloorBaseline);
    renderer->SetPropertyBlock(propertyBlock);
    if (!loggedFloorRestore) {
        loggedFloorRestore = true;
        INFO("[MenuFloorRestoration] LateUpdate corrected BasicMenuGround from RGBA=({},{},{},{})",
             current.r, current.g, current.b, current.a);
    }
}

namespace {
    void StartMenuFloorRestoration() {
        auto* probe = GameObject::New_ctor("HalloweenMenuFloorRestoration");
        auto* sampler = probe->AddComponent<Halloween::FloorRendererSample*>();
        if (!sampler) {
            GameObject::Destroy(probe);
            INFO("[MenuFloorRestoration] Failed to create floor controller");
            return;
        }
        sampler->nextLookupAt = Time::get_realtimeSinceStartup() + 0.5f;
        sampler->propertyBlock = MaterialPropertyBlock::New_ctor();
        GameObject::DontDestroyOnLoad(probe);
    }
}



static modloader::ModInfo modInfo{MOD_ID, VERSION, 0};
// Stores the ID and version of our mod, and is sent to
// the modloader upon startup

// static std::shared_ptr<cColor::ColorChanger> colorChanger;

//MAKE_HOOK_MATCH(SongPreviewPlayer_OnEnable, &GlobalNamespace::SongPreviewPlayer::OnEnable, void, GlobalNamespace::SongPreviewPlayer* self) {
//
//   MenuMusic::menuMusicLoader.set_OriginalClip(self->_defaultAudioClip);
//
//    if (!MenuMusic::menuMusicLoader.get_OriginalClip()) {
//        PaperLogger.info("Loading custom menu music...");
//        MenuMusic::menuMusicLoader.LoadCustomMusic();  // Keine Coroutine mehr nÃ¶tig
//    }
//
//    self->_defaultAudioClip = MenuMusic::menuMusicLoader.get_OriginalClip();
//
//    SongPreviewPlayer_OnEnable(self);
//}

// Called whenever the main menu scene loads
MAKE_HOOK_MATCH(SongPreviewPlayer_OnEnable, &SongPreviewPlayer::OnEnable, void, SongPreviewPlayer* self) {
    if(getModConfig().EnableMenuMusic.GetValue()) PlayRandomSong(self);
    SongPreviewPlayer_OnEnable(self);
}

// Called whenever the SongPreviewPlayer transitions from one song to another
MAKE_HOOK_MATCH(SongPreviewPlayer_CrossfadeTo, static_cast<void(SongPreviewPlayer::*)(AudioClip*, float, float, float, bool, System::Action*)>(&SongPreviewPlayer::CrossfadeTo), void, SongPreviewPlayer* self, AudioClip* audioClip, float musicVolume, float startTime, float duration, bool isDefault, System::Action* onFadeOutCallback) {
    // Choose a new random main menu song when transitioning away from the main menu music
    if(getModConfig().EnableMenuMusic.GetValue() && self->_defaultAudioClip && self->_defaultAudioClip.ptr() != audioClip) PlayRandomSong(self);
    SongPreviewPlayer_CrossfadeTo(self, audioClip, musicVolume, startTime, duration, isDefault, onFadeOutCallback);
}

// The results view owns the ghost lifecycle. RankModel::Rank is ordered
// E=0 D=1 C=2 B=3 A=4 S=5 SS=6 SSS=7 in the 1.40.8 codegen.
MAKE_HOOK_MATCH(HalloweenGhostResultsDidActivate, &GlobalNamespace::ResultsViewController::DidActivate, void,
    GlobalNamespace::ResultsViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    HalloweenGhostResultsDidActivate(self, firstActivation, addedToHierarchy, screenSystemEnabling);
    auto* completed = self ? self->__cordl_internal_get__levelCompletionResults() : nullptr;
    const bool rankSOrAbove = completed && completed->__cordl_internal_get_rank().value__ >= 5;
    Halloween::ghostDrift::SetResultsScreen(self, rankSOrAbove);
    // FireworkItemController::InitializeParticleSystem commonly runs before
    // the results screen opens. Starting only from that hook made A/S blank.
    // Start independently from the results-screen lifecycle instead.
    if (rankSOrAbove) {
        Halloween::ghostSting::EnterResults();
        if (!Halloween::ghostDrift::Begin()) {
            PaperLogger.info("Results ghost controller could not initialize");
        }
    }
}

MAKE_HOOK_MATCH(HalloweenGhostResultsDidDeactivate, &GlobalNamespace::ResultsViewController::DidDeactivate, void,
    GlobalNamespace::ResultsViewController* self, bool removedFromHierarchy, bool screenSystemDisabling) {
    Halloween::ghostSting::ExitResults();
    Halloween::ghostDrift::EndResultsScreen();
    HalloweenGhostResultsDidDeactivate(self, removedFromHierarchy, screenSystemDisabling);
}

// Results-screen celebration: replace firework particles and their explosion sound.
MAKE_HOOK_MATCH(FireworkGhostParticles, &GlobalNamespace::FireworkItemController::InitializeParticleSystem, void, GlobalNamespace::FireworkItemController* self) {
    FireworkGhostParticles(self);
    // The new controller lives under the results environment, with its own
    // Unity Update lifecycle. No reliance on the short firework burst duration.
    if (Halloween::ghostDrift::Begin()) {
        if (self && self->_particleSystems) {
            for (auto* entry : self->_particleSystems) {
                if (!entry || !entry->_particleSystem) continue;
                Flair::ParticleController::EmissionModule::set_enabled(
                    entry->_particleSystem.ptr(), false);
            }
        }
    } else if (Halloween::ghostDrift::IsEligible()) {
        // For rank S+ only: keep the old implementation as a fallback if
        // the dedicated ghost controller cannot start.
        Halloween::ghostCelebration::ApplyGhostParticles(self);
    }
}

MAKE_HOOK_MATCH(FireworkGhostAudio, &GlobalNamespace::FireworkItemController::PlayExplosionSound, void, GlobalNamespace::FireworkItemController* self) {
    // Dedicated controller uses a single arrival sting. Suppress repeated
    // firework booms and legacy ghost moans on rank S+ results.
    if (Halloween::ghostDrift::IsEligible()) return;
    FireworkGhostAudio(self);
}

// float BytesToFloat(uint8_t byte0, uint8_t byte1, uint8_t byte2, uint8_t byte3) {
//     float out = 0;
//     uint8_t* tempPtr = reinterpret_cast<uint8_t*>(&out);
    
//     // Arm processors are usually little endian, so assume this operation is okay
//     tempPtr[0] = byte0;
//     tempPtr[1] = byte1;
//     tempPtr[2] = byte2;
//     tempPtr[3] = byte3;

//     return out;
// }

//MAKE_HOOK_MATCH(GameServerLobbyFlowCoordinator_DidActivate, &GlobalNamespace::GameServerLobbyFlowCoordinator::DidActivate, void, GlobalNamespace::GameServerLobbyFlowCoordinator* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
//    MenuMusic::menuMusicLoader.set_OriginalClip(self->_ambienceAudioClip);
//    
//    self->_ambienceAudioClip = MenuMusic::menuMusicLoader.get_OriginalClip();
//    GameServerLobbyFlowCoordinator_DidActivate(self, firstActivation, addedToHierarchy, screenSystemEnabling);
//}

//MAKE_HOOK_MATCH(GameServerLobbyFlowCoordinator_DidDeactivate, &GlobalNamespace::GameServerLobbyFlowCoordinator::DidDeactivate, void, GlobalNamespace::GameServerLobbyFlowCoordinator* self, bool removedFromHierarchy, bool screenSystemDisabling) {
//   self->_ambienceAudioClip = MenuMusic::menuMusicLoader.get_OriginalClip();
//    GameServerLobbyFlowCoordinator_DidDeactivate(self, removedFromHierarchy, screenSystemDisabling);
//}

//MAKE_HOOK_MATCH(MultiplayerModeSelectionFlowCoordinator_DidActivate, &GlobalNamespace::MultiplayerModeSelectionFlowCoordinator::DidActivate, void, GlobalNamespace::MultiplayerModeSelectionFlowCoordinator* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
//    self->_ambienceAudioClip = MenuMusic::menuMusicLoader.get_OriginalClip();
//    MultiplayerModeSelectionFlowCoordinator_DidActivate(self, firstActivation, addedToHierarchy, screenSystemEnabling);
//}

//MAKE_HOOK_MATCH(MultiplayerModeSelectionFlowCoordinator_DidDeactivate, &GlobalNamespace::MultiplayerModeSelectionFlowCoordinator::DidDeactivate, void, GlobalNamespace::MultiplayerModeSelectionFlowCoordinator* self, bool removedFromHierarchy, bool screenSystemDisabling) {
//    self->_ambienceAudioClip = MenuMusic::menuMusicLoader.get_OriginalClip();
//    MultiplayerModeSelectionFlowCoordinator_DidDeactivate(self, removedFromHierarchy, screenSystemDisabling);
//}


//MAKE_HOOK_MATCH(NoteControllerInit, &GlobalNamespace::NoteController::Init, void, GlobalNamespace::NoteController *self, GlobalNamespace::NoteData *noteData, float worldRotation, UnityEngine::Vector3 moveStartPos, UnityEngine::Vector3 moveEndPos, UnityEngine::Vector3 jumpEndPos, float moveDuration, float jumpDuration, float jumpGravity) {
    //NoteControllerInit(self, noteData, worldRotation, moveStartPos, moveEndPos, jumpEndPos, moveDuration, jumpDuration, jumpGravity);

    //if (colorChanger) {
    //    colorChanger->UpdateColor(UnityEngine::Time::get_deltaTime());
    //}

    //auto renderers = self->GetComponentInChildren<UnityEngine::Renderer *>();
    //if (renderers) {
    //    for (auto m : renderers->get_materials()) {
    //        m->SetColor("_SimpleColor", colorChanger->currentColor);
     //       m->SetColor("_Color", colorChanger->currentColor);
        //}
    //}
//}


// MAKE_HOOK_MATCH(NoteControllerInit, &GlobalNamespace::NoteController::Init, void, GlobalNamespace::NoteController* self, GlobalNamespace::NoteData* noteData, float worldRotation, UnityEngine::Vector3 moveStartPos, UnityEngine::Vector3 moveEndPos, UnityEngine::Vector3 jumpEndPos, float moveDuration, float jumpDuration, float jumpGravity, float endRotation, float uniformScale, bool rotatesTowardsPlayer, bool useRandomRotation) {
//     NoteControllerInit(self, noteData, worldRotation, moveStartPos, moveEndPos, jumpEndPos, moveDuration, jumpDuration, jumpGravity, endRotation, uniformScale, rotatesTowardsPlayer, useRandomRotation);
//     if(!getModConfig().EnableBlocks.GetValue())
// 		return;

//     if (colorChanger) colorChanger->UpdateColor(UnityEngine::Time::get_deltaTime());


//     for(auto m : self->GetComponentInChildren<Renderer *>()->get_materials()) {
//         //m->SetColor("_SimpleColor", colorChanger->currentColor);
//         m->SetColor("_Color", colorChanger->currentColor);
//     }
// }

// Original Beat Saber neon sign remains untouched. No replacement-logo hook.

// Very quick and dirty way to override the menu light colors
MAKE_HOOK_MATCH(OverrideEnvironmentColors, &GlobalNamespace::LightWithIdManager::SetColorForId, void, GlobalNamespace::LightWithIdManager* self, int lightId, UnityEngine::Color color) {
    // Leave gameplay colors untouched. In the menu, tint environment lights
    // toward alternating Halloween orange and purple instead of forcing every
    // light to white (which erased the intended seasonal color treatment).
    if (!getModConfig().EnableHauntedEnvColors.GetValue() ||
        UnityEngine::SceneManagement::SceneManager::GetActiveScene().name == "GameCore") {
        OverrideEnvironmentColors(self, lightId, color);
        return;
    }

    const UnityEngine::Color halloweenOrange(1.0f, 0.22f, 0.025f, color.a);
    const UnityEngine::Color halloweenPurple(0.52f, 0.08f, 0.95f, color.a);
    const UnityEngine::Color tint = (lightId % 2 == 0) ? halloweenOrange : halloweenPurple;

    // Blend the original color with the seasonal tint to retain some variation.
    const float blend = 0.70f;
    const UnityEngine::Color seasonalColor(
        color.r * (1.0f - blend) + tint.r * blend,
        color.g * (1.0f - blend) + tint.g * blend,
        color.b * (1.0f - blend) + tint.b * blend,
        color.a
    );
    OverrideEnvironmentColors(self, lightId, seasonalColor);
}

namespace Halloween::logoColors { void ApplyLogoColors(); }

MAKE_HOOK_MATCH(AutumnLeavesInit, &GlobalNamespace::MainMenuViewController::DidActivate, void, GlobalNamespace::MainMenuViewController* self, bool firstActivation, bool addedToHierarchy, bool screenSystemEnabling) {
    AutumnLeavesInit(self, firstActivation, addedToHierarchy, screenSystemEnabling);

    if(!firstActivation) return;
    StartMenuFloorRestoration();

    Halloween::logoColors::ApplyLogoColors();

    
    if(getModConfig().EnableAutumnLeaves.GetValue()) Halloween::autumnLeaves::CreateAutumnLeaves();
    if(getModConfig().EnablePumpkinAccents.GetValue()) Halloween::pumpkinAccents::CreatePumpkinAccents();
}


// Called at the early stages of game loading
MOD_EXTERN_FUNC void setup(CModInfo *info) noexcept {
    *info = modInfo.to_c();

    getModConfig().Init(modInfo);

    // File logging
    Paper::Logger::RegisterFileContextId(PaperLogger.tag);

    INFO("Completed setup!");
}

// Called later on in the game loading - a good time to install function hooks
MOD_EXTERN_FUNC void late_load() noexcept {
    il2cpp_functions::Init();
    custom_types::Register::AutoRegister();


    INFO("Installing hooks...");
    //INSTALL_HOOK(PaperLogger, NoteControllerInit);
    // Keep the game's original Beat Saber logo; no logo replacement code is compiled.
    INSTALL_HOOK(PaperLogger, OverrideEnvironmentColors);
    INSTALL_HOOK(PaperLogger, MenuFloorSetPropertyBlock);
    INSTALL_HOOK(PaperLogger, AutumnLeavesInit);

    INSTALL_HOOK(PaperLogger, SongPreviewPlayer_OnEnable);
    INSTALL_HOOK(PaperLogger, SongPreviewPlayer_CrossfadeTo);
    INSTALL_HOOK(PaperLogger, HalloweenGhostResultsDidActivate);
    INSTALL_HOOK(PaperLogger, HalloweenGhostResultsDidDeactivate);
    INSTALL_HOOK(PaperLogger, FireworkGhostParticles);
    INSTALL_HOOK(PaperLogger, FireworkGhostAudio);
    //INSTALL_HOOK(PaperLogger, GameServerLobbyFlowCoordinator_DidActivate);
    //INSTALL_HOOK(PaperLogger, GameServerLobbyFlowCoordinator_DidDeactivate);
    //INSTALL_HOOK(PaperLogger, MultiplayerModeSelectionFlowCoordinator_DidActivate);
    //INSTALL_HOOK(PaperLogger, MultiplayerModeSelectionFlowCoordinator_DidDeactivate);



    INFO("Installed all hooks!");
}
