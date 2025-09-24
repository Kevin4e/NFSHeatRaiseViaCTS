#include "pch.h"

#include <windows.h>
#include <algorithm>
#include <cctype>

#include "../includes/iniReader.hpp"
#include "../includes/heatLevelCalculations.h"
#include "../includes/settings.h"

const auto SetHeat = reinterpret_cast<void(*)(float)>(0x612660); // Function pointer to set heat level. Thanks to ExOpts Team for finding this

constexpr DWORD baseAddress = 0x400000;          // 'speed.exe' base address
constexpr DWORD pursuitFlagAddress = 0x92FD34;   // Boolean flag (0 or 1) indicating if player is in a pursuit
constexpr DWORD ctsAddress = 0x91D3F0;           // CTS value address

DWORD WINAPI MainThread(LPVOID) {
    // Ini reader object
    IniReader iniReader("NFSCHeatRaiseViaCTSThresholds.ini");

    // Read values from .ini
    Enable = iniReader.read<int32_t>("Enable", 1);

    if (Enable)
    {
        CalculationMode = iniReader.read<std::string>("CalculationMode", "Progressive");
        PreventLowerHeat = iniReader.read<int32_t>("PreventLowerHeat", 1);

        if (CalculationMode == "progressive" || CalculationMode == "absolute" || CalculationMode == "cumulative")
        {
            for (int i = 0; i < 9; ++i) {
                std::string key("ThresholdForHeat" + std::to_string(i + 2));
                heatsThresholds[i].cts = iniReader.read<uint32_t>(key, heatsThresholds[i].cts);
            }

            // Declare a function pointer with the correct signature
            float (*HeatCalcFunc)(uint32_t, uint32_t) = nullptr;

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

            while (true) {
                if ((*(uint32_t*)pursuitFlagAddress)) { // if in pursuit
                    uint32_t currentCTSValue = *(uint32_t*)ctsAddress;
                    uint32_t currentHeatLevelValue = *(uintptr_t*)(*(uintptr_t*)(baseAddress + 0x593CC8) + 0x104);

                    SetHeat(HeatCalcFunc(currentCTSValue, currentHeatLevelValue));
                }

                Sleep(500);
            }
        }
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