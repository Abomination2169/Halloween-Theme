# Halloween Theme 1.40.8 — Source Audit

Scope: current source branch only. The installed v1.0.0 QMOD, release assets, and `main` are not modified by this branch.

## Changes staged

- Added `include/modconfig.hpp` using the documented config-utils `DECLARE_CONFIG` API to declare the four values consumed by active hooks.
- Added `scripts/audit-source.py` and ran it as a preflight step in the QMOD build workflow.
- Kept package ID, mod ID, version, menu music file-copy path, dependencies, and seasonal assets unchanged.

## Still requiring verification

1. **Compile/package:** Run `Build Halloween QMOD` on this branch, then inspect the workflow log and artifact. Static preflight is not a substitute for an Android ARM64 build.
2. **Runtime:** On a separate test installation, verify menu startup, transitions to gameplay, repeated return to menu, pumpkin and leaf cleanup, and music playback. Do not replace the working installed v1.0.0 package until testing succeeds.
3. **Settings UI:** `assets/settings.xml` contains legacy BSML bindings (`deactivateCustomMenuMusic`, `deactivateAutumnLeaves`, `deactivateHalloweenBlocks`), but no corresponding settings controller/registration exists in tracked source. This XML does not establish a working in-game menu. The restored config values persist defaults but are **not** exposed as toggles in this branch.
4. **Audio lifetime:** `src/Audio/player.cpp` captures a raw SongPreviewPlayer pointer in an async callback. Review lifecycle before changing behavior.
5. **Particle lifecycle:** `CreateAutumnLeaves` disables the original DustPS, but the tracked active hooks do not call `DestroyAutumnLeaves` on menu exit. Verify this behavior before changing it.
6. **Release reproducibility:** Compare the published v1.0.0 QMOD with its matching source tag/archive. A passing build from current main does not prove binary equivalence.

## Non-goals

- No 1.45.3 port.
- No modification to the Halloween v1.0.0 release or bsqmods listing.
- No new public release before CI and Quest 3 testing.
