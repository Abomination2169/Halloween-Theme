#include "Audio/ghostDriftController.hpp"
#include "Audio/ghostSting.hpp"

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
        float driftX;
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
    std::array<Sprite*, 12> sprites = {};
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
        sprites[4] = BSML::Utilities::LoadSpriteRaw(
            static_cast<ArrayW<uint8_t>>(IncludedAssets::ghost_new_0_png));
        sprites[5] = BSML::Utilities::LoadSpriteRaw(
            static_cast<ArrayW<uint8_t>>(IncludedAssets::ghost_new_1_png));
        sprites[6] = BSML::Utilities::LoadSpriteRaw(
            static_cast<ArrayW<uint8_t>>(IncludedAssets::ghost_new_2_png));
        sprites[7] = BSML::Utilities::LoadSpriteRaw(
            static_cast<ArrayW<uint8_t>>(IncludedAssets::ghost_new_3_png));
        sprites[8] = BSML::Utilities::LoadSpriteRaw(
            static_cast<ArrayW<uint8_t>>(IncludedAssets::ghost_new_4_png));
        sprites[9] = BSML::Utilities::LoadSpriteRaw(
            static_cast<ArrayW<uint8_t>>(IncludedAssets::ghost_new_5_png));
        sprites[10] = BSML::Utilities::LoadSpriteRaw(
            static_cast<ArrayW<uint8_t>>(IncludedAssets::ghost_new_6_png));
        sprites[11] = BSML::Utilities::LoadSpriteRaw(
            static_cast<ArrayW<uint8_t>>(IncludedAssets::ghost_new_7_png));
        for (auto* sprite : sprites) if (!sprite) return false;
        return true;
    }

    void SpawnGhost(int sequence) {
        if (!root || active.size() >= 8) return;

        auto* go = GameObject::New_ctor("HalloweenIndependentGhost");
        auto* tr = go->get_transform().ptr();
        tr->SetParent(root->get_transform(), false);
        auto* renderer = go->AddComponent<SpriteRenderer*>();
        if (!renderer) {
            UnityEngine::Object::Destroy(go);
            return;
        }

        // Original four and eight additional designs share normalized 256x256 RGBA sprites.
        // Every ghost picks one independently; never use a UV sprite-sheet animation.
        renderer->set_sprite(sprites[std::rand() % sprites.size()]);
        // Higher transparent sorting priority can help against ordinary scene
        // SpriteRenderers, but not necessarily screen-space UI canvases.
        renderer->set_sortingOrder(100);
        // Spawn on either side at randomized positions, away from the central
        // rank and score panel. Do not alternate rigidly between sides.
        // Avoid placing a new ghost directly over a recently spawned one.
        float startX = 0.0f;
        float startY = 0.0f;
        float side = -1.0f;
        bool foundClearSpawn = false;

        // The previous fifty-fifty random choice could place several ghosts
        // consecutively on the right. Balance the *current* side occupancy,
        // with a gentle left preference whenever counts are equal.
        int leftCount = 0;
        int rightCount = 0;
        for (const auto& earlier : active) {
            const float currentX = earlier.startX + earlier.driftX * earlier.age;
            if (currentX < 0.0f) ++leftCount;
            else ++rightCount;
        }
        const float preferredSide = leftCount < rightCount ? -1.0f
            : rightCount < leftCount ? 1.0f
            : (Random01() < 0.65f ? -1.0f : 1.0f);

        // Sample three horizontal lanes and different heights on both sides
        // to get a dispersed cloud without covering the score/rank panel.
        // Check both active positions and the positions they will reach shortly
        // after this ghost appears, rather than just their birth positions.
        for (int attempt = 0; attempt < 28; ++attempt) {
            side = (attempt < 17 || Random01() < 0.70f)
                ? preferredSide : -preferredSide;
            const float lane = Random01();
            const float radius = lane < 0.30f ? Between(2.05f, 2.75f)
                : lane < 0.72f ? Between(2.75f, 3.55f)
                               : Between(3.55f, 4.45f);
            startX = side * radius;
            startY = Between(-1.55f, 1.30f);
            bool overlapping = false;
            for (const auto& earlier : active) {
                const float nowX = earlier.startX + earlier.driftX * earlier.age
                    + std::sin(earlier.wavePhase + earlier.age * earlier.waveSpeed)
                      * earlier.waveWidth;
                const float nowY = earlier.startY + earlier.riseSpeed * earlier.age;
                const float dx = startX - nowX;
                const float dy = startY - nowY;
                if (dx * dx + dy * dy < 1.15f * 1.15f) {
                    overlapping = true;
                    break;
                }
                // Exclude forecast overlap during the first second as well.
                const float futureX = nowX + earlier.driftX * 1.0f;
                const float futureY = nowY + earlier.riseSpeed * 1.0f;
                const float fx = startX - futureX;
                const float fy = startY + 0.35f - futureY;
                if (fx * fx + fy * fy < 1.0f * 1.0f) {
                    overlapping = true;
                    break;
                }
            }
            if (!overlapping) {
                foundClearSpawn = true;
                break;
            }
        }
        if (!foundClearSpawn) {
            UnityEngine::Object::Destroy(go);
            return; // Prefer skipping a ghost over overlapping five others.
        }
        // Independent constant drift can travel in either horizontal direction.
        // Sine-wave motion below adds subtle, non-linear floating on top.
        // Bias roughly two-thirds of ghosts outward to reduce their path
        // through the pumpkin/sign area, while retaining inward movement.
        const float travelSide = Random01() < 0.68f ? side : -side;
        const float driftX = travelSide * Between(0.045f, 0.15f);
        // The originals rendered like huge cut-outs at 1.15-1.65 scale.
        // Reduce to ~30% of their former apparent size.
        float scale = Between(0.36f, 0.49f);
        tr->set_localScale(Vector3(scale, scale, 1.0f));
        tr->set_localPosition(Vector3(startX, startY, 0.0f));
        tr->set_localRotation(UnityEngine::Quaternion::get_identity());
        renderer->set_color(UnityEngine::Color(0.90f, 0.95f, 1.0f, 0.0f));

        Halloween::ghostSting::PlaySpawnVoice(go);
        active.push_back({
            go, renderer, 0.0f,
            Between(7.0f, 9.5f), startX, startY, driftX,
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

    bool Begin() {
        if (!IsEligible()) return false;
        if (root) return true;  // One controller for the current results screen.
        if (!LoadSprites()) {
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
        // Put ghosts slightly in front of the pumpkin-sign plane in camera
        // space, reducing depth occlusion without changing the pumpkins.
        constexpr float distance = 5.3f;
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
        untilSpawn = Between(0.60f, 1.10f);
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
        float alpha = 0.48f * std::min(fadeIn, fadeOut);
        float sway = std::sin(ghost.wavePhase + ghost.age * ghost.waveSpeed)
                     * ghost.waveWidth;
        auto* tr = ghost.gameObject->get_transform().ptr();
        tr->set_localPosition(Vector3(
            ghost.startX + ghost.driftX * ghost.age + sway,
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
