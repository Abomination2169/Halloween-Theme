#!/usr/bin/env python3
"""Prepare CC0 Ogrebane ghost SFX variants for the Halloween 1.40.8 QMOD.

SOURCE: Ogrebane, "Ghost", OpenGameArt (17 December 2010), CC0.
https://opengameart.org/content/ghost
https://opengameart.org/sites/default/files/ghost.wav

This script downloads the REAL recorded Ghost sound, rather than synthesizing
cartoon voices. It produces three subtly different versions to vary ghost
spawn playback. Source and license are documented in ORIGINAL-GHOST-SFX-LICENSE.md.
"""
from pathlib import Path
import subprocess
import urllib.request
import wave
import tempfile

URL = "https://opengameart.org/sites/default/files/ghost.wav"
OUT = Path("FileCopies")
VARIATIONS = (
    ("ghost_cc0_ogrebane_original.ogg", 1.0, 0),
    ("ghost_cc0_ogrebane_lower.ogg", 0.91, 65),
    ("ghost_cc0_ogrebane_higher.ogg", 1.09, 110),
)

def main():
    OUT.mkdir(exist_ok=True)
    with tempfile.TemporaryDirectory() as folder:
        original = Path(folder) / "ogrebane_ghost.wav"
        req = urllib.request.Request(URL, headers={"User-Agent": "HalloweenThemeQMOD/1.40.8 (CC0 asset download)"})
        with urllib.request.urlopen(req, timeout=40) as response:
            original.write_bytes(response.read())
        if original.stat().st_size < 20000:
            raise RuntimeError("Downloaded ghost.wav is unexpectedly small")
        with wave.open(str(original), "rb") as wav:
            sr = wav.getframerate()
            frames = wav.getnframes()
        if sr < 8000 or frames < sr // 2:
            raise RuntimeError("Unexpected source ghost WAV format")

        for filename, pitch, echo_ms in VARIATIONS:
            output = OUT / filename
            filters = [f"asetrate={int(sr * pitch)}", f"aresample={sr}"]
            if echo_ms:
                filters.append(f"aecho=0.8:0.15:{echo_ms}:0.13")
            filters.extend(["afade=t=in:st=0:d=0.09", "volume=0.8"])
            subprocess.run([
                "ffmpeg", "-nostdin", "-hide_banner", "-loglevel", "error",
                "-y", "-i", str(original), "-af", ",".join(filters),
                "-c:a", "libvorbis", "-q:a", "4", str(output)
            ], check=True)
            if output.stat().st_size < 4000:
                raise RuntimeError("Failed to generate: " + str(output))
            print(output, output.stat().st_size)
    print("CC0 source: Ogrebane / OpenGameArt / https://opengameart.org/content/ghost")

if __name__ == "__main__":
    main()
