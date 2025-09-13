// assembly.h
/*
#pragma once

extern void* OriginalUpdateHeatLevel;

void __declspec(naked) HookedUpdateHeatLevel() {
    __asm {
        pushad

        mov eax, [0091D3F0h]        // Valore CTS
        mov ecx, [esi + 0x104]      // Heat level corrente

        mov ebx, 1

        check10:
            cmp eax, 5000000
            jl check9
            mov ebx, 10
            jmp done

        check9:
            cmp eax, 2000000
            jl check8
            cmp ecx, 9
            jge check8
            mov ebx, 9
            jmp done

        check8:
            cmp eax, 1000000
            jl check7
            cmp ecx, 8
            jge check7
            mov ebx, 8
            jmp done

        check7:
            cmp eax, 500000
            jl check6
            cmp ecx, 7
            jge check6
            mov ebx, 7
            jmp done

        check6:
            cmp eax, 300000
            jl check5
            cmp ecx, 6
            jge check5
            mov ebx, 6
            jmp done

        check5:
            cmp eax, 100000
            jl check4
            cmp ecx, 5
            jge check4
            mov ebx, 5
            jmp done

        check4:
            cmp eax, 50000
            jl check3
            cmp ecx, 4
            jge check3
            mov ebx, 4
            jmp done

        check3:
            cmp eax, 15000
            jl check2
            cmp ecx, 3
            jge check2
            mov ebx, 3
            jmp done

        check2:
            cmp eax, 5000
            jl done
            cmp ecx, 2
            jge done
            mov ebx, 2

        done:
            mov[esi + 0x104], ebx
            popad
            mov eax, ebx
            // Salto alla funzione originale
            jmp OriginalUpdateHeatLevel
    }
}
*/
// helpers.h
#pragma once

#include <vector>

constexpr uintptr_t baseAddress = 0x400000;
static constexpr uintptr_t CTSAddress = 0x0091D3F0;

struct HeatThreshold {
    int cts;
    unsigned long long heatLevel;
};

std::vector<HeatThreshold> heatsThresholds {
    {5000000, 10},
    {2000000, 9},
    {1000000, 8},
    {500000, 7},
    {300000, 6},
    {100000, 5},
    {50000, 4},
    {15000, 3},
    {5000, 2}
};

uintptr_t getHeatLevelAddress() {
    uintptr_t baseAddress = (uintptr_t)GetModuleHandleA("speed.exe");
    uintptr_t address = baseAddress + 0x00593CC8;

    uintptr_t ptr = *(uintptr_t*)address;

    uintptr_t ptrToHeatLevel = ptr + 0x104;

    return ptrToHeatLevel;
}

unsigned long long calculateNewHeatLevel() {
    int currentCTSValue = *(int*)CTSAddress;

    int currentHeatLevelValue = *(int*)getHeatLevelAddress();

    for (int i = 0; i < heatsThresholds.size(); ++i) {
        if (currentCTSValue >= heatsThresholds[i].cts && currentHeatLevelValue < heatsThresholds[i].heatLevel) {
            return heatsThresholds[i].heatLevel;
        }
    }

    return 1;
}
/*
void __declspec(naked) NewFunction() {
    __asm {
        pushfd
        pushad

        call calculateNewHeatLevel
        mov ebx, eax
        call getHeatLevelAddress

        ; eax now holds the stable pointer to heat level
        ; ebx holds the heat level calculated with the CTS amount logic

        mov [eax], ebx ; in the address that eax is holding, insert the value ebx is holding ([ptrToHeatLevel] <- newHeatLevel)

        popad
        popfd

        jmp [NextGameInstruction]
    }
}
*/
/*
void __declspec(naked) NewFunction() {
    __asm {
        pushfd
        pushad
        ; Variables
        CTSAmount dd [CTSAddress]
        HeatLevel dd ?

        mov HeatLevel, eax
        
        mov eax, [CTSAddress] ;  CTS Value
        mov ecx, [esi + 0x104] ; Current heat level

        mov ebx, 1

        check10:
            cmp eax, 5000000
            jl check9
            mov ebx, 10
            jmp done

        check9:
            cmp eax, 2000000
            jl check8
            cmp ecx, 9
            jge check8
            mov ebx, 9
            jmp done

        check8:
            cmp eax, 1000000
            jl check7
            cmp ecx, 8
            jge check7
            mov ebx, 8
            jmp done

        check7:
            cmp eax, 500000
            jl check6
            cmp ecx, 7
            jge check6
            mov ebx, 7
            jmp done

        check6:
            cmp eax, 300000
            jl check5
            cmp ecx, 6
            jge check5
            mov ebx, 6
            jmp done

        check5:
            cmp eax, 100000
            jl check4
            cmp ecx, 5
            jge check4
            mov ebx, 5
            jmp done

        check4:
            cmp eax, 50000
            jl check3
            cmp ecx, 4
            jge check3
            mov ebx, 4
            jmp done

        check3:
            cmp eax, 15000
            jl check2
            cmp ecx, 3
            jge check2
            mov ebx, 3
            jmp done

        check2:
            cmp eax, 5000
            jl done
            cmp ecx, 2
            jge done
            mov ebx, 2

        done:
            mov[esi + 0x104], ebx
            popad
            popfd
            mov eax, ebx
            
            ; Salto alla funzione originale
            jmp [OriginalGameFunction]
    }
}
*/