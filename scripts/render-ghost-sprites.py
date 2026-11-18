#!/usr/bin/env python3
"""Prepare exact original ghost artwork for the Beat Saber 1.40.8 controller.

The originals are supplied separately as four transparent PNGs. We intentionally
do not substitute the old stylized SVG stand-ins when originals are missing.
"""
from pathlib import Path
from PIL import Image, ImageFilter

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

# The source artwork includes a faint grid/checker imprint *inside* its
# opaque ghost body. Transparency is already correct outside the silhouettes,
# so altering alpha would be destructive. Instead gently smooth only the color
# texture, using alpha-normalized blur so the black transparent pixels do not
# bleed into the pale ghost edges. Keep all original PNGs untouched.
def soften_embedded_checker(image: Image.Image) -> Image.Image:
    rgba = image.convert("RGBA")
    r, g, b, a = rgba.split()
    blurred_alpha = a.filter(ImageFilter.GaussianBlur(1.15))
    softened = []
    for channel in (r, g, b):
        premul = Image.blend(Image.new("L", rgba.size, 0), channel, 1.0)
        premul.putdata([(value * opacity + 127) // 255
                        for value, opacity in zip(channel.getdata(), a.getdata())])
        filtered = premul.filter(ImageFilter.GaussianBlur(1.15))
        softened.append(Image.frombytes(
            "L", rgba.size,
            bytes(min(255, (int(v) * 255 + int(av) // 2) // max(1, int(av)))
                  for v, av in zip(filtered.getdata(), blurred_alpha.getdata()))
        ))
    return Image.merge("RGBA", (*softened, a))

cleaned = [soften_embedded_checker(img) for img in images]

# Dedicated controller: one cleaned original per sprite with its original
# alpha channel intact; no generative redraw, no silhouette replacement.
for i, img in enumerate(cleaned):
    img.save(assets / f"ghost_single_{i}.png", optimize=True)

# Retain the legacy 4-column atlas for fallback, without using the wrong 2x2
# atlas as a 4x1 particle sheet. Dedicated sprites use the images above.
atlas = Image.new("RGBA", (1024, 256), (0, 0, 0, 0))
for i, img in enumerate(cleaned):
    atlas.alpha_composite(img, (i * 256, 0))
atlas.save(assets / "ghost_sprites.png", optimize=True)
print("Prepared four original ghost silhouettes with softened baked-in checker texture")
