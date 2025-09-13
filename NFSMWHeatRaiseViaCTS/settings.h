#pragma once

#include <cstdint>

// Toggle
bool Enable;

// CalculationMode
std::string CalculationMode;
bool PreventLowerHeat;

// Thresholds
struct HeatThreshold {
	uint32_t heatLevel;
	uint32_t cts;
};

HeatThreshold heatsThresholds[] = {
	{2,  5000},
	{3,  15000},
	{4,  50000},
	{5,  100000},
	{6,  200000},
	{7,  500000},
	{8,  1000000},
	{9,  2000000},
	{10, 5000000},
};