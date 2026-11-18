#include "Audio/ghostDriftController.hpp"

#include "assets.hpp"
#include "logging.hpp"
#include "bsml/shared/Helpers/utilities.hpp"
#include "GlobalNamespace/FireworkItemController.hpp"
#include "GlobalNamespace/ResultsViewController.hpp"
#include "UnityEngine/Camera.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Object.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "UnityEngine/Sprite.hpp"
#include "UnityEngine/SpriteRenderer.hpp"
#include "UnityEngine/Time.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Vector3.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <vector>

DEFINE_TYPE(Halloween, GhostDriftController);

namespace {
    using UnityEngine::GameObject;
    using UnityEngine::Sprite;
    using UnityEngine::SpriteRenderer;
    using UnityEngine::Vector3;

    struct ActiveGhost {
        GameObject* gameObject;
        SpriteRenderer* renderer;
        float age;
        float lifetime;
        float startX;
        float startY;
        float riseSpeed;
        float awaySpeed;
        float wavePhase;
        float waveSpeed;
        float waveWidth;
        float baseSize;
    };

    std::vector<ActiveGhost> active;
    SafePtrUnity<GameObject> root;
    SafePtrUnity<GlobalNamespace::ResultsViewController> resultsScreen;
    bool resultsVisible = false;
    bool rankEligible = false;

    void ClearGhosts() {
        // Destroy the renderers immediately from our tracking set; the GameObjects
        // are destroyed by Unity at the end of the frame.
        for (auto& ghost : active) {
            if (ghost.gameObject) UnityEngine::Object::Destroy(ghost.gameObject);
        }
        active.clear();
        if (root) UnityEngine::Object::Destroy(root.ptr());
        root = nullptr;
    }
    std::array<Sprite*, 4> sprites = {nullptr, nullptr, nullptr, nullptr};
    bool texturesAttempted = false;

    float Random01() {
        return static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
    }

    float Between(float min, float max) {
        return min + (max - min) * Random01();
    }

    bool LoadSprites() {
        if (texturesAttempted) {
            for (auto* sprite : sprites) if (!sprite) return false;
            return true;
        }
        texturesAttempted = true;
        sprites[0] = BSML::Utilities::LoadSpriteRaw(
            static_cast<ArrayW<uint8_t>>(IncludedAssets::ghost_single_0_png));
        sprites[1] = BSML::Utilities::LoadSpriteRaw(
            static_cast<ArrayW<uint8_t>>(IncludedAssets::ghost_single_1_png));
        sprites[2] = BSML::Utilities::LoadSpriteRaw(
            static_cast<ArrayW<uint8_t>>(IncludedAssets::ghost_single_2_png));
        sprites[3] = BSML::Utilities::LoadSpriteRaw(
            static_cast<ArrayW<uint8_t>>(IncludedAssets::ghost_single_3_png));
        for (auto* sprite : sprites) if (!sprite) return false;
        return true;
    }

    void SpawnGhost(int sequence) {
        if (!root || active.size() >= 6) return;

        auto* go = GameObject::New_ctor("HalloweenIndependentGhost");
        auto* tr = go->get_transform().ptr();
        tr->SetParent(root->get_transform(), false);
        auto* renderer = go->AddComponent<SpriteRenderer*>();
        if (!renderer) {
            UnityEngine::Object::Destroy(go);
            return;
        }

        renderer->set_sprite(sprites[std::rand() % sprites.size()]);
        // Alternate sides so the score stays unobstructed. Each sprite
        // is animated independently rather than moving as a firework burst.
        float side = (sequence % 2 == 0) ? -1.0f : 1.0f;
        float startX = side * Between(2.2f, 3.4f);
        float startY = Between(-1.6f, -0.7f);
        float scale = Between(1.15f, 1.65f);
        tr->set_localScale(Vector3(scale, scale, 1.0f));
        tr->set_localPosition(Vector3(startX, startY, 0.0f));
        tr->set_localRotation(UnityEngine::Quaternion::get_identity());
        renderer->set_color(UnityEngine::Color(0.90f, 0.95f, 1.0f, 0.0f));

        active.push_back({
            go, renderer, 0.0f,
            Between(7.0f, 9.5f), startX, startY,
            Between(0.30f, 0.46f), Between(0.16f, 0.28f),
            Between(0.0f, 6.28318f), Between(0.8f, 1.35f),
            Between(0.15f, 0.28f), scale
        });
    }
}

namespace Halloween::ghostDrift {
    void EndResultsScreen() {
        // Called for every exit, including Continue, Restart and navigation
        // away. Never allow ghosts to survive into the menu or next song.
        resultsVisible = false;
        rankEligible = false;
        resultsScreen = nullptr;
        ClearGhosts();
    }

    void SetResultsScreen(GlobalNamespace::ResultsViewController* results, bool eligible) {
        // The previous results may still have been in the Unity destroy queue.
        ClearGhosts();
        resultsScreen = results;
        resultsVisible = (results != nullptr);
        rankEligible = resultsVisible && eligible;
        if (rankEligible) {
            PaperLogger.info("Ghost results: rank A or above, enabled");
        } else {
            PaperLogger.info("Ghost results: below A or inactive, disabled");
        }
    }

    bool IsEligible() {
        return resultsVisible && rankEligible && resultsScreen;
    }

    bool Begin(GlobalNamespace::FireworkItemController* firework) {
        if (!IsEligible()) return false;
        if (root) return true;  // One controller for the current results screen.
        if (!firework || !LoadSprites()) {
            PaperLogger.info("Dedicated ghosts unavailable; use legacy particles");
            return false;
        }

        auto* camera = UnityEngine::Camera::get_main().ptr();
        if (!camera) {
            PaperLogger.info("No main camera for dedicated ghost placement");
            return false;
        }

        // Keep independent sprites in world space. Parenting them to the
        // results UI canvas can distort their size and motion. Their lifetime
        // is explicitly managed by ResultsViewController::DidDeactivate.
        auto* object = GameObject::New_ctor("HalloweenGhostResultsController");
        auto* camTr = camera->get_transform().ptr();
        const auto cam = camTr->get_position();
        const auto forward = camTr->get_forward();
        constexpr float distance = 6.5f;
        object->get_transform()->set_position(Vector3(
            cam.x + forward.x * distance,
            cam.y + forward.y * distance,
            cam.z + forward.z * distance
        ));
        object->get_transform()->set_rotation(camTr->get_rotation());
        root = object;

        auto* controller = object->AddComponent<GhostDriftController*>();
        if (!controller) {
            UnityEngine::Object::Destroy(object);
            root = nullptr;
            return false;
        }
        controller->elapsed = 0.0f;
        controller->untilSpawn = 0.0f;
        controller->sequence = 0;
        PaperLogger.info("Dedicated results ghost controller activated");
        return true;
    }
}

void Halloween::GhostDriftController::Update() {
    if (!Halloween::ghostDrift::IsEligible()) {
        ClearGhosts();
        return;
    }
    float dt = std::min(UnityEngine::Time::get_deltaTime(), 0.1f);
    if (dt <= 0.0f) return;

    elapsed += dt;
    untilSpawn -= dt;
    // Stagger births, not simultaneous firework bursts.
    if (untilSpawn <= 0.0f) {
        SpawnGhost(sequence++);
        untilSpawn = Between(0.85f, 1.55f);
    }

    for (std::size_t i = 0; i < active.size();) {
        auto& ghost = active[i];
        ghost.age += dt;
        if (ghost.age >= ghost.lifetime) {
            if (ghost.gameObject) UnityEngine::Object::Destroy(ghost.gameObject);
            active.erase(active.begin() + i);
            continue;
        }

        float t = ghost.age / ghost.lifetime;
        float fadeIn = std::min(1.0f, ghost.age / 0.9f);
        float fadeOut = std::min(1.0f, (ghost.lifetime - ghost.age) / 1.7f);
        float alpha = 0.69f * std::min(fadeIn, fadeOut);
        float sway = std::sin(ghost.wavePhase + ghost.age * ghost.waveSpeed)
                     * ghost.waveWidth;
        auto* tr = ghost.gameObject->get_transform().ptr();
        tr->set_localPosition(Vector3(
            ghost.startX + sway,
            ghost.startY + ghost.riseSpeed * ghost.age,
            ghost.awaySpeed * ghost.age
        ));
        // Subtle extra recession in size, beyond natural 3D perspective.
        float scale = ghost.baseSize * (1.0f - 0.17f * t);
        tr->set_localScale(Vector3(scale, scale, 1.0f));
        ghost.renderer->set_color(UnityEngine::Color(
            0.90f, 0.95f, 1.0f, alpha
        ));
        ++i;
    }
}

void Halloween::GhostDriftController::OnDestroy() {
    // Parent/results deactivation might already have cleared everything.
    // Only clear the active root: a later results screen could be alive.
    if (root && root.ptr() == this->get_gameObject().ptr()) {
        ClearGhosts();
    }
}
