#pragma once

// Standalone, persistent configuration for the 1.45.3 port.
// config-utils 2.x is incompatible with beatsaber-hook 9.x.
// File: /sdcard/ModData/com.beatgames.beatsaber/Mods/Halloween/settings.cfg
// Format: four explicit key=true|false entries. Unknown keys are ignored.
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

struct HalloweenPortFlag {
    bool value = true;
    bool GetValue() const { return value; }
    void SetValue(bool enabled) { value = enabled; }
};

struct ModConfig {
    HalloweenPortFlag EnableMenuMusic{};
    HalloweenPortFlag EnableAutumnLeaves{};
    HalloweenPortFlag EnableHauntedEnvColors{};
    HalloweenPortFlag EnablePumpkinAccents{};

    static constexpr const char* SettingsPath =
        "/sdcard/ModData/com.beatgames.beatsaber/Mods/Halloween/settings.cfg";

    static bool ParseBool(std::string_view input, bool& output) {
        if (input == "true" || input == "1") { output = true; return true; }
        if (input == "false" || input == "0") { output = false; return true; }
        return false;
    }

    bool Load() {
        std::ifstream file(SettingsPath);
        if (!file.is_open()) return false;
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;
            const auto separator = line.find('=');
            if (separator == std::string::npos) continue;
            const auto key = std::string_view(line).substr(0, separator);
            const auto raw = std::string_view(line).substr(separator + 1);
            bool enabled = false;
            if (!ParseBool(raw, enabled)) continue;
            if (key == "EnableMenuMusic") EnableMenuMusic.SetValue(enabled);
            else if (key == "EnableAutumnLeaves") EnableAutumnLeaves.SetValue(enabled);
            else if (key == "EnableHauntedEnvColors") EnableHauntedEnvColors.SetValue(enabled);
            else if (key == "EnablePumpkinAccents") EnablePumpkinAccents.SetValue(enabled);
        }
        return !file.bad();
    }

    bool Save() const {
        const std::filesystem::path destination(SettingsPath);
        std::error_code ec;
        std::filesystem::create_directories(destination.parent_path(), ec);
        if (ec) return false;
        const auto temporary = destination.string() + ".tmp";
        {
            std::ofstream file(temporary, std::ios::out | std::ios::trunc);
            if (!file) return false;
            file << "# Halloween Theme 1.45.3 settings\n"
                 << "EnableMenuMusic=" << (EnableMenuMusic.GetValue() ? "true" : "false") << '\n'
                 << "EnableAutumnLeaves=" << (EnableAutumnLeaves.GetValue() ? "true" : "false") << '\n'
                 << "EnableHauntedEnvColors=" << (EnableHauntedEnvColors.GetValue() ? "true" : "false") << '\n'
                 << "EnablePumpkinAccents=" << (EnablePumpkinAccents.GetValue() ? "true" : "false") << '\n';
            file.flush();
            if (!file.good()) return false;
        }
        std::filesystem::rename(temporary, destination, ec);
        if (ec) {
            std::filesystem::remove(temporary, ec);
            return false;
        }
        return true;
    }

    template <typename T> void Init(const T&) {
        // On first launch, write default-enabled settings. If a file already
        // exists, preserve its contents, even if some keys are malformed.
        try {
            if (!Load()) {
                std::error_code ec;
                if (!std::filesystem::exists(SettingsPath, ec) && !ec) Save();
            }
        } catch (...) {
            // setup() is noexcept: configuration I/O must never terminate Beat Saber.
        }
    }
};

inline ModConfig& getModConfig() {
    static ModConfig config{};
    return config;
}
