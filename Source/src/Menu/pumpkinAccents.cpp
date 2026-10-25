#include "Menu/pumpkinAccents.hpp"
#include "bsml/shared/BSML.hpp"
#include "main.hpp"
#include "assets.hpp"
#include "bsml/shared/Helpers/utilities.hpp"
#include "UnityEngine/SpriteRenderer.hpp"

#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/Quaternion.hpp"
#include "UnityEngine/Color.hpp"
#include "UnityEngine/PrimitiveType.hpp"
#include "UnityEngine/Collider.hpp"
#include "UnityEngine/MeshRenderer.hpp"
#include "UnityEngine/Material.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/Object.hpp"

using namespace UnityEngine;

// Keep the original namespace and entry points used by main.cpp.
namespace Halloween::pumpkinAccents {
    SafePtrUnity<GameObject> pumpkinAccents;

    struct GraveSpot {
        Vector3 originalPosition;
        float scale;
        float angle;
    };

    static GameObject* MakePart(Transform* parent, PrimitiveType type,
                                const char* name, Vector3 position,
                                Vector3 scale, Material* material) {
        GameObject* part = GameObject::CreatePrimitive(type);
        part->set_name(name);
        part->get_transform()->SetParent(parent, false);
        part->get_transform()->set_localPosition(position);
        part->get_transform()->set_localScale(scale);
        auto* renderer = part->GetComponent<MeshRenderer*>();
        if (renderer && material) renderer->set_sharedMaterial(material);
        auto* collider = part->GetComponent<Collider*>();
        if (collider) Object::Destroy(collider);
        return part;
    }

    void CreatePumpkinAccents() {
        if (pumpkinAccents) return;
        GameObject* environment = GameObject::Find("DefaultMenuEnvironment");
        if (!environment) return;

        GameObject* note = GameObject::Find("Note");
        Material* baseMaterial = nullptr;
        if (note) {
            auto* renderer = note->GetComponent<MeshRenderer*>();
            if (renderer) baseMaterial = renderer->get_sharedMaterial();
        }
        if (!baseMaterial) {
            auto shader = Shader::Find("Standard");
            if (!shader) shader = Shader::Find("Sprites/Default");
            if (!shader) return;
            baseMaterial = Material::New_ctor(shader);
        }

        pumpkinAccents = GameObject::New_ctor("HalloweenGraveyardAccents");
        pumpkinAccents->get_transform()->SetParent(environment->get_transform(), false);

        // Two independent pumpkin canvases, following the earlier
       // Halloween mod positioning pattern.
       // Preserve the original Beat Saber neon logo and flickering E.
       auto* pumpkinSignPlane = GameObject::New_ctor("HalloweenPumpkinSignPlane");
       pumpkinSignPlane->get_transform()->SetParent(pumpkinAccents->get_transform(), false);
       pumpkinSignPlane->get_transform()->set_localPosition(Vector3(0.0f, 16.6f, 26.1f));

       auto makePumpkin = [&](const char* name, ArrayW<uint8_t> png, float side) {
           auto* canvas = BSML::Lite::CreateCanvas();
           canvas->set_name(name);
           canvas->get_transform()->SetParent(pumpkinSignPlane->get_transform(), false);
           canvas->get_transform()->set_localPosition(Vector3(side * 10.0f, 0.4f, 0.0f));

           BSML::Lite::CreateImage(
               canvas,
               BSML::Utilities::LoadSpriteRaw(png),
               UnityEngine::Vector2(0.0f, 0.0f),
               UnityEngine::Vector2(260.0f, 260.0f)
           );
       };

       makePumpkin(
           "HalloweenPumpkinLeft",
           static_cast<ArrayW<uint8_t>>(IncludedAssets::pumpkin_left_png),
           -1.0f
       );

       makePumpkin(
           "HalloweenPumpkinRight",
           static_cast<ArrayW<uint8_t>>(IncludedAssets::pumpkin_right_png),
           1.0f
       );
       auto* stone = Object::Instantiate(baseMaterial);
        auto* darkerStone = Object::Instantiate(baseMaterial);
        auto* engraving = Object::Instantiate(baseMaterial);
        auto* moss = Object::Instantiate(baseMaterial);
        stone->SetColor("_Color", Color(0.43f, 0.46f, 0.48f, 1.0f));
        darkerStone->SetColor("_Color", Color(0.29f, 0.32f, 0.35f, 1.0f));
        engraving->SetColor("_Color", Color(0.12f, 0.13f, 0.15f, 1.0f));
        moss->SetColor("_Color", Color(0.18f, 0.26f, 0.14f, 1.0f));

        // Original twenty locations, including the original division by two.
        const GraveSpot spots[] = {
            {Vector3(-5.5f, -1.3f, 16.1f), 1.00f, 12.0f},
            {Vector3(16.4f, -1.3f, 0.0f), 0.88f, -25.0f},
            {Vector3(6.5f, -1.3f, 16.4f), 1.12f, 31.0f},
            {Vector3(15.9f, -1.0f, 10.8f), 0.96f, -12.0f},
            {Vector3(-18.3f, -0.8f, -11.4f), 1.18f, 38.0f},
            {Vector3(-9.0f, -1.7f, -7.7f), 0.85f, -36.0f},
            {Vector3(-18.1f, -1.3f, 2.7f), 1.07f, 18.0f},
            {Vector3(-2.5f, -1.3f, -14.2f), 0.93f, -16.0f},
            {Vector3(-15.7f, -1.3f, 12.0f), 1.13f, 27.0f},
            {Vector3(10.0f, -1.3f, -10.5f), 0.91f, -28.0f},
            {Vector3(-15.1f, -1.3f, 26.5f), 1.28f, 42.0f},
            {Vector3(25.4f, -0.9f, -15.5f), 1.36f, -43.0f},
            {Vector3(7.0f, -1.3f, 29.5f), 1.22f, 11.0f},
            {Vector3(29.9f, -1.3f, 19.2f), 1.40f, -32.0f},
            {Vector3(34.4f, -1.3f, 0.5f), 1.25f, 22.0f},
            {Vector3(-34.3f, -0.9f, 16.6f), 1.32f, -19.0f},
            {Vector3(-36.0f, -1.3f, -6.0f), 1.17f, 33.0f},
            {Vector3(-15.2f, -1.3f, -25.3f), 1.38f, -10.0f},
            {Vector3(7.9f, -1.3f, -30.2f), 1.30f, 45.0f},
            {Vector3(-25.8f, -0.7f, 7.0f), 1.04f, -38.0f}
        };

        for (int i = 0; i < 20; ++i) {
            const auto& spot = spots[i];
            GameObject* grave = GameObject::New_ctor("HalloweenTombstone");
            grave->get_transform()->SetParent(pumpkinAccents->get_transform(), false);
            grave->get_transform()->set_localPosition(
                Vector3(spot.originalPosition.x / 2.0f,
                        spot.originalPosition.y / 2.0f,
                        spot.originalPosition.z / 2.0f));
            grave->get_transform()->set_localRotation(
                Quaternion::Euler(Vector3(0.0f, spot.angle, (i % 5 - 2) * 2.0f)));
            grave->get_transform()->set_localScale(Vector3(spot.scale, spot.scale, spot.scale));
            Transform* root = grave->get_transform();

            // A sturdy upright gravestone, partly sunk into the ground.
            MakePart(root, PrimitiveType::Cube, "StoneBase",
                     Vector3(0.0f, 0.10f, 0.0f), Vector3(1.15f, 0.23f, 0.43f), darkerStone);
            MakePart(root, PrimitiveType::Cube, "UprightStone",
                     Vector3(0.0f, 0.89f, 0.0f), Vector3(0.91f, 1.52f, 0.30f), stone);

            // Rounded-top stones alternate with square/stepped gothic stones.
            if (i % 3 != 0) {
                MakePart(root, PrimitiveType::Sphere, "RoundedTop",
                         Vector3(0.0f, 1.65f, 0.0f), Vector3(0.91f, 0.57f, 0.30f), stone);
            } else {
                MakePart(root, PrimitiveType::Cube, "GothicTop",
                         Vector3(0.0f, 1.70f, 0.0f), Vector3(0.63f, 0.27f, 0.33f), darkerStone);
                MakePart(root, PrimitiveType::Cube, "GothicCrown",
                         Vector3(0.0f, 1.90f, 0.0f), Vector3(0.30f, 0.20f, 0.33f), stone);
            }

            // Raised dark cross, placed slightly in front of the face (-Z).
            MakePart(root, PrimitiveType::Cube, "EngravedCrossVertical",
                     Vector3(0.0f, 1.13f, -0.163f), Vector3(0.10f, 0.50f, 0.035f), engraving);
            MakePart(root, PrimitiveType::Cube, "EngravedCrossHorizontal",
                     Vector3(0.0f, 1.24f, -0.166f), Vector3(0.36f, 0.09f, 0.035f), engraving);
            // Thin moss at the foot for a weathered look.
            MakePart(root, PrimitiveType::Cube, "MossAtFoot",
                     Vector3(-0.22f, 0.24f, -0.163f), Vector3(0.29f, 0.12f, 0.040f), moss);
        }
    }

    void DestroyPumpkinAccents() {
        if (pumpkinAccents) {
            GameObject::Destroy(pumpkinAccents.ptr());
            pumpkinAccents = nullptr;
        }
    }
}







