# Halloween Theme — Beat Saber 1.45.3 Quest 3 port

Status: **port preparation; no 1.45.3 QMOD has been compiled or tested**.

## Preservation rules

- Production `main` and the verified Beat Saber `1.40.8_7379` v1.0.0 release remain untouched.
- All port work is isolated on `port/halloween-1.45.3` and a draft pull request.
- Do not install a port build on the working 1.40.8 headset.
- Preserve the Halloween menu track, pumpkin PNGs, gravestones, leaf sprite sheet, sign, lighting, and placement.

## Verified target and ecosystem references

- QuestPackageManager/bsqmods has a `mods/1.45.3_30054` directory, identifying the target Quest package build.
- Its 1.45.3 listings include `bsml` 0.6.6, `custom-types` 0.21.2, and `paper2_scotland2` 4.8.0.
- The 1.45.3 BSML and custom-types listings depend on `beatsaber-hook` ^9.3.0. This is a major change from our 1.40.8 dependency ^6.4.2.
- These are **distribution-listing versions**, not proof that the Halloween C++ APIs or a matching QPM header set compile.

Sources:
- https://github.com/QuestPackageManager/bsqmods/tree/main/mods/1.45.3_30054
- https://github.com/QuestPackageManager/bsqmods/blob/main/mods/1.45.3_30054/bsml-0.6.6.json
- https://github.com/QuestPackageManager/bsqmods/blob/main/mods/1.45.3_30054/custom-types-0.21.2.json

## Porting sequence

1. Identify and pin a **1.45.3-compatible bs-cordl/game bindings** package, plus matching QPM versions for Scotland2, config-utils, beatsaber-hook, BSML, custom-types, and Paperlog. Do not guess a bs-cordl version.
2. Update `qpm.json` and any generated/locked QPM metadata on this branch only.
3. Change the port's `mod.template.json` `packageVersion` to `1.45.3_30054`; assign a distinct port version before packaging. Keep `id: halloween` and `libhalloween.so`.
4. Update build validation to check the new target without weakening the existing 1.40.8 checks on `main`.
5. Compile and fix C++ API differences, especially `SongPreviewPlayer`, `MediaAsyncLoader`, `MainMenuViewController`, `LightWithIdManager`, `BSML::Lite`, and particle APIs.
6. Verify QMOD archive manifest, assets, fileCopies, and library.
7. Test on a **separate 1.45.3 Quest setup** only after CI passes. Compare music, menu geometry, lights, leaf colors, sign, transitions, and repeated menu visits.
8. Keep the port PR in draft until the user authorizes release. Do not merge the 1.45.3 port into production `main` by default because that would replace the 1.40.8 source target.

## Current blockers

- A verified compatible 1.45.3 generated-header/bs-cordl package and matching build toolchain have not yet been established.
- No ARM64 compile or runtime verification has been completed for 1.45.3.

This file is preparatory documentation and does not change runtime behavior.
