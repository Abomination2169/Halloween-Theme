#!/usr/bin/env python3
"""Prepare exact original ghost artwork for the Beat Saber 1.40.8 controller.

The originals are supplied separately as four transparent PNGs. We intentionally
do not substitute the old stylized SVG stand-ins when originals are missing.
"""
from pathlib import Path
from PIL import Image

root = Path(__file__).resolve().parents[1]
assets = root / "assets"

sources = [assets / f"ghost_original_{index}.png" for index in range(1, 5)]
missing = [str(p) for p in sources if not p.is_file()]
if missing:
    raise SystemExit(
        "Missing user's original ghost artwork; cannot build this art branch:\n"
        + "\n".join(missing)
        + "\nUpload the four PNGs from the original files.zip into assets/ "
          "using names ghost_original_1.png ... ghost_original_4.png."
    )

images = []
for source in sources:
    with Image.open(source) as src:
        rgba = src.convert("RGBA")
        if rgba.size != (256, 256):
            raise SystemExit(f"{source.name}: expected original 256x256 PNG, got {rgba.size}")
        if rgba.getchannel("A").getextrema() != (0, 255):
            raise SystemExit(f"{source.name}: missing expected transparent alpha")
        images.append(rgba.copy())

# One exact image per SpriteRenderer. No resampling, repainting, atlas UV mixing,
# replacement shapes, or change to transparent wisps.
for i, img in enumerate(images):
    img.save(assets / f"ghost_single_{i}.png", optimize=True)

# Retain the legacy 4-column atlas for fallback, without using the wrong 2x2
# atlas as a 4x1 particle sheet. Dedicated sprites use the images above.
atlas = Image.new("RGBA", (1024, 256), (0, 0, 0, 0))
for i, img in enumerate(images):
    atlas.alpha_composite(img, (i * 256, 0))
atlas.save(assets / "ghost_sprites.png", optimize=True)
print("Prepared all 4 ORIGINAL wispy ghosts, unchanged, for dedicated controller")
