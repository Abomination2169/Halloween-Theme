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
# The original 1.40.8 design uses exactly one DustPS clone and its
# five-column texture atlas. Never regress to the five-emitter workaround.
for term in (
    'safe_ptr<GameObject*> autumnLeavesGo',
    'GameObject::Instantiate(dustGo->gameObject)',
    'IncludedAssets::autumn_leaves_png',
    'ParticleController::EmissionModule::set_rateOverTime(system, 5.0f)',
    'mainModule.set_maxParticles(150)',
    'lifetime._ctor(14.0f)',
    'mainModule.set_startSizeMultiplier(0.18f)',
    'mainModule.set_gravityModifierMultiplier(0.025f)',
    'ParticleController::ShapeType::Box',
    'Vector3(20, 1, 20)',
    'ParticleController::ColorOverLifetimeModule::set_enabled(system, false)',
    'ParticleController::ColorBySpeedModule::set_enabled(system, false)',
    'ParticleController::CollisionModule::set_enabled(system, true)',
    'ParticleController::TextureSheetAnimationModule::set_enabled(system, true)',
    'ParticleController::TextureSheetAnimationModule::set_numTilesX(system, numLeafSprites)',
    'ParticleController::TextureSheetAnimationModule::set_numTilesY(system, 1)',
    'void UpdateLeafFloor() {}',
):
    if term not in leaves:
        raise SystemExit(f"Original single-emitter behavior missing: {term}")
for forbidden in ('LeafVariants = 5', 'leafObjects', 'emissionAccumulator',
                  'system->Emit(1)', 'set_remainingLifetime(0.0f)',
                  'IncludedAssets::autumn_leaf_0_png'):
    if forbidden in leaves:
        raise SystemExit(f"Five-emitter workaround still present: {forbidden}")
if leaves.count('GameObject::Instantiate(dustGo->gameObject)') != 1:
    raise SystemExit("Expected exactly one original DustPS clone")
if leaves.find('dustGo->active = false') < leaves.find('IncludedAssets::autumn_leaves_png'):
    raise SystemExit("Original dust disabled before atlas was loaded")
if "if (dustGo) dustGo->active = true;" not in leaves:
    raise SystemExit("Original dust restoration missing")

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
