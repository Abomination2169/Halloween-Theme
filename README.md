## 🎃 Halloween Theme — Video

Watch Halloween Theme in action!

[![Halloween Theme - Beat Saber](Halloween-Banner.png)](https://www.youtube.com/watch?v=LgSdA3DGreM)

▶️ **Watch on YouTube:** https://www.youtube.com/watch?v=LgSdA3DGreM

Happy Halloween!

Transform your Beat Saber menu into a spooky graveyard with falling autumn leaves.
Based on the fantastic Christmas Mod by vcmikuu laynia. Free music provided by the brilliant Alex Morgan.

Note: Development assisted with use of A.I. Please support talented artists and coders whenever possible.

**Compatible with:** Beat Saber 1.40.8 (Quest)

## 🎃 Halloween Theme — Beat Saber 1.45.3 development port

This branch is the separate Beat Saber **1.45.3** development port. The stable `main` branch and v1.0.0 release remain on Beat Saber 1.40.8.

The port preserves the existing Halloween environment, autumn leaves, pumpkin artwork, and Ghostly Horror menu music. It updates QPM dependencies, Android NDK, and the QMOD target to the 1.45.3 Quest build (`1.45.3_30054`).

**Status:** Development build only. CI compilation and package validation are not a substitute for headset testing. Do not install this build over your stable 1.40.8 setup.

## Building

Requirements:
- QPM CLI
- Android NDK 29.0.14206865
- CMake 3.22+
- Ninja
- Python 3

```sh
qpm ndk resolve -d
qpm restore
qpm qmod manifest
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build --parallel
qpm qmod zip --import mod.json --skip_build
```

The generated package targets Beat Saber `1.45.3_30054`. The stable 1.40.8 release is unchanged.
