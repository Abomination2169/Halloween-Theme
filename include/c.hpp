#pragma once

// 1.45.3 port-only compatibility defaults.
// Legacy config-utils 2.x requires beatsaber-hook <8 and cannot resolve
// alongside beatsaber-hook 9.3.0. This temporary shim preserves the
// four enabled-by-default behaviors but DOES NOT persist user settings.
// Replace with a compatible settings implementation before release.
struct HalloweenPortFlag {
    bool value = true;
    bool GetValue() const { return value; }
};

struct ModConfig {
    HalloweenPortFlag EnableMenuMusic{};
    HalloweenPortFlag EnableAutumnLeaves{};
    HalloweenPortFlag EnableHauntedEnvColors{};
    HalloweenPortFlag EnablePumpkinAccents{};

    template <typename T>
    void Init(const T&) {}
};

inline ModConfig& getModConfig() {
    static ModConfig config{};
    return config;
}
