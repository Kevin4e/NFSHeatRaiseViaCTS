#pragma once

#include "settings.h"
#include <cstdint>

float HeatLevelProgressiveCalculationDetour(uint32_t cts, float heatlvl) {
	for (int i = 0; i < 9; ++i) {
		if (cts < heatsThresholds[i].cts) {
			if (PreventLowerHeat) {
				if (heatlvl <= heatsThresholds[i].heatLevel) {
					return heatsThresholds[i].heatLevel - 1.0f;
				}
			}
			else {
				return heatsThresholds[i].heatLevel - 1.0f;
			}
		}
	}

	return 10.0f; // Fallback if none of the conditions are met
}

float HeatLevelAbsoluteCalculationDetour(uint32_t cts, float heatlvl) {
	for (int i = 8; i >= 0; --i) {
		if (cts >= heatsThresholds[i].cts) {
			if (PreventLowerHeat) {
				if (heatlvl < heatsThresholds[i].heatLevel) {
					return heatsThresholds[i].heatLevel;
				}
				else {
					return heatlvl;
				}
			}
			else {
				return heatsThresholds[i].heatLevel;
			}
		}
	}

	return heatlvl; // Fallback if none of the conditions are met
}

float HeatLevelCumulativeCalculationDetour(uint32_t cts, float heatlvl) {
	for (int i = 8; i >= 0; --i) {
		if (cts >= heatsThresholds[i].cts) {
			return heatsThresholds[i].heatLevel;
		}
	}

	return 1.0f; // Fallback if none of the conditions are met
}