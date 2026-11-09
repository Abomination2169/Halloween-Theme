#!/usr/bin/env python3
"""Read-only source and QMOD metadata preflight for Halloween 1.40.8."""
import json
import re
from pathlib import Path

root = Path(__file__).resolve().parents[1]
qpm = json.loads((root / "qpm.json").read_text(encoding="utf-8"))
manifest = json.loads((root / "mod.template.json").read_text(encoding="utf-8"))
source = (root / "src/main.cpp").read_text(encoding="utf-8-sig")
config = (root / "include/c.hpp").read_text(encoding="utf-8")
workflow = (root / ".github/workflows/build-release.yml").read_text(encoding="utf-8")

assert qpm["info"]["id"] == manifest["id"] == "halloween"
assert qpm["info"]["version"] == manifest["version"] == "1.0.0"
assert manifest["packageVersion"] == "1.40.8_7379"
assert qpm["info"]["additionalData"]["overrideSoName"] == "libhalloween.so"
assert (root / manifest["coverImage"]).is_file()
assert '#include "c.hpp"' in source
assert "DECLARE_CONFIG(ModConfig)" in config
assert "getModConfig().Init(modInfo)" in source

# Ignore comments when checking config values referenced by live code.
def uncomment(text):
    text = re.sub(r"/\\*.*?\\*/", "", text, flags=re.DOTALL)
    return re.sub(r"//[^\\n]*", "", text)

active_source = uncomment(source)
active_config = uncomment(config)
for name in set(re.findall(r"getModConfig\\(\\)\\.(\\w+)\\.GetValue\\(\\)", active_source)):
    assert re.search(r"CONFIG_VALUE\\(\\s*" + re.escape(name) + r"\\s*,", active_config), (
        f"Active config value {name} is not declared in c.hpp"
    )

for entry in manifest["fileCopies"]:
    assert (root / "FileCopies" / Path(entry["name"]).name).is_file()
assert "packageVersion" in workflow and "Halloween-Theme.qmod" in workflow
print("PASS: existing config, 1.40.8 metadata, and required packaging assets")
