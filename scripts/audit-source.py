#!/usr/bin/env python3
"""Read-only source/manifest guard for Halloween's Beat Saber 1.40.8 branch."""
import json
import re
from pathlib import Path

root = Path(__file__).resolve().parents[1]
qpm = json.loads((root / "qpm.json").read_text(encoding="utf-8"))
manifest = json.loads((root / "mod.template.json").read_text(encoding="utf-8"))
source = (root / "src/main.cpp").read_text(encoding="utf-8-sig")
config = (root / "include/modconfig.hpp").read_text(encoding="utf-8")
main_header = (root / "include/main.hpp").read_text(encoding="utf-8")
workflow = (root / ".github/workflows/build-release.yml").read_text(encoding="utf-8")

assert qpm["info"]["id"] == manifest["id"] == "halloween"
assert qpm["info"]["version"] == manifest["version"] == "1.0.0"
assert manifest["packageVersion"] == "1.40.8_7379"
assert qpm["info"]["additionalData"]["overrideSoName"] == "libhalloween.so"
assert (root / manifest["coverImage"]).is_file()
assert "modconfig.hpp" in main_header
assert "DECLARE_CONFIG(ModConfig" in config
assert "getModConfig().Init(modInfo)" in source
for name in set(re.findall(r"getModConfig\(\)\.(\w+)\.GetValue\(\)", source)):
    assert re.search(r"CONFIG_VALUE\(\s*" + re.escape(name) + r"\s*,", config), (
        f"Config value {name} used but not declared"
    )
for entry in manifest["fileCopies"]:
    assert (root / "FileCopies" / Path(entry["name"]).name).is_file()
assert "packageVersion" in workflow and "Halloween-Theme.qmod" in workflow
print("PASS: 1.40.8 metadata, config references, and required packaging assets")
