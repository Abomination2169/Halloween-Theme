# Halloween Theme 1.45.3 — pre-release and Quest 3 validation

**Status:** CI produced a QMOD for Beat Saber `1.45.3_30054` (run [42634367459](https://github.com/Abomination2169/Halloween-Theme/actions/runs/42634367459)). **This is a development artifact, not a tested release.**

## Preserve the known-good installation

- Do not overwrite the `main` branch, Halloween v1.0.0 release, or the existing 1.40.8 QMOD.
- Do not upgrade or reinstall Beat Saber on the Quest 3 solely to run this test. Wait until you choose to migrate and have verified a recovery path for the 1.40.8 setup.
- Save the 1.40.8 mod list, version, and relevant settings before changing versions. Back up game data using an appropriate supported method.
- Only use the 1.45.3 QMOD with **Beat Saber 1.45.3_30054**, not 1.40.8.

## Before installing a development build

1. Require a green CI run that validates the ELF architecture, dynamic dependencies, QMOD manifest, embedded artwork, and audio asset.
2. Review the **temporary** `scripts/diagnose-hook-constexpr.py` patch: it alters a downloaded beatsaber-hook header during CI. Replace with a supported upstream fix before publishing.
3. Replace the `include/c.hpp` defaults-only shim with a compatible settings implementation; it currently does **not** persist enable/disable choices.
4. Review all `i2c::resolve_icall(...).value()` particle-system calls against Unity's 1.45.3 runtime: a missing or changed internal call can fail at runtime despite successful compilation.
5. Review the `safe_ptr<GameObject*>` ownership and destruction paths for the pumpkins and leaves, including reentering the main menu.
6. Check that Flamingo is installed/resolved as a runtime dependency, not merely linked at build time.

## On-device smoke test — only when ready to migrate

- Confirm QuestPatcher recognizes the target game version and all required mods; avoid mixing 1.40.8 and 1.45.3 dependencies.
- Capture a clean ADB logcat session from before Beat Saber startup.
- Launch Beat Saber with the Halloween mod disabled first; confirm a working 1.45.3 baseline.
- Enable Halloween and verify it reaches the menu without a loading hang or crash.
- Verify original Beat Saber logo, pumpkin sprites, tombstones, falling leaves, orange/purple lighting, and Halloween menu audio.
- Switch into a song and back to the menu; confirm no duplicate objects, broken lighting, or audio corruption.
- Verify multiplayer/gameplay lighting is not unintentionally tinted.
- Repeat menu entry/exit and game restart to look for lifecycle issues.
- Check logs for missing Unity internal calls, Flamingo hook installation failures, GC/safe_ptr errors, and missing OGG assets.
- If anything fails, disable the mod and save the logs; do not reset or delete game data as a first troubleshooting step.

## Release gate

Do **not** merge draft PR #4 or tag/publish v1.1.0 until runtime behavior, settings, dependencies, licensing, and recovery steps are validated.
