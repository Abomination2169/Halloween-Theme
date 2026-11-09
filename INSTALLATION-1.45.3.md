# Halloween Theme 1.45.3 — controlled Quest 3 installation plan

**Status: NOT YET DEVICE-VERIFIED.** This is a pre-install checklist for a future, deliberate migration to Beat Saber `1.45.3_30054`. A green GitHub Actions build verifies compilation and package contents, **not** Quest menu behavior. Leave the development PR draft and release job disabled until on-device smoke tests pass.

## Stop conditions

- **Do not install this QMOD into Beat Saber 1.40.8**. Preserve the known-good Halloween v1.0.0 release and existing game/mod installation.
- Do not update the headset or downgrade a game install just to test this mod without preserving a supported rollback route, app data, and a compatible mod list.
- Do not install if the headset's Beat Saber version does not match `1.45.3_30054`, if QuestPatcher cannot recognize the install, or if core mods are unresolved.
- A GitHub artifact ZIP is **not** itself a QMOD. It contains `Halloween-Theme.qmod`, which is the file QuestPatcher imports.

## Before deciding to migrate

1. Record your current game version, mods, and settings; save backups with a suitable supported Quest backup workflow. Keep the original Halloween 1.40.8 QMOD and related working assets locally.
2. Verify a rollback path independently. Returning to an older Beat Saber APK can affect game data and installed mods; do not assume a simple downgrade is lossless.
3. On your Windows PC, connect the Quest 3 with USB debugging enabled. From your ADB folder run:
   ```powershell
   .\adb.exe devices
   .\adb.exe shell dumpsys package com.beatgames.beatsaber | Select-String 'versionName=|versionCode='
   ```
   Read the reported version; do **not** proceed on 1.40.8.
4. Download the **latest green, matching-commit** `Halloween-1453-QMOD` artifact from the draft PR's workflow. Extract the artifact ZIP to obtain `Halloween-Theme.qmod`.
5. Inspect the QMOD manifest if desired (`.qmod` is a ZIP archive). Verify `id=halloween`, `version=1.1.0`, `packageVersion=1.45.3_30054`, and an intact `libhalloween.so`, banner, and `Ghostly_Horror_Menu.ogg`. CI enforces these on build.
6. Using the current QuestPatcher for the **matching game version**, confirm availability of the five declared runtime dependencies: paper2_scotland2, beatsaber-hook, BSML, Custom Types, Flamingo. Do not mix 1.40.8 dependency QMODs into 1.45.3.

## Controlled test on an already-compatible 1.45.3 installation

1. Launch unmodded or baseline-modded Beat Saber **without Halloween enabled**; confirm the menu and one song work.
2. With the app closed, import `Halloween-Theme.qmod` via QuestPatcher's Mod Management. Confirm all dependencies are present; leave unrelated mods unchanged.
3. Capture startup logs **before launching the game**:
   ```powershell
   .\adb.exe logcat -v threadtime > ".\Halloween-1453-startup-log.txt"
   ```
   Start Beat Saber; after the smoke test, stop logging with Ctrl+C. Do not erase game data to investigate a failure.
4. Confirm startup reaches the menu without three white dots, crash, or hook installation errors. Check the original neon logo remains in place, pumpkin accents/tombstones appear once, orange/purple menu lighting is visible, and leaves **replace** the original dust.
5. Confirm the custom Halloween menu OGG plays as expected, and test switching songs and returning to menu. Watch for duplicate loads, silent audio, or repeated objects.
6. Launch a song and return to the menu several times; test gameplay lighting, menu recreation, and audio transitions. Confirm multiplayer and other non-menu scenes are not unintentionally tinted.
7. Verify settings creation at `/sdcard/ModData/com.beatgames.beatsaber/Mods/Halloween/settings.cfg`. With game fully closed, change one value to `false` and relaunch. Confirm the feature stays disabled. Restore the value and verify it returns. **Do not edit or overwrite any existing 1.40.8 files without a backup.**
8. Review logcat for `Halloween`, `halloween`, `flamingo`, `icall`, `SIGSEGV`, and managed exceptions. Capture logs and screenshots for discrepancies.

## If any smoke test fails

- Close Beat Saber and **disable only the Halloween 1.45.3 mod** in QuestPatcher; verify the base game still starts before changing any other mods.
- Preserve the logcat file and the QMOD's Actions run URL/commit SHA.
- Do **not** reset game data, uninstall Beat Saber, or attempt a blind downgrade to recover.
- Report the first crash signature and last successful menu interaction for a focused fix.

## Promotion gate

Mark draft PR #4 ready, merge, or publish a 1.1.0 release **only after** a matching green CI build, dependency checks, successful Quest 3 runtime smoke tests, settings persistence confirmation, a reviewed licensing/assets status, and a tested recovery plan. Until then, keep the current 1.40.8 production branch and release intact.
