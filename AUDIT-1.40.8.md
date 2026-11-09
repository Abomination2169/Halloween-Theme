# Halloween Theme 1.40.8 — Source Audit

Scope: current source branch only. The installed v1.0.0 QMOD, release assets, and `main` are not modified by this branch.

## Changes staged

- Verified the existing `include/c.hpp` already declares the four config values used by active hooks. Removed the accidentally introduced duplicate config declaration.
- Added `scripts/audit-source.py` and ran it as a preflight step in the QMOD build workflow.
- Kept package ID, mod ID, version, menu music file-copy path, dependencies, and seasonal assets unchanged.

## Still requiring verification

1. **Compile/package — PASSED:** GitHub Actions run [#42618041977](https://github.com/Abomination2169/Halloween-Theme/actions/runs/42618041977) completed successfully on commit `402867a03926c0c49884d03a5b9003a2e83e06be`. Source preflight, ARM64 compilation, QMOD packaging, archive validation, and artifact upload passed. Artifact: `Halloween-QMOD` (GitHub artifact ID `13673343212`). This confirms CI packaging, **not** headset behavior or binary identity with the published release.
2. **Runtime:** On a separate test installation, verify menu startup, transitions to gameplay, repeated return to menu, pumpkin and leaf cleanup, and music playback. Do not replace the working installed v1.0.0 package until testing succeeds.
3. **Settings UI:** `assets/settings.xml` contains legacy BSML bindings (`deactivateCustomMenuMusic`, `deactivateAutumnLeaves`, `deactivateHalloweenBlocks`), but no corresponding settings controller/registration exists in tracked source. This XML does not establish a working in-game menu. The existing config values persist defaults but are **not** exposed as toggles in this branch.
4. **Audio lifetime:** `src/Audio/player.cpp` captures a raw SongPreviewPlayer pointer in an async callback. Review lifecycle before changing behavior.
5. **Particle lifecycle:** `CreateAutumnLeaves` disables the original DustPS, but the tracked active hooks do not call `DestroyAutumnLeaves` on menu exit. Verify this behavior before changing it.
6. **Release reproducibility:** Compare the published v1.0.0 QMOD with its matching source tag/archive. A passing build from current main does not prove binary equivalence.

## Additional source-review observations (no runtime changes)

- **Menu lifecycle:** `AutumnLeavesInit` hooks `MainMenuViewController::DidActivate`, returns unless `firstActivation`, and creates leaves/pumpkins only on first activation. No matching `DidDeactivate` cleanup hook is installed. Verify behavior when leaving and returning to the menu before modifying this logic.
- **Audio lifecycle:** `PlayRandomSong` creates an asynchronous audio-load task and captures a raw `SongPreviewPlayer*` in its completion callback. A delayed completion could outlive the Unity object. This is a risk to investigate, **not** a confirmed Quest crash.
- **Settings:** `assets/settings.xml` is present but the source has no demonstrated BSML settings controller registration. Its three legacy toggle bindings do not match the four active `getModConfig()` values. Treat it as nonfunctional until implemented and tested.
- **Configuration:** `include/c.hpp` is the authoritative existing config declaration. Do not introduce a second `DECLARE_CONFIG(ModConfig)` header.

## Non-goals

- No 1.45.3 port.
- No modification to the Halloween v1.0.0 release or bsqmods listing.
- No new public release before CI and Quest 3 testing.
