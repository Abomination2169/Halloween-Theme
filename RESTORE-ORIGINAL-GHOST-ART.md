# Restore the original illustrated ghosts

This branch replaces the **temporary SVG ghost artwork** with the user's exact four transparent 256×256 PNG designs from `files.zip`. The dedicated controller's lifetime, fade, movement and positioning code remain unchanged.

## Upload the four original files

1. Unzip the supplied `original_ghost_artwork_for_github.zip`. It contains an `assets` folder with exactly four transparent PNG files.
2. Open the [assets folder on this branch](https://github.com/Abomination2169/Halloween-Theme/tree/feature/original-artwork-ghost-controller-1.40.8/assets).
3. Choose **Add file → Upload files** and upload the four PNG files (the files *inside* the downloaded `assets` folder) into this GitHub folder.
4. Commit directly to `feature/original-artwork-ghost-controller-1.40.8`, **not main**.

The files must be named:
- `assets/ghost_original_1.png`
- `assets/ghost_original_2.png`
- `assets/ghost_original_3.png`
- `assets/ghost_original_4.png`

## Build process

The `scripts/render-ghost-sprites.py` script checks for all four originals and builds `ghost_single_0.png` through `ghost_single_3.png` from the original pixels **without resampling or redrawing**. It creates a separate 4×1 atlas as a fallback for legacy particle code, but the dedicated controller uses each individual texture independently.

The branch intentionally **fails its build if the exact original art is absent**. This prevents another QMOD silently falling back to the SVG sketches. Once the four PNGs are committed, GitHub Actions should build a new `Halloween-QMOD` artifact for headset testing.

The known-working v2.5 test branch and the separate dedicated controller build remain available. No published Halloween release is modified.
