#pragma once

#include "config-utils/shared/config-utils.hpp"

// The config-utils 2.0.3 DECLARE_CONFIG macro accepts only the config name.
// Keep all defaults enabled to preserve the existing Halloween presentation.
DECLARE_CONFIG(ModConfig) {
    CONFIG_VALUE(EnableMenuMusic, bool, "EnableMenuMusic", true);
    CONFIG_VALUE(EnableAutumnLeaves, bool, "EnableAutumnLeaves", true);
    CONFIG_VALUE(EnablePumpkinAccents, bool, "EnablePumpkinAccents", true);
    CONFIG_VALUE(EnableHauntedEnvColors, bool, "EnableHauntedEnvColors", true);
};
