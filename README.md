## 🎃 Halloween Theme — Video

Watch Halloween Theme in action!

[![Halloween Theme - Beat Saber](Halloween-Banner.png)](https://www.youtube.com/watch?v=LgSdA3DGreM)

▶️ **Watch on YouTube:** https://www.youtube.com/watch?v=LgSdA3DGreM

Happy Halloween!

Transform your Beat Saber menu into a spooky graveyard with falling autumn leaves.
Based on the fantastic Christmas Mod by vcmikuu laynia. Free music provided by the brilliant Alex Morgan.

Note: Development assisted with use of A.I. Please support talented artists and coders whenever possible.

**Compatible with:** Beat Saber 1.40.8 (Quest)

## Building from source

The supported Quest baseline is Beat Saber **1.40.8**.

Requirements:
- QPM CLI (GitHub Actions pins QPM v1.5.12)
- Android NDK 27.3.13750724
- CMake 3.22 or newer
- Ninja
- Python 3

From the repository root:

```sh
qpm ndk resolve -d
qpm restore
qpm qmod manifest
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build --parallel
qpm qmod zip --import mod.json --skip_build
```

The generated package is `Halloween.qmod`. GitHub Actions builds and validates the QMOD on pushes and pull requests. A new version tag (for example, `v1.0.1`) publishes the QMOD and a source ZIP. Update the QPM and mod template versions before tagging a release.
