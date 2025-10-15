#pragma once

void msgbox(std::string s) {
	MessageBoxA(nullptr, s.c_str(), "Debug", NULL);
}

#include "settings.h"

extern int lowestIndex;
extern int highestIndex;

float HeatLevelProgressiveCalculationDetour(uint32_t cts, uint32_t heatlvl) {
	for (int i = lowestIndex; i < highestIndex; ++i) {
		const auto& ht = heatsThresholds[i];

		if (cts < ht.cts && heatlvl < ht.heatLevel)
			return ht.heatLevel - 1.0f;
	}

	return MaxHeatLevel; // Fallback if none of the conditions are met
}

float HeatLevelProgressiveCalculationDetour_NO_PD(uint32_t cts, uint32_t) {
	for (int i = lowestIndex; i < highestIndex; ++i) {
		const auto& ht = heatsThresholds[i];

		if (cts < ht.cts)
			return ht.heatLevel - 1.0f;
	}

	return MaxHeatLevel; // Fallback if none of the conditions are met
}

float HeatLevelAbsoluteCalculationDetour(uint32_t cts, uint32_t heatlvl) {
	for (int i = highestIndex - 1; i >= lowestIndex; --i) {
		const auto& ht = heatsThresholds[i];

		if (cts >= ht.cts && heatlvl < ht.heatLevel)
			return ht.heatLevel;
	}

	//msgbox(std::to_string(heatlvl));

	return heatlvl; // Fallback if none of the conditions are met
}

float HeatLevelAbsoluteCalculationDetour_NO_PD(uint32_t cts, uint32_t) {
	for (int i = highestIndex - 1; i >= lowestIndex; --i) {
		const auto& ht = heatsThresholds[i];

		if (cts >= ht.cts)
			return ht.heatLevel;
	}

	return MinHeatLevel; // Fallback if none of the conditions are met
}