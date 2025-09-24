#include "pch.h"

#include <windows.h>

#include "../includes/iniReader.hpp"
#include "../includes/heatLevelCalculations.h"
#include "../includes/settings.h"
#include "../includes/helpers.h"
#include "../includes/logger.h"

//#include "../includes/injector/injector.hpp"

constexpr std::uintptr_t setHeatLevelAddress = 0x00612660;  // Instruction address of the instruction that sets the heat level globally. Thanks to ExOpts Team for finding this
constexpr std::uintptr_t baseAddress = 0x00400000;          // 'speed.exe' base address
constexpr std::uintptr_t pursuitFlagAddress = 0x0092FD34;   // Boolean flag (0 or 1) indicating if player is in a pursuit
constexpr std::uintptr_t ctsAddress = 0x0091D3F0;           // CTS value address
constexpr std::uintptr_t gameStateAddress = 0x00925E90;     // Game state address (3 = FrontEnd, 4 & 5 = Loading Screen, 6 = Free Roam)

void(*setHeat)(float newHeatLevel) = (void(*)(float))setHeatLevelAddress; // Function pointer to set heat level. 

DWORD WINAPI MainThread(LPVOID) {
    // Ini reader object
    IniReader iniReader("NFSHeatRaiseViaCTSThresholds.ini");

    // Read values from .ini

	CreateLog = iniReader.read<int32_t>("CreateLog", 1);

    Log
    (
        "\n\n"
        "========================================\n"
        "New Session Started at: " + getTimestamp() + '\n' +
        "========================================\n",
        false
    );

    Enable = iniReader.read<int32_t>("Enable", 1);

    if (!Enable) {
		Log("NFSMWHeatRaiseViaCTS.asi disabled via .ini. Exiting...");
        return 1;
    }

	Log("NFSMWHeatRaiseViaCTS.asi enabled via .ini. Continuing...");

    CalculationMode = iniReader.read<std::string>("CalculationMode", "Progressive");

    if (CalculationMode != "progressive" && CalculationMode != "absolute" && CalculationMode != "cumulative") {
		Log("Invalid CalculationMode in .ini, valid options are: progressive, absolute, cumulative. The user entered: " + CalculationMode + ". Exiting...\n");
        return 1;
    }

    Log("CalculationMode read from .ini: " + CalculationMode + ". Continuing...");

    PreventLowerHeat = iniReader.read<int32_t>("PreventLowerHeat", 1);
    
    for (int i = 0; i < 9; ++i) {
        heatsThresholds[i].cts = iniReader.read<uint32_t>("ThresholdForHeat" + std::to_string(i + 2), heatsThresholds[i].cts);
    }

	Log("All thresholds have been successfully read from .ini. Continuing...");

    float (*HeatCalcFunc)(uint32_t, float) = nullptr; // Declare a function pointer with the correct signature

    if (CalculationMode == "progressive") {
        HeatCalcFunc = HeatLevelProgressiveCalculationDetour;
    }
    else if (CalculationMode == "absolute") {
        HeatCalcFunc = HeatLevelAbsoluteCalculationDetour;
    }
    else {
        std::sort(heatsThresholds, heatsThresholds + 9, [](const HeatThreshold& a, const HeatThreshold& b) {
            return a.cts < b.cts;
        });

        HeatCalcFunc = HeatLevelCumulativeCalculationDetour;
    }

    while (readMemory<uint32_t>(gameStateAddress) != 6) { // Wait until we're in free roam once to prepare variables/pointer
        Sleep(1000);
    } 

    float newHeatLevel;

    uint32_t currentCTSValue;
    float currentHeatLevelValue;

    // Prepare heatlevel address pointer
    uintptr_t ptr = readMemory<uintptr_t>(baseAddress + 0x0052D378);
    ptr = readMemory<uintptr_t>(ptr + 0x1C);
    ptr = readMemory<uintptr_t>(ptr + 0x4);
    ptr = readMemory<uintptr_t>(ptr + 0x24);
    ptr += 0x1C;

    uintptr_t ptrToHeatLevel = ptr;

    /*
    * Known issues:
    * 
    * 1. Heat oscillation:
    *    - When current heat is 3 and a higher heat is calculated every frame,
    *      the value alternates between 3 and the new heat level.
    * 
    * 2. PreventLowerHeat bug:
    *    - If PreventLowerHeat is enabled and the default heat is >3,
    *      it resets to 3 once a pursuit starts.
    * 
    * Both issues likely stem from the same underlying problem in the handling of heat level 3.
    */
    while (true) {
        if (readMemory<uint32_t>(pursuitFlagAddress)) { // if in pursuit
            currentCTSValue = readMemory<uint32_t>(ctsAddress);
            currentHeatLevelValue = readMemory<float>(ptrToHeatLevel);

            newHeatLevel = HeatCalcFunc(currentCTSValue, currentHeatLevelValue);

            setHeat(newHeatLevel);
        }

        Sleep(100);
    }

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, MainThread, nullptr, 0, nullptr);
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}