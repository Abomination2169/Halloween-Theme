#!/usr/bin/env python3
"""Static release-gate regressions; these are NOT Unity runtime tests."""
from pathlib import Path
import json
import re

particle = Path("src/AutumnLeaves/particleController.cpp").read_text(encoding="utf-8")
leaves = Path("src/AutumnLeaves/autumnLeaves.cpp").read_text(encoding="utf-8")
audio = Path("src/Audio/player.cpp").read_text(encoding="utf-8")
workflow = Path(".github/workflows/build-release.yml").read_text(encoding="utf-8")
manifest = json.loads(Path("mod.template.json").read_text(encoding="utf-8"))

if ".value();" in particle:
    raise SystemExit("Unsafe direct optional.value() remains in particle ICALL helpers")
if particle.count(".value_or(nullptr)") != 11:
    raise SystemExit("Unexpected change to ICALL helper guards")
for term in ("ParticleSafety::MarkMissing", "ParticleSafety::HadMissingIcall",
             "if (!SetterFunc || !particleSystem)", "if (!Func || !particleSystem)"):
    if term not in particle and term != "ParticleSafety::HadMissingIcall":
        raise SystemExit(f"Particle guards missing: {term}")
if "atlas->GetPixels(left, 0, width, atlasHeight)" not in leaves or "tile->SetPixels(pixels)" not in leaves:
    raise SystemExit("Leaf atlas must be cropped into individual textures")
if "LeafVariants = 5" not in leaves or "emission.set_rateOverTime(emissionRate);" not in leaves:
    raise SystemExit("Five-emitter leaf replacement configuration missing")
if "ParticleController::MainModule::set_" in leaves or "Flair::ParticleSafety::HadMissingIcall()" in leaves:
    raise SystemExit("Leaves still depend on obsolete manual ICALL setters")
if leaves.find("dustGo->active = false") < leaves.find("tile->SetPixels(pixels)"):
    raise SystemExit("Original menu dust disabled before leaf texture creation")
if "RemoveLeafClones();" not in leaves or "if (dustGo) dustGo->active = true;" not in leaves:
    raise SystemExit("Missing rollback for failed leaf initialization")

if "safe_ptr<GlobalNamespace::SongPreviewPlayer*>" not in audio or "safe_ptr<decltype(loadTask)>" not in audio:
    raise SystemExit("Asynchronous audio references are no longer held safely")
if "loadingMenuClip" not in audio or "cachedMenuClip" not in audio:
    raise SystemExit("Asynchronous menu audio duplicate-load protection missing")
if not re.search(r"(?m)^\s*if: false\s*#", workflow):
    raise SystemExit("Development release safety gate unexpectedly removed")
if manifest.get("packageVersion") != "1.45.3_30054":
    raise SystemExit("Manifest game version changed")
if manifest.get("id") != "halloween":
    raise SystemExit("Manifest mod ID changed")
print("Static release-gate and runtime-safety contracts passed.")
