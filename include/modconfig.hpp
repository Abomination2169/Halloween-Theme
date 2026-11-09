#pragma once

#include "config-utils/shared/config-utils.hpp"

// Persisted Quest config used by the installed 1.40.8 hooks.
// Keep defaults enabled to preserve the existing Halloween presentation.
DECLARE_CONFIG(ModConfig,
    CONFIG_VALUE(EnableMenuMusic, bool, "EnableMenuMusic", true);
    CONFIG_VALUE(EnableAutumnLeaves, bool, "EnableAutumnLeaves", true);
    CONFIG_VALUE(EnablePumpkinAccents, bool, "EnablePumpkinAccents", true);
    CONFIG_VALUE(EnableHauntedEnvColors, bool, "EnableHauntedEnvColors", true);
)
