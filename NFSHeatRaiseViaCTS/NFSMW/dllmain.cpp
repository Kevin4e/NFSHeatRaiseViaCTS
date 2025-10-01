#include <windows.h>
#include <string>
#include <cstdint>
#include <filesystem>
#include <array>

#include "../includes/iniReader.hpp"
#include "../includes/heatLevelCalculations.h"
#include "../includes/settings.h"
#include "../includes/helpers.h"

constexpr DWORD setHeatLevelAddress = 0x00612660;  // Instruction address of the instruction that sets the heat level globally. Thanks to ExOpts Team for finding this
constexpr DWORD baseAddress = 0x00400000;          // 'speed.exe' base address
constexpr DWORD pursuitFlagAddress = 0x0092FD34;   // Boolean flag (0 or 1) indicating if player is in a pursuit
constexpr DWORD ctsAddress = 0x0091D3F0;           // CTS value address
constexpr DWORD gameStateAddress = 0x00925E90;     // Game state address (3 = Front-End, 4 & 5 = Loading Screen, 6 = Free Roam)

void (*setHeat)(float) = reinterpret_cast<void(*)(float)>(setHeatLevelAddress); // Function pointer to set heat level. 

constexpr std::array<const char*, 2> readableIniFiles = {
    "NFSHeatRaiseViaCTSThresholds.ini",
    "NFSMWHeatRaiseViaCTSThresholds.ini"
};

DWORD WINAPI MainThread(LPVOID) {
    // Find the iterator that points to the first ini file that exists
    auto it = std::find_if(readableIniFiles.begin(), readableIniFiles.end(), [](const auto& ini) { return std::filesystem::exists(ini); });

    if (it == readableIniFiles.end())
        return 1; // End if no readable .ini exists

    // Ini reader object
    IniReader iniReader(*it);

    // Read values from .ini

    Enable = iniReader.read<int32_t>("Enable", 1);

    if (!Enable)
        return 1;

    CalculationMode = iniReader.read<std::string>("CalculationMode", "Progressive");

    float (*HeatCalcFunc)(uint32_t, float) = nullptr; // Function pointer to one of the heat level calculation ones

    if (CalculationMode == "progressive") {
        HeatCalcFunc = HeatLevelProgressiveCalculationDetour;
    }
    else if (CalculationMode == "absolute") {
        HeatCalcFunc = HeatLevelAbsoluteCalculationDetour;
    }
    else if (CalculationMode == "cumulative") {
        std::sort(heatsThresholds, heatsThresholds + 9, [](const HeatThreshold& a, const HeatThreshold& b) {
            return a.cts < b.cts;
        });

        HeatCalcFunc = HeatLevelCumulativeCalculationDetour;
    }
    else
        return 1;

    PreventLowerHeat = iniReader.read<int32_t>("PreventLowerHeat", 1);
    
    for (int i = 0; i < 9; ++i)
        heatsThresholds[i].cts = iniReader.read<uint32_t>("ThresholdForHeat" + std::to_string(i + 2), heatsThresholds[i].cts);

    while (readMemory<uint32_t>(gameStateAddress) != 6) { // Wait until we're in free roam once to prepare variables/pointers
        Sleep(1000);
    } 

    float newHeatLevel;

    uint32_t currentCTSValue;
    float currentHeatLevelValue;

    // Prepare heatlevel address pointer
    DWORD ptr = readMemory<DWORD>(baseAddress + 0x0052D378);
    ptr = readMemory<DWORD>(ptr + 0x1C);
    ptr = readMemory<DWORD>(ptr + 0x4);
    ptr = readMemory<DWORD>(ptr + 0x24);
    ptr += 0x1C;

    DWORD ptrToHeatLevel = ptr;

    /*
    * Known issues:
    * 
    * 1. Heat oscillation:
    *    - When the current heat is 3 and a higher heat is calculated every frame,
    *      the value alternates between 3 and the new heat level.
    * 
    * 2. Instability with ExOpts' HeatLevelHack:
    *    - If the heat level is manually changed with the F7 key and a pursuit starts,
    *      the value fluctuates between 0 and 10 with random decimal points
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

extern "C" __declspec(dllexport) void InitializeASI() {
    // Check if .exe file is compatible - Thanks to thelink2012 and MWisBest
    // Simplified condition for clarity; logic unchanged, there was a redundant operation

    uintptr_t base = (uintptr_t)GetModuleHandleA(nullptr);
    IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)(base);
    IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)(base + dos->e_lfanew);

    if ((nt->OptionalHeader.AddressOfEntryPoint + 0x400000) == 0x7C4040) // Check if .exe file is compatible - Thanks to thelink2012 and MWisBest
        CreateThread(nullptr, 0, MainThread, nullptr, 0, nullptr);

    else
        MessageBoxA(nullptr, "This .exe is not supported.\nPlease use v1.3 speed.exe (5.75 MB (6.029.312 bytes)).", "NFSMW Heat Raise via CTS by Kevin4e", MB_ICONERROR);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}