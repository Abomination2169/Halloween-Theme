#!/usr/bin/env python3
"""Temporary, exact-match compiler probe for beatsaber-hook 9.4.x.

This is applied only to restored CI dependency headers, never to upstream or
production source. Remove when an upstream release fixes find_method_info.
"""
from pathlib import Path

header = Path("extern/includes/beatsaber-hook/shared/find.hpp")
if not header.is_file():
    raise SystemExit(f"Cannot locate restored dependency header: {header}")
before = header.read_text(encoding="utf-8")
old = (
    "        constexpr find_method_info(\n"
    "            std::convertible_to<std::string_view> auto const& name, "
    "i2c::view<Il2CppClass const*> generics, i2c::view<Il2CppType const*> params\n"
    "        ) :"
)
new = old.replace("constexpr find_method_info(", "find_method_info(", 1)
if before.count(old) != 1:
    raise SystemExit("Upstream find_method_info declaration changed; refusing to patch")
header.write_text(before.replace(old, new, 1), encoding="utf-8")
print("CI-only diagnostic: removed constexpr from 3-argument find_method_info constructor")
