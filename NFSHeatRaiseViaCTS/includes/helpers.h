#pragma once

#include <windows.h>
#include <cstring>
#include <cstdint>
#include <variant>

constexpr uint8_t NOP_OPCODE = 0x90;
constexpr uint8_t JMP_OPCODE = 0xE9;

constexpr size_t JMP_SIZE = 5;

using MemoryPtr = std::variant<uintptr_t, void*>;

/// Writes a value of type T to the specified memory address
template<typename T>
inline void writeMemory(uintptr_t address, const T& value) {
    const SIZE_T len = sizeof(T);

    DWORD oldProtect;

    if (!VirtualProtect(reinterpret_cast<void*>(address), len, PAGE_EXECUTE_READWRITE, &oldProtect))
        return;

    // Write the value directly
    std::memcpy(reinterpret_cast<void*>(address), &value, len);

    DWORD temp;

    VirtualProtect(reinterpret_cast<void*>(address), len, oldProtect, &temp);
}

/// Reads a value of type T from the specified memory address
template<typename T>
inline T readMemory(uintptr_t address) {
    // Read the value directly
    return *reinterpret_cast<T*>(address);
}

// Writes a number of NOP instructions to an address
inline void makeNOP(void* address, size_t count = 1) {
    DWORD oldProtect;

    // Make the page writable
    if (!VirtualProtect(address, count, PAGE_EXECUTE_READWRITE, &oldProtect))
        return;

    // Write 0x90 directly
    std::memset(address, NOP_OPCODE, count);

    DWORD temp;

    // Restore original protection
    VirtualProtect(address, count, oldProtect, &temp);
}

// Creates a relative jump from an address to another one
inline void makeJMP(uintptr_t addressAt, MemoryPtr addressDest) {
    uint8_t patch[JMP_SIZE];
    patch[0] = JMP_OPCODE;

    uintptr_t addressDestCasted;

    if (std::holds_alternative<uintptr_t>(addressDest))
        addressDestCasted = std::get<uintptr_t>(addressDest);
    else
        addressDestCasted = reinterpret_cast<uintptr_t>(std::get<void*>(addressDest));
                                  
    int32_t relativeOffset = static_cast<int32_t>(addressDestCasted - (addressAt + JMP_SIZE));

    // Fills the empty part of the array 'patch' with the value of the relative offset
    std::memcpy(&patch[1], &relativeOffset, sizeof(relativeOffset));

    DWORD oldProtect;

    if (!VirtualProtect(reinterpret_cast<void*>(addressAt), JMP_SIZE, PAGE_EXECUTE_READWRITE, &oldProtect))
        return; // Could optionally log or return a bool

    std::memcpy(reinterpret_cast<void*>(addressAt), patch, JMP_SIZE); // Patches

    DWORD temp;
    VirtualProtect(reinterpret_cast<void*>(addressAt), JMP_SIZE, oldProtect, &temp);
}