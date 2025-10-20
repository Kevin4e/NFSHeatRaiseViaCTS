#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <cstdint>
#include <algorithm>
#include <fstream>
#include <string>
#include <thread>

#include "../includes/iniReader.hpp"
#include "../includes/heatLevelCalculations.h"
#include "../includes/settings.h"
#include "../includes/memPatcher.h"
#include "../includes/logger.hpp"

#include "assembly.h"

constexpr DWORD setHeatLevelAddress = 0x00612660;  // Instruction address of the instruction that sets the heat level globally. Thanks to ExOpts Team for finding this
constexpr DWORD baseAddress = 0x00400000;          // 'speed.exe' base address
constexpr DWORD pursuitFlagAddress = 0x0092FD34;   // Boolean flag (0 or 1) indicating if player is in a pursuit
constexpr DWORD ctsAddress = 0x0091D3F0;           // CTS value address
constexpr DWORD openWorldFlagAddress = 0x0092D884; // Boolean flag (0 or 1) indicating if player is in the open world

// Heat level stable pointer offsets
constexpr DWORD moduleOffset = 0x00593CC8;         // Module offset
constexpr DWORD finalOffset = 0x00000104;          // Final offset

void (*const setHeat)(float) = reinterpret_cast<void(*)(float)>(setHeatLevelAddress); // Function pointer to set heat level. 

float (*HeatCalcFunc)(uint32_t, uint32_t) = nullptr; // Function pointer to one of the heat level calculation ones

constexpr const char* readableIniFiles[] = {
    "NFSHeatRaiseViaCTSConfiguration.ini",
    "NFSMWHeatRaiseViaCTSConfiguration.ini"
};

void MainThread() {
    while (!MemPatcher::readMemory<uint32_t>(openWorldFlagAddress)) // Wait until we're in the open world once to get heat level pointers
        Sleep(100);

    const DWORD ptrToHeatLevel = MemPatcher::readMemory<DWORD>(baseAddress + moduleOffset) + finalOffset; // Address of integer heat level
	const DWORD ptrToHeatLevelFloat = MemPatcher::readMemory<DWORD>(MemPatcher::readMemory<DWORD>(MemPatcher::readMemory<DWORD>(0x9352B0) + 0x14) + 0x24) + 0x1C; // Address of decimal heat level (HUD)

	uint32_t previousCTSValue = 0;

    while (true) {
        if (MemPatcher::readMemory<uint32_t>(pursuitFlagAddress)) { // if in pursuit

            while (MemPatcher::readMemory<uint32_t>(ptrToHeatLevel) == 0) // wait until heat level (integer) is initialized
                Sleep(30);

			float tmp = MemPatcher::readMemory<float>(ptrToHeatLevelFloat);

            if (tmp < MinHeatLevel) // if heat level is below minimum
                setHeat(MinHeatLevel); // set heat level to minimum

			else if (std::truncf(tmp) < MinHeatLevel) // if truncating the decimal part makes heat level go below minimum
                setHeat(MinHeatLevel); // set heat level to minimum

            else
                MemPatcher::writeMemory<float>(ptrToHeatLevelFloat, std::truncf(tmp)); // trunc decimal part of the heat level in the HUD

            while (MemPatcher::readMemory<uint32_t>(pursuitFlagAddress)) {
                const uint32_t currentCTSValue = MemPatcher::readMemory<uint32_t>(ctsAddress);
                const uint32_t currentHeatLevelValue = MemPatcher::readMemory<uint32_t>(ptrToHeatLevel);

                if (previousCTSValue != currentCTSValue) {
                    previousCTSValue = currentCTSValue;

                    const float newHeatLevelCalculated = HeatCalcFunc(currentCTSValue, currentHeatLevelValue);

                    if (newHeatLevelCalculated != currentHeatLevelValue) {
						setHeat(newHeatLevelCalculated); // Make sure to set a new heat level only if CTS and heat level calculation changed
                    }
                }

                Sleep(30);
            }
        }

        Sleep(100);
    }
}

// Read INI, configure everything, start MainThread()
void Setup() {

    // Find the iterator that points to the first ini file that exists
    const auto it = std::find_if(std::begin(readableIniFiles), std::end(readableIniFiles), [](const auto& ini) { return std::ifstream(ini).good(); });

    if (it == std::end(readableIniFiles))
        return; // End if no readable .ini file exists

    // INI reader object
    INIReader iniReader(*it);
    
	// Logger object
    Logger logger("NFSMWHeatRaiseViaCTSDebug.log");

    // Read values from the INI
    {
        Enable = iniReader.read<int32_t>("Enable", 0);

        if (!Enable) {
            logger.log("NFSMW Heat Raise Via CTS script disabled. Exiting...");
            return;
        }

        CalculationMode = iniReader.read<int32_t>("CalculationMode", 0);

        if (CalculationMode < 0 || CalculationMode > 2) {
            logger.log("CalculationMode value is out of bounds (0-2). Exiting...");
            return;
        }

        PreventLowerHeat = iniReader.read<int32_t>("PreventLowerHeat", 0);

        MaxHeatLevel = iniReader.read<float>("MaxHeatLevel", 10.0f);
        MinHeatLevel = iniReader.read<float>("MinHeatLevel", 1.0f);

        if (MaxHeatLevel < 1.0f || MaxHeatLevel > 10.0f || (MinHeatLevel > MaxHeatLevel)) {
            logger.log("Heat level limits are out of bounds or minimum is greater than maximum. Exiting...");
            return;
        }

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

        MemPatcher::makeNOP(0x409326, 3); // NOP out the instruction that updates the decimal part of the heat level in the HUD

        /*
         *  Respectively:
         *
         *   - Prepares the game for Ultimate force heat level hack;
         *   - Sets the max heat level recognizable by Front-End
         *
         *  Credits to ExOpts Team
         */

        MemPatcher::makeJMP(0x443DC3, HeatLevelsCodeCave);

        // CustomizeMeter::Init
        MemPatcher::writeMemory<float>(0x7BB502, MaxHeatLevel); // CustomizeCategoryScreen
        MemPatcher::writeMemory<float>(0x7B1387, MaxHeatLevel); // CustomizationScreenHelper
        MemPatcher::writeMemory<float>(0x7B0C89, MaxHeatLevel); // CustomizeShoppingCart::Setup
        MemPatcher::writeMemory<float>(0x7B4D7C, MaxHeatLevel); // UIQRCarSelect::InitStatsSliders

        // SetHeatLevel
        MemPatcher::writeMemory<float*>(0x435079, &MaxHeatLevel); // AIVehicleHuman::~AIVehicleHuman
        MemPatcher::writeMemory<float>(0x435088, MaxHeatLevel);

        // Safehouse car select icon stuff (HEAT_X%.0f)
        MemPatcher::writeMemory<float*>(0x7A5B03, &MaxHeatLevel);
        MemPatcher::writeMemory<float*>(0x7A5B12, &MaxHeatLevel);
    }

	logger.log("NFSMW Heat Raise Via CTS script initialized successfully.");

    std::thread(MainThread).detach();
}

extern "C" __declspec(dllexport) void InitializeASI() {
    // Check if .exe file is compatible - Thanks to thelink2012 and MWisBest
    // A few tweaks and simplified condition for clarity; logic unchanged, there were a few redundant operations

    IMAGE_NT_HEADERS* nt = reinterpret_cast<IMAGE_NT_HEADERS*>(0x400108);

    if (nt->OptionalHeader.AddressOfEntryPoint == 0x3C4040)
        Setup();

    else
        MessageBoxA(nullptr, "This .exe is not supported.\nPlease use v1.3 speed.exe (5.75 MB (6.029.312 bytes)).", "NFSMW Heat Raise via CTS by Kevin4e", MB_ICONERROR);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID /*lpReserved*/) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
        DisableThreadLibraryCalls(hModule);

    return TRUE;
}