#pragma once

#include "settings.h"

constexpr DWORD loc_71D316 = 0x71D316; // address where the call instruction that calculates the heat level resides
constexpr DWORD sub_7C4B80 = 0x7C4B80; // address where the prologue of the original heat level calculation function resides
constexpr DWORD loc_71D31B = 0x71D31B; // address where the value calculated is stored at the heat level address
constexpr DWORD loc_71D321 = 0x71D321; // address where the game proceeds after storing the calculated heat level

void __declspec(naked) HeatLevelAbsoluteCalculationDetour()
{
    __asm
    {
        push ebx
        push ecx
        push edx
        push esi
        push edi

        mov ebx, [0x91D3F0]   // ebx = CTS amount

        mov edx, 0x400000
        add edx, 0x593CC8
        mov edx, [edx]

        cmp edx, 0
        je CallOriginal       // pointer not ready, skip

        add edx, 0x104        // edx = heat level address

        mov ecx, 9

        // preparing raw memory offsets on indexes registers
        mov esi, OFFSET heatsThresholds       // esi = &(heatsThresholds[0].heatLevel)
        add esi, 4                            // esi = &(heatsThresholds[0].cts)
        add esi, 64                           // esi = &(heatsThresholds[8].cts)

        mov edi, OFFSET heatsThresholds       // edi = &(heatsThresholds[0].heatLevel)
        add edi, 64                           // edi = &(heatsThresholds[8].heatLevel)

        mov eax, 1 // Fallback if none of the conditions are met
        push eax
        mov eax, [edx]

    ForLoop:
        cmp ebx, [esi]
        jb GoToNextLoop

        cmp eax, [edi]
        jb Exit
            
        GoToNextLoop:
            sub esi, 8
            sub edi, 8

        loop ForLoop

    Exit:
        pop eax
        mov eax, [edi]
        mov [edx], eax

        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx

        jmp loc_71D321

    CallOriginal:
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx

        call sub_7C4B80
    }
}

void __declspec(naked) HeatLevelAbsoluteCalculationDetour_NO_PD() {
    __asm
    {
        push ebx
        push ecx
        push edx
        push esi
        push edi

        mov ebx, [0x91D3F0]   // ebx = CTS amount

        mov edx, 0x400000
        add edx, 0x593CC8
        mov edx, [edx]

        cmp edx, 0
        je CallOriginal       // pointer not ready, skip

        add edx, 0x104        // edx = heat level address

        mov ecx, 9

        // preparing raw memory offsets on indexes registers
        mov esi, OFFSET heatsThresholds       // esi = &(heatsThresholds[0].heatLevel)
        add esi, 4                            // esi = &(heatsThresholds[0].cts)
        add esi, 64                           // esi = &(heatsThresholds[8].cts)

        mov edi, OFFSET heatsThresholds       // edi = &(heatsThresholds[0].heatLevel)
        add edi, 64                           // edi = &(heatsThresholds[8].heatLevel)

        mov eax, 1 // Fallback if none of the conditions are met

    ForLoop:
        cmp ebx, [esi]
        jae Exit

        sub esi, 8
        sub edi, 8
        loop ForLoop

    Exit:
        mov eax, [edi]
        mov [edx], eax

        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx

        jmp loc_71D321

    CallOriginal:
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx

        call sub_7C4B80
    }
}

void __declspec(naked) HeatLevelProgressiveCalculationDetour()
{
    __asm
    {
        push ebx
        push ecx
        push edx
        push esi
        push edi

        mov ebx, [0x91D3F0]   // ebx = CTS amount

        mov edx, 0x400000
        add edx, 0x593CC8
        mov edx, [edx]

        cmp edx, 0
        je CallOriginal       // pointer not ready, skip

        add edx, 0x104        // edx = heat level address

        mov ecx, 9

        // preparing raw memory offsets on indexes registers
        mov esi, OFFSET heatsThresholds       // esi = &(heatsThresholds[0].heatLevel)
        add esi, 4                            // esi = &(heatsThresholds[0].cts)

        mov edi, OFFSET heatsThresholds       // edi = &(heatsThresholds[0].heatLevel)

        mov eax, 10 // Fallback if none of the conditions are met
        push eax
		mov eax, [edx]

    ForLoop:
        cmp ebx, [esi]
        jae GoToNextLoop

        cmp eax, [edi]
        jb Exit

        GoToNextLoop:
            add esi, 8
            add edi, 8

        loop ForLoop

    Exit:
		pop eax
        mov eax, [edi]
        dec eax
        mov [edx], eax

        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx

        jmp loc_71D321

    CallOriginal:
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx

        call sub_7C4B80
    }
}

void __declspec(naked) HeatLevelProgressiveCalculationDetour_NO_PD() {
    __asm
    {
        push ebx
        push ecx
        push edx
        push esi
        push edi

        mov ebx, [0x91D3F0]   // ebx = CTS amount

        mov edx, 0x400000
        add edx, 0x593CC8
        mov edx, [edx]

        cmp edx, 0
        je CallOriginal       // pointer not ready, skip

        add edx, 0x104        // edx = heat level address

        mov ecx, 9

        // preparing raw memory offsets on indexes registers
        mov esi, OFFSET heatsThresholds       // esi = &(heatsThresholds[0].heatLevel)
        add esi, 4                            // esi = &(heatsThresholds[0].cts)

        mov edi, OFFSET heatsThresholds       // edi = &(heatsThresholds[0].heatLevel)

        mov eax, 10 // Fallback if none of the conditions are met

    ForLoop:
        cmp ebx, [esi]
        jb Exit
        
        add esi, 8
        add edi, 8
        loop ForLoop

    Exit:
        mov eax, [edi]
        dec eax
        mov [edx], eax

        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx

        jmp loc_71D321

    CallOriginal:
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx

        call sub_7C4B80
    }
}

void __declspec(naked) HeatLevelCumulativeCalculationDetour() {
    __asm {
        push ebx
        push ecx
        push edx
        push esi
        push edi

        mov ebx, [0x91D3F0]   // ebx = CTS amount

        mov edx, 0x400000
        add edx, 0x593CC8
        mov edx, [edx]

        cmp edx, 0
        je CallOriginal       // pointer not ready, skip

        add edx, 0x104        // edx = heat level address

        mov ecx, 9

        // preparing raw memory offsets on indexes registers
        mov esi, OFFSET heatsThresholds       // esi = &(heatsThresholds[0].heatLevel)
        add esi, 4                            // esi = &(heatsThresholds[0].cts)

        mov edi, OFFSET heatsThresholds       // edi = &(heatsThresholds[0].heatLevel) 

        mov eax, 1 // Fallback if none of the conditions are met

    ForLoop:
        cmp ebx, esi
        jb SkipAssign

        mov eax, edi
       
        SkipAssign:
            add esi, 8
            add edi, 8

        loop ForLoop

    Exit:
        mov [edx], eax

        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx

        jmp loc_71D321

    CallOriginal:
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx

        call sub_7C4B80
    }
}