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
if "BSML::Utilities::LoadTextureRaw(encodedLeaves[i])" not in leaves:
    raise SystemExit("Individual leaves must be decoded via BSML")
if "GetPixels(" in leaves or "SetPixels(" in leaves:
    raise SystemExit("Runtime pixel copy path causes white leaf cubes")
if "scripts/split-leaf-atlas.py" not in workflow:
    raise SystemExit("Build must split atlas into standalone PNGs")
if "LeafVariants = 5" not in leaves or "emission.set_rateOverTime(emissionRate);" not in leaves:
    raise SystemExit("Five-emitter leaf replacement configuration missing")
if "system->Stop(true);" not in leaves or "system->Clear(true);" not in leaves or "system->Play();" not in leaves:
    raise SystemExit("Leaf clones must clear inherited particles and restart after configuration")
if "emissionRate._ctor(1.0f)" not in leaves or "mainModule.set_maxParticles(150)" not in leaves:
    raise SystemExit("Five leaf emitters must retain original aggregate emission and particle cap")
# The 1.45.3 collision-restoration test deliberately removes the synthetic
# Y=0.5 particle kill plane. Require actual Unity world collision instead.
if "UpdateLeafFloor()" not in leaves or "system->Emit(1)" not in leaves:
    raise SystemExit("Working 1.45.3 emission path was removed")
for term in (
    "Flair::ParticleController::CollisionModule::set_enabled(system, true)",
    "Flair::ParticleController::CollisionModule::set_type(",
    "Flair::ParticleController::CollisionModule::set_collidesWith(system, allLayers)",
    "Flair::ParticleController::CollisionModule::set_dampen(",
    "Flair::ParticleController::CollisionModule::set_bounce(",
    "Flair::ParticleController::CollisionModule::set_lifetimeLoss(",
):
    if term not in leaves:
        raise SystemExit(f"World leaf collision configuration missing: {term}")
for term in (
    "ColorOverLifetimeModule::set_enabled(system, false)",
    "ColorBySpeedModule::set_enabled(system, false)",
    "ShapeModule::set_shapeType(",
    "ShapeType::Box",
    "Vector3(20.0f, 1.0f, 20.0f)",
    "white._ctor(Color(1, 1, 1, 1.0f))",
    "Time::get_deltaTime() * 5.0f",
):
    if term not in leaves:
        raise SystemExit(f"Original Halloween leaf behavior missing: {term}")
if "FloorY = 0.5f" in leaves or "set_remainingLifetime(0.0f)" in leaves:
    raise SystemExit("Artificial raised floor cutoff remains in collision-restoration test")
if "mainModule.set_simulationSpace(ParticleSystemSimulationSpace::World)" not in leaves:
    raise SystemExit("Leaf particles must simulate in world space")
if "ParticleController::MainModule::set_" in leaves or "Flair::ParticleSafety::HadMissingIcall()" in leaves:
    raise SystemExit("Leaves still depend on obsolete manual ICALL setters")
if leaves.find("dustGo->active = false") < leaves.find("BSML::Utilities::LoadTextureRaw(encodedLeaves[i])"):
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
