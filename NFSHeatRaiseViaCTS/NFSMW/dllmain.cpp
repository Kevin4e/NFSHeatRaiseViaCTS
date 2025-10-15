#include <windows.h>
#include <cstdint>
#include <algorithm>
#include <fstream>
#include <string>

#include "../includes/iniReader.hpp"
#include "../includes/heatLevelCalculations.h"
#include "../includes/settings.h"
#include "../includes/helpers.h"

#include "assembly.h"

constexpr DWORD setHeatLevelAddress = 0x00612660;  // Instruction address of the instruction that sets the heat level globally. Thanks to ExOpts Team for finding this
constexpr DWORD baseAddress = 0x00400000;          // 'speed.exe' base address
constexpr DWORD pursuitFlagAddress = 0x0092FD34;   // Boolean flag (0 or 1) indicating if player is in a pursuit
constexpr DWORD ctsAddress = 0x0091D3F0;           // CTS value address
constexpr DWORD gameStateAddress = 0x00925E90;     // Game state address (3 = Front-End, 4 & 5 = Loading Screen, 6 = Free Roam)

// Heat level stable pointer offsets
constexpr DWORD moduleOffset = 0x00593CC8;         // Module offset
constexpr DWORD finalOffset = 0x00000104;          // Final offset

void (*const setHeat)(float) = reinterpret_cast<void(*)(float)>(setHeatLevelAddress); // Function pointer to set heat level. 

float (*HeatCalcFunc)(uint32_t, uint32_t) = nullptr; // Function pointer to one of the heat level calculation ones

int lowestIndex;
int highestIndex;

constexpr const char* readableIniFiles[2] = {
    "NFSHeatRaiseViaCTSThresholds.ini",
    "NFSMWHeatRaiseViaCTSThresholds.ini"
};

DWORD WINAPI MainThread(LPVOID) {
    while (readMemory<uint32_t>(gameStateAddress) != 6) // Wait until we're in free roam once to get heat level pointer
        Sleep(100);

    const DWORD ptrToHeatLevel = readMemory<DWORD>(baseAddress + moduleOffset) + finalOffset;

	uint32_t previousCTS = 0;
    float previousHeatLevel = 0;

    while (true) {
        if (readMemory<uint32_t>(pursuitFlagAddress)) { // if in pursuit
            const uint32_t currentCTSValue = readMemory<uint32_t>(ctsAddress);
            const uint32_t currentHeatLevelValue = readMemory<uint32_t>(ptrToHeatLevel);

            if (previousCTS != currentCTSValue) {
                previousCTS = currentCTSValue;

				const float newHeatLevel = HeatCalcFunc(currentCTSValue, currentHeatLevelValue);

                if (newHeatLevel != previousHeatLevel) {
                    previousHeatLevel = newHeatLevel;
                    setHeat(newHeatLevel);
                }
                else {
                    setHeat(currentHeatLevelValue);
                }
            }
            else {
                setHeat(currentHeatLevelValue);
            } 
        }

        Sleep(50);
    }

    return 0;
}

// Read INI, configure everything, start MainThread()
void Setup() {

    // Find the iterator that points to the first ini file that exists
    const auto it = std::find_if(std::begin(readableIniFiles), std::end(readableIniFiles), [](const auto& ini) { return std::ifstream(ini).good(); });

    if (it == std::end(readableIniFiles))
        return; // End if no readable .ini file exists

    // INI reader object
    INIReader iniReader(*it);
        
    // Read values from the INI
    {
        Enable = iniReader.read<int32_t>("Enable", 0);

        if (!Enable)
            return;

        CalculationMode = iniReader.read<int32_t>("CalculationMode", 0);

        if (CalculationMode < 0 || CalculationMode > 2)
			return; // End if calculation mode is out of bounds

        PreventLowerHeat = iniReader.read<int32_t>("PreventLowerHeat", 0);

        MaxHeatLevel = iniReader.read<float>("MaxHeatLevel", 10.0f);
        MinHeatLevel = iniReader.read<float>("MinHeatLevel", 1.0f);

        if (MaxHeatLevel < 1.0f || MaxHeatLevel > 10.0f || (MinHeatLevel > MaxHeatLevel))
            return; // End if heat levels are out of bounds or minimum is greater than maximum

        for (int i = 0; i < 9; ++i)
            heatsThresholds[i].cts = iniReader.read<uint32_t>("ThresholdForHeat" + std::to_string(i + 2), heatsThresholds[i].cts);
    }

	// Apply configuration
    {
        switch (CalculationMode) {
            case 0: // Progressive
                HeatCalcFunc = PreventLowerHeat ? HeatLevelProgressiveCalculationDetour : HeatLevelProgressiveCalculationDetour_NO_PD;
                break;

            case 1: // Absolute
                HeatCalcFunc = PreventLowerHeat ? HeatLevelAbsoluteCalculationDetour : HeatLevelAbsoluteCalculationDetour_NO_PD;
                break;

            case 2: // Cumulative
                std::sort(std::begin(heatsThresholds), std::end(heatsThresholds), [](const HeatThreshold& a, const HeatThreshold& b) {
                    return a.cts < b.cts;
                });

                // The cumulative algorithm is the same as the absolute one without preventing lower heat, but it needs the thresholds sorted
                HeatCalcFunc = HeatLevelAbsoluteCalculationDetour_NO_PD;

                break;

            default: return;
        }

        highestIndex = MaxHeatLevel - 1;
        lowestIndex = MinHeatLevel - 1;

        /*
         * Respectively:
         *
         *  - Prepares the game for Ultimate force heat level hack;
         *  - Sets the max heat level recognizable by Front-End
         *
         *  Credits to ExOpts Team
         */

        makeJMP(0x443DC3, HeatLevelsCodeCave);

        // CustomizeMeter::Init
        writeMemory<float>(0x7BB502, MaxHeatLevel); // CustomizeCategoryScreen
        writeMemory<float>(0x7B1387, MaxHeatLevel); // CustomizationScreenHelper
        writeMemory<float>(0x7B0C89, MaxHeatLevel); // CustomizeShoppingCart::Setup
        writeMemory<float>(0x7B4D7C, MaxHeatLevel); // UIQRCarSelect::InitStatsSliders

        // SetHeatLevel
        writeMemory<const float*>(0x435079, &MaxHeatLevel); // AIVehicleHuman::~AIVehicleHuman
        writeMemory<float>(0x435088, MaxHeatLevel);

        // Safehouse car select icon stuff (HEAT_X%.0f)
        writeMemory<const float*>(0x7A5B03, &MaxHeatLevel);
        writeMemory<const float*>(0x7A5B12, &MaxHeatLevel);
    }

    CreateThread(nullptr, 0, MainThread, nullptr, 0, nullptr);
}

extern "C" __declspec(dllexport) void InitializeASI() {
    // Check if .exe file is compatible - Thanks to thelink2012 and MWisBest
    // Simplified condition for clarity; logic unchanged, there were a few redundant operations

    uintptr_t base = (uintptr_t)GetModuleHandleA(nullptr);
    IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)(base);
    IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)(base + dos->e_lfanew);

    if (nt->OptionalHeader.AddressOfEntryPoint == 0x3C4040)
        Setup();

    else
        MessageBoxA(nullptr, "This .exe is not supported.\nPlease use v1.3 speed.exe (5.75 MB (6.029.312 bytes)).", "NFSMW Heat Raise via CTS by Kevin4e", MB_ICONERROR);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
        DisableThreadLibraryCalls(hModule);
    return TRUE;
}