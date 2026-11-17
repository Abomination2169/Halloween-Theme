#!/usr/bin/env python3
"""Rasterize the four hand-drawn ghost tiles for the Quest particle renderer.

Run before CMake configuration; assets.cmake embeds all PNGs found in assets/.
Requires CairoSVG: python -m pip install cairosvg
"""
from pathlib import Path

import cairosvg

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
