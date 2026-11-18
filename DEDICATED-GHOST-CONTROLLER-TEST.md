# Dedicated Ghost Results Controller — Quest 1.40.8 prototype

This branch was cut from the existing v2.5 test commit `ad947e3`. The original `feature/ghost-results-1.40.8` branch and stable `main` are unchanged.

## Design

- The existing Beat Saber `FireworkItemController::InitializeParticleSystem` hook is retained as a **results celebration trigger only**.
- A `GhostDriftController` MonoBehaviour owns individual ghost GameObjects / SpriteRenderers.
- Four distinct, alpha-transparent sprites are generated from `assets/ghost_sprites.svg` during the build.
- Each ghost spawns at staggered 0.85–1.55 second intervals, alternating left/right, with jitter to avoid stacking.
- Each ghost lives 7–9.5 seconds, slowly rises, sways, moves farther away in 3D, fades in for ~0.9 s and fades out over ~1.7 s.
- Maximum six visible ghosts; no per-particle fireworks lifespan, burst gravity, or short-lived alpha animation.
- Controller is parented under the firework parent so it is cleaned up with the results-scene hierarchy.
- If the Unity camera / sprite loading fails, it falls back to the older particle implementation rather than losing effects entirely.
- Public QMODs do not include separate Pixabay audio, and this prototype makes no audio changes.

## Testing

1. Continue testing v2.5 from the original branch first.
2. Only after CI builds the new dedicated branch, install its `Halloween-QMOD` on Quest 3 / Beat Saber 1.40.8.
3. Verify that ghosts do not vanish after a second, are independently spaced, and move farther into the scene.
4. Check whether the independent sprites align with the results screen and disappear after leaving the screen.
5. Check for stutter or crashes and whether the mod's original menu graphics are intact.

This is not considered a confirmed headset-ready implementation until visual testing verifies the parent lifetime, render depth, camera positioning, fade, and event timing.
