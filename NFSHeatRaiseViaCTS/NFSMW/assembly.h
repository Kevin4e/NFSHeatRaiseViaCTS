#pragma once

#include <windows.h>
#include "../includes/settings.h"

constexpr DWORD HeatLevelsCodeCaveExit = 0x443DC9;

void __declspec(naked) HeatLevelsCodeCave()
{
    __asm
    {
        mov ebx, MaxHeatLevel
        mov [esi + 0xE0], ebx
        mov ebx, MinHeatLevel
        mov [esi + 0xDC], ebx
        mov edx, [esi + 0xE0]
        jmp HeatLevelsCodeCaveExit
    }
}