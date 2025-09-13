#include "pch.h"
#include <windows.h>
#include <string>
#include <algorithm>

#include "../includes/injector/injector.hpp"
#include "iniReader.hpp"

#include "assembly.h"
#include "settings.h"

void Init() {
	// Ini reader object
	IniReader iniReader("NFSMWHeatRaiseViaCTSThresholds.ini");

	// Read values from .ini
	bool Enable = iniReader.readInt32("Enable", 1);

	if (Enable)
	{
		CalculationMode = iniReader.readString("CalculationMode", "Progressive");
		PreventLowerHeat = iniReader.readInt32("PreventLowerHeat", 1);

		std::transform(CalculationMode.begin(), CalculationMode.end(), CalculationMode.begin(), ::tolower);
		
		if (CalculationMode == "progressive" || CalculationMode == "absolute" || CalculationMode == "cumulative")
		{
			for (int i = 0; i < 9; ++i) {
				std::string key("ThresholdForHeat" + std::to_string(i + 2));
				heatsThresholds[i].cts = iniReader.readUInt32(key, heatsThresholds[i].cts);
			}

			if (CalculationMode == "progressive") {
				if (PreventLowerHeat)
					injector::MakeJMP(0x71D31B, HeatLevelProgressiveCalculationDetour, true);
				else
					injector::MakeJMP(0x71D31B, HeatLevelProgressiveCalculationDetour_NO_PD, true);
			}
			else if (CalculationMode == "absolute") {
				if (PreventLowerHeat)
					injector::MakeJMP(0x71D31B, HeatLevelAbsoluteCalculationDetour, true);
				else
					injector::MakeJMP(0x71D31B, HeatLevelAbsoluteCalculationDetour_NO_PD, true);
			}
			else {
				std::sort(heatsThresholds, heatsThresholds + 9, [](const HeatThreshold& a, const HeatThreshold& b) {
					return a.cts < b.cts;
				});

				injector::MakeJMP(0x71D31B, HeatLevelCumulativeCalculationDetour, true);
			}
		}
	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		Init();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}