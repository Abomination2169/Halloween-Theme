#!/usr/bin/env python3
"""Rasterize the four hand-drawn ghost tiles for the Quest particle renderer.

Run before CMake configuration; assets.cmake embeds all PNGs found in assets/.
Requires CairoSVG: python -m pip install cairosvg
"""
from pathlib import Path

import cairosvg
from PIL import Image

root = Path(__file__).resolve().parents[1]
svg = root / "assets" / "ghost_sprites.svg"
png = root / "assets" / "ghost_sprites.png"
cairosvg.svg2png(
    url=str(svg),
    write_to=str(png),
    output_width=1024,
    output_height=256,
)
data = png.read_bytes()
if not data.startswith(b"\x89PNG\r\n\x1a\n"):
    raise SystemExit("Ghost sprite atlas was not generated as PNG")
print(f"Generated 4 transparent ghost tiles: {png} ({len(data)} bytes)")

# Dedicated ghost component loads one complete Sprite per GameObject. Crop
# four separate transparent tiles so it never inherits a particle-sheet UV
# animation or samples multiple ghosts at once.
with Image.open(png).convert("RGBA") as atlas:
    tile_width = atlas.width // 4
    for index in range(4):
        bounds = (index * tile_width, 0, (index + 1) * tile_width, atlas.height)
        atlas.crop(bounds).save(root / "assets" / f"ghost_single_{index}.png", optimize=True)
print("Generated separate alpha sprites for dedicated ghost controller")
