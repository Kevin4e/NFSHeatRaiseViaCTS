#pragma once

#include <cstdint>

// Toggle
bool Enable;

// CalculationMode
int32_t CalculationMode;
bool PreventLowerHeat;

// Limits
float MaxHeatLevel;
float MinHeatLevel;

// Thresholds
struct HeatThreshold {
	float heatLevel;
	uint32_t cts;
};

// These are the default values, can be modified in the ini, from 2 to 10, the 1 is the base level
HeatThreshold heatsThresholds[] = {
	{2.0f,  5000},
	{3.0f,  15000},
	{4.0f,  50000},
	{5.0f,  100000},
	{6.0f,  200000},
	{7.0f,  500000},
	{8.0f,  1000000},
	{9.0f,  2000000},
	{10.0f, 5000000},
};