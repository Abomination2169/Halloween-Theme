#pragma once
#include "config-utils/shared/config-utils.hpp"

DECLARE_CONFIG(ModConfig) {
    CONFIG_VALUE(EnableMenuMusic, bool, "Enable Menu Music", true);
    CONFIG_VALUE(EnableAutumnLeaves, bool, "Enable Falling Autumn Leaves", true);
    CONFIG_VALUE(EnableHauntedEnvColors, bool, "Enable Haunted Environment Colors", true);
    CONFIG_VALUE(EnablePumpkinAccents, bool, "Enable Pumpkin Accent Mounds", true);
    // CONFIG_VALUE(EnableBlocks, bool, "Enable Halloween blocks", true);

};