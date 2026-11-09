# Halloween Theme — Beat Saber 1.45.3 Quest 3 port

Status: **first 1.45.3 metadata/dependency build candidate staged; compilation and runtime compatibility are unverified**.

## Preservation rules

- Production `main` and the verified Beat Saber `1.40.8_7379` v1.0.0 release remain untouched.
- All port work is isolated on `port/halloween-1.45.3` and a draft pull request.
- Do not install a port build on the working 1.40.8 headset.
- Preserve the Halloween menu track, pumpkin PNGs, gravestones, leaf sprite sheet, sign, lighting, and placement.

## Verified target and ecosystem references

- QuestPackageManager/bsqmods has a `mods/1.45.3_30054` directory, identifying the target Quest package build.
- Its 1.45.3 listings include `bsml` 0.6.6, `custom-types` 0.21.2, and `paper2_scotland2` 4.8.0.
- The 1.45.3 BSML and custom-types listings depend on `beatsaber-hook` ^9.3.0.
- QuestPackageManager/bs-cordl published `v4503.0.0` on 2026-10-15. This is the selected candidate generated-header version; CI must establish compatibility. This is a major change from our 1.40.8 dependency ^6.4.2.
- These are **distribution-listing versions**, not proof that the Halloween C++ APIs or a matching QPM header set compile.

Sources:
- https://github.com/QuestPackageManager/bsqmods/tree/main/mods/1.45.3_30054
- https://github.com/QuestPackageManager/bsqmods/blob/main/mods/1.45.3_30054/bsml-0.6.6.json
- https://github.com/QuestPackageManager/bsqmods/blob/main/mods/1.45.3_30054/custom-types-0.21.2.json

## Porting sequence

1. **Staged:** Pin the `bs-cordl` ^4503.0.0 candidate and update QPM dependencies to beatsaber-hook ^9.3.0, BSML ^0.6.6, custom-types ^0.21.2 and Paperlog ^4.8.0. Scotland2 and config-utils remain on their previous ranges pending compatibility evidence.
2. **Staged:** Update `qpm.json` on this branch only. QPM restore/lock resolution must be validated in CI.
3. **Staged:** Set `mod.template.json` `packageVersion` to `1.45.3_30054` and development mod version to `1.1.0` (not published). Keep `id: halloween` and `libhalloween.so`.
4. **Staged:** Update the port branch's CI checks to target 1.45.3, leaving the existing 1.40.8 checks on `main` unchanged.
5. Compile and fix C++ API differences, especially `SongPreviewPlayer`, `MediaAsyncLoader`, `MainMenuViewController`, `LightWithIdManager`, `BSML::Lite`, and particle APIs.
6. Verify QMOD archive manifest, assets, fileCopies, and library.
7. Test on a **separate 1.45.3 Quest setup** only after CI passes. Compare music, menu geometry, lights, leaf colors, sign, transitions, and repeated menu visits.
8. Keep the port PR in draft until the user authorizes release. Do not merge the 1.45.3 port into production `main` by default because that would replace the 1.40.8 source target.

## Current blockers

- `bs-cordl` ^4503.0.0 is a published candidate; QPM restore and API compatibility have not yet been established.
- No ARM64 compile or runtime verification has been completed for 1.45.3.

The staged metadata and CI target changes do not modify the Halloween runtime source. The port is not yet a validated QMOD.

## First CI findings (2026-11-09)

- GitHub Actions run [42631061051](https://github.com/Abomination2169/Halloween-Theme/actions/runs/42631061051) passed source audit, tool installation and NDK resolution, then **failed QPM restore**.
- Resolver explicitly reported that `config-utils` 2.0.3 requires beatsaber-hook 6.x and 2.0.4 requires 7.x; neither is compatible with the port's beatsaber-hook 9.3.0.
- For the next CI experiment, the port branch removes `config-utils` and replaces its config macros with a **temporary defaults-only shim** in `include/c.hpp`. All four features remain enabled. This shim **does not load/save persistent settings**, so it is not a release-ready settings migration.
- CI must establish the next dependency or C++ compile blocker. Do not interpret this change as a successful 1.45.3 port.

## CI compile investigation (run 42631395169)

- Dependency restore, QMOD manifest generation/validation and Android CMake configuration **passed**.
- Compilation stopped on the removed `beatsaber-hook/shared/config/config-utils.hpp` header and C++20 not supporting `std::expected` used by beatsaber-hook 9.x.
- Port branch removes the obsolete include from `include/main.hpp` and sets CMake C++ standard to 23. This addresses only the observed first-layer toolchain errors; generated API migration remains outstanding.
- No successful port QMOD or Quest runtime validation yet.

## Next compile run (42631735184)

- QPM restored `bs-cordl` 4503.0.0 and `beatsaber-hook` 9.4.3; compile still failed.
- `include/main.hpp` referenced removed legacy `beatsaber-hook/shared/utils/hooking.hpp`. Port branch now uses `beatsaber-hook/shared/hooking.hpp` and removes legacy il2cpp-functions/logging includes; **this change is unverified**.
- A separate generated-header incompatibility remains: `beatsaber-hook/shared/find.hpp` `find_method_info` constexpr construction and numerous `bs-cordl` generated `zzzz__*_impl.hpp` constructors. Investigate the exact C++ toolchain/header API combination before attempting any broad replacements.
- Next errors may expose obsolete `MAKE_HOOK_MATCH`, `INSTALL_HOOK`, `il2cpp_functions::Init` and generated Beat Saber API signatures. No runtime testing until a clean compile and QMOD archive validation.

## Run 42632239406 (2026-11-09)

- Dependency restoration and Android configuration succeeded, but C++ compilation failed.
- Current primary upstream header problem: beatsaber-hook 9.4.3 `shared/find.hpp:73` declares a `constexpr` constructor accepting `i2c::view`, whose constructors in `shared/utils.hpp` are not `constexpr`. Clang rejects this, causing cascaded generated `bs-cordl` errors. This must be resolved against upstream compatibility, **not** by bulk-editing generated headers.
- A separate local issue is the asset generator's obsolete `beatsaber-hook/shared/utils/typedefs.h` include; the port's `assets.cmake` was updated to use `shared/arrayw.hpp` and `i2c::class_of`. This change is pending CI verification.
- The port still needs updated hooks and a settings implementation; no 1.45.3 QMOD is validated.


## CI investigation: run 42632764024

- Dependency installation, manifest validation and CMake configuration passed; compilation failed.
- The previous missing `typedefs.h` error is no longer present after updating `assets.cmake`.
- The earliest persistent failure comes from beatsaber-hook's `shared/find.hpp`: a `constexpr find_method_info` constructor takes non-literal `i2c::view` parameters; this causes cascaded generated `bs-cordl` constructor errors. The same declaration appears in upstream beatsaber-hook `v9.4.3` and `master` as checked on 2026-11-09.
- A strictly guarded, **CI-only diagnostic** in `scripts/diagnose-hook-constexpr.py` removes `constexpr` from exactly this one dependency declaration after QPM restore. It refuses to run if the upstream declaration differs. This is not a runtime fix or an upstream source change; assess its compiler result and replace it with an upstream-supported solution before release.
- Do not deploy any build until all APIs, packages and game behavior pass validation.


## Run 42633254429 — compiler migration

- The guarded, CI-only beatsaber-hook `find_method_info` diagnostic passed, allowing compilation to reach Halloween source files. This is **not** a supported upstream fix and must be replaced or justified before release.
- Compilation then failed on legacy `il2cpp_utils::resolve_icall`, `Array::get_Length()`, `SafePtrUnity`, and `il2cpp_functions::Init()` APIs.
- First-pass branch-only replacements use `i2c::resolve_icall(...).value()`, `Array::max_length`, `safe_ptr<UnityEngine::GameObject*>`, and `i2c::functions::initialize()`. These changes are **compiler candidates**, not gameplay-tested modifications.
- Review `safe_ptr` lifecycle, Unity object destruction and particle-system icall signatures before any Quest installation. CI must prove the build first.

## Run 42633805936 — source compilation passes

- All seven C++ translation units compiled successfully, including Halloween music, leaves, pumpkins, hooks, and menu effects.
- Linking `libhalloween.so` failed on `flamingo::Install(flamingo::HookInfo&&)`. The link command did not include `libflamingo.so`, even though the restored beatsaber-hook headers refer to Flamingo.
- Added a direct `flamingo` QPM dependency `^1.3.1` on the **port branch only** so the generated QPM link configuration includes its library (to be confirmed in the next CI run).
- A successful link is not proof of runtime compatibility; the guarded upstream-header diagnostic, configuration shim, and Quest runtime behavior still require validation before installation.

## First successful CI package — run 42634367459 (2026-11-09)

- Build workflow completed **successfully** at commit `48cffed78d09f45dc38d5fdcf4f12b1987c61aaf`.
- QPM restore, CI-only beatsaber-hook constexpr diagnostic, 1.45.3 manifest, all 7 C++ translation units, ARM64 shared-library link, QMOD zip generation, asset verification and upload-artifact checks passed.
- Artifact: [Halloween-1453-QMOD](https://github.com/Abomination2169/Halloween-Theme/actions/runs/42634367459/artifacts/13680994498), ~5.6 MB as uploaded ZIP; contains `Halloween-Theme.qmod`.
- Linker fix: adding explicit `flamingo ^1.3.1` QPM dependency resolved `flamingo::Install(flamingo::HookInfo&&)`.
- **NOT release-ready**: no Quest headset runtime test; temporary header diagnostic edits downloaded dependency headers in CI; port config shim does not persist settings; need inspect runtime hook ABI and particle ICALL safety/appearance. Keep draft PR unmerged and preserve production `main` plus v1.0.0 release.

## Release safety gate (2026-11-09)

- The port branch's GitHub Actions `release` job is explicitly disabled (`if: false`) until Quest 3 smoke tests, dependency manifest verification, settings persistence and the upstream header compatibility patch are resolved. This does **not** change production `main` or existing releases.
- A new CI check inspects the built ARM64 ELF and requires a `libflamingo.so` DT_NEEDED entry. CI result pending at time of change.
- The QMOD manifest currently has an empty `dependencies` array in `mod.template.json`; review whether the generated package declares all required runtime mods before any install or release.

## QMOD runtime dependency audit

- CI now prints and validates generated `mod.json` dependency IDs after `qpm qmod manifest`. Required IDs: `beatsaber-hook`, `bsml`, `custom-types`, `paper2_scotland2`, `flamingo`.
- This check deliberately runs **after** QPM manifest generation and fails closed if required dependencies are absent. It does not assume that linking the `.so` automatically populates the QMOD dependency manifest.
- Confirm the next CI result before treating the package as installable. Do not merge or release the draft port.

## Runtime dependency audit: PASS (2026-11-09)

- [GitHub Actions run 42636062683](https://github.com/Abomination2169/Halloween-Theme/actions/runs/42636062683) passed the generated manifest dependency audit, ARM64 ELF check, QMOD packaging and artifact upload.
- QPM generated five installable `mod.json` dependencies with downloadIfMissing URLs: `paper2_scotland2` `^4.8.0`; `beatsaber-hook` `^9.3.0` (resolved 9.4.3); `bsml` `^0.6.6`; `custom-types` `^0.21.2`; and `flamingo` `^1.3.1`. The source template's empty dependencies array is populated by `qpm qmod manifest` as intended.
- The build still applies a temporary upstream beatsaber-hook header diagnostic and settings remain defaults-only. No Quest 3 runtime verification yet. Leave draft PR open and release disabled.
