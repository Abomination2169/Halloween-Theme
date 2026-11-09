// Host-side smoke test of the same settings implementation compiled into the QMOD.
// The test uses a temporary directory, never the Quest /sdcard path.
#include "c.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>

int main() {
    namespace fs = std::filesystem;
    const fs::path path(ModConfig::SettingsPath);
    std::error_code ec;
    fs::remove_all(path.parent_path(), ec);

    ModConfig first;
    first.Init(0);
    assert(fs::exists(path));
    assert(first.EnableMenuMusic.GetValue());
    assert(first.EnableAutumnLeaves.GetValue());
    assert(first.EnableHauntedEnvColors.GetValue());
    assert(first.EnablePumpkinAccents.GetValue());

    first.EnableMenuMusic.SetValue(false);
    first.EnableAutumnLeaves.SetValue(false);
    assert(first.Save());

    ModConfig second;
    second.Init(0);
    assert(!second.EnableMenuMusic.GetValue());
    assert(!second.EnableAutumnLeaves.GetValue());
    assert(second.EnableHauntedEnvColors.GetValue());
    assert(second.EnablePumpkinAccents.GetValue());

    // Invalid values and unknown keys must not override valid settings.
    {
        std::ofstream file(path, std::ios::app);
        file << "EnablePumpkinAccents=garbage\nUnknownSetting=false\n";
    }
    ModConfig third;
    third.Init(0);
    assert(!third.EnableMenuMusic.GetValue());
    assert(third.EnablePumpkinAccents.GetValue());

    // An existing file must never be overwritten just because it is malformed.
    {
        std::ofstream file(path, std::ios::trunc);
        file << "bad line without equals\n";
    }
    ModConfig fourth;
    fourth.Init(0);
    std::ifstream readback(path);
    std::string line;
    std::getline(readback, line);
    assert(line == "bad line without equals");

    fs::remove_all(path.parent_path(), ec);
}
