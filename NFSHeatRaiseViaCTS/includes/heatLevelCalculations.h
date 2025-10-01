#pragma once

#include "settings.h"
#include <ranges>

constexpr float MinHeatLevel = 1.0f;
constexpr float MaxHeatLevel = 10.0f;

float HeatLevelProgressiveCalculationDetour(uint32_t cts, float heatlvl) {
	for (const auto& ht : heatsThresholds) {
		if (cts < ht.cts) {
			if (PreventLowerHeat) {
				if (heatlvl <= ht.heatLevel) {
					return ht.heatLevel - 1.0f;
				}
			}
			else {
				return ht.heatLevel - 1.0f;
			}
		}
	}

	return MaxHeatLevel; // Fallback if none of the conditions are met
}

float HeatLevelAbsoluteCalculationDetour(uint32_t cts, float heatlvl) {
	for (const auto& ht : std::views::reverse(heatsThresholds)) {
		if (cts >= ht.cts) {
			if (PreventLowerHeat) {
				if (heatlvl < ht.heatLevel) {
					return ht.heatLevel;
				}
			}
			else {
				return ht.heatLevel;
			}
		}
	}

	return PreventLowerHeat ? heatlvl : MinHeatLevel; // Fallback if none of the conditions are met
}

float HeatLevelCumulativeCalculationDetour(uint32_t cts, float heatlvl) {
	for (const auto& ht : std::views::reverse(heatsThresholds)) {
		if (cts >= ht.cts) {
			return ht.heatLevel;
		}
	}

	return MinHeatLevel; // Fallback if none of the conditions are met
}