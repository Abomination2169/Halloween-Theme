#!/usr/bin/env python3
"""Produce five standalone transparent PNGs from the original 1.40.8 leaf atlas.

Run before CMake config so assets.cmake embeds the five individual PNG files.
Pillow is a build-only dependency; no additional Quest runtime dependency.
"""
from pathlib import Path
from PIL import Image

source = Path("assets/autumn_leaves.png")
with Image.open(source) as image:
    image = image.convert("RGBA")
    width, height = image.size
    if width < 5 or height <= 0:
        raise SystemExit("Invalid Halloween leaf atlas dimensions")
    for i in range(5):
        left = width * i // 5
        right = width * (i + 1) // 5
        output = source.parent / f"autumn_leaf_{i}.png"
        tile = image.crop((left, 0, right, height))
        if tile.getchannel("A").getextrema()[0] == 255:
            raise SystemExit(f"{output} has no transparent pixels; aborting")
        tile.save(output, "PNG")
        print(f"Generated {output}: {tile.width}x{tile.height}, RGBA")
