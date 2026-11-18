#!/usr/bin/env python3
"""Download three real OpenGameArt ghost sounds and convert to release-ready OGG.

Sources/licenses: see FileCopies/GHOST_AUDIO_CREDITS.txt.
No synthesized replacements or additional pitch variants.
"""
from pathlib import Path
import subprocess
import tempfile
import urllib.request

SOUNDS = (
    ("ghost_ogrebane.ogg", "ghost.wav", "https://opengameart.org/sites/default/files/ghost.wav"),
    ("ghost_scary_1.ogg", "ghost_1.flac", "https://opengameart.org/sites/default/files/ghost_1.flac"),
    ("ghost_breath.ogg", "ghostbreath.flac", "https://opengameart.org/sites/default/files/ghostbreath.flac"),
)
OUT = Path("FileCopies")

def main():
    OUT.mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory() as folder:
        for target_name, source_name, url in SOUNDS:
            source_path = Path(folder) / source_name
            request = urllib.request.Request(url, headers={"User-Agent": "HalloweenThemeQMOD/1.40.8"})
            with urllib.request.urlopen(request, timeout=65) as response:
                source_path.write_bytes(response.read())
            if source_path.stat().st_size < 12000:
                raise RuntimeError(f"Unexpectedly small source audio {source_name}")
            output = OUT / target_name
            subprocess.run([
                "ffmpeg", "-nostdin", "-hide_banner", "-loglevel", "error",
                "-y", "-i", str(source_path), "-af", "volume=0.8",
                "-c:a", "libvorbis", "-q:a", "4", str(output)
            ], check=True)
            if output.stat().st_size < 3000:
                raise RuntimeError(f"OGG conversion failed: {target_name}")
            print(f"{source_name} -> {target_name} ({output.stat().st_size} bytes)")

if __name__ == "__main__":
    main()
