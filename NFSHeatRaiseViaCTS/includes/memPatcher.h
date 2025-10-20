#pragma once

/*
 *  Personal memory patching utilities for Windows only.
 *  Author: Kevin4e
 * 
 *  Modifies memory permissions protections with Virtual Protect, restoring them afterwards.
 *  
 *  All functions guarantee 32-bit and 64-bit compatibility.
 * 
 *  The caller must ensure the addresses passed to these functions are valid.
 */ 

#include <windows.h>
#include <cstring>
#include <cstdint>
#include <variant>

namespace MemPatcher {
    enum Result {
        Success,
		ProtectionChangeFailed,
		ReadFailed,
        InvalidRange,
        TooFarJumpDistance
    };

    namespace Details {
        constexpr uint8_t NOP_OPCODE = 0x90;
        constexpr uint8_t JMP_OPCODE = 0xE9;
		constexpr uint8_t INDIRECT_CALL_OPCODE = 0xE8;

        constexpr size_t JMP_SIZE = 5;
		constexpr size_t CALL_SIZE = 5;

        using AddressOrPtr = std::variant<uintptr_t, void*>;
    }

    using namespace Details;

    // Writes a value of type T to the specified memory address
    template<typename T>
    inline Result writeMemory(uintptr_t address, T value) noexcept {
        const size_t len = sizeof(T);

        DWORD oldProtect;

		if (!VirtualProtect(reinterpret_cast<void*>(address), len, PAGE_EXECUTE_READWRITE, &oldProtect)) // Make the page writable by changing its protection
            return ProtectionChangeFailed; // Couldn't change protection

        // Write the value directly
        *reinterpret_cast<T*>(address) = value;

        // Flush instruction cache to ensure CPU fetches the updated instructions, recommended after writing new bytes into code memory
        FlushInstructionCache(GetCurrentProcess(), reinterpret_cast<void*>(address), len);
        
        DWORD temp;
		VirtualProtect(reinterpret_cast<void*>(address), len, oldProtect, &temp); // Restore original protection

        return Success;
    }

    // Reads a value of type T from the specified memory address
    template<typename T>
    inline T readMemory(uintptr_t address, Result* result = nullptr) noexcept {
        __try {
            if (result) *result = Success;
            return *reinterpret_cast<T*>(address); // Read the value directly and return it
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            if (result) *result = ReadFailed;
            return T{};
        }
    }

    // Writes a number of NOP instructions to an address
    inline Result makeNOP(uintptr_t address, size_t count = 1) noexcept {
        DWORD oldProtect;

        if (!VirtualProtect(reinterpret_cast<void*>(address), count, PAGE_EXECUTE_READWRITE, &oldProtect)) // Make the page writable by changing its protection
			return ProtectionChangeFailed; // Couldn't change protection

        // Write 0x90 directly
        std::memset(reinterpret_cast<void*>(address), NOP_OPCODE, count);

        // Flush instruction cache to ensure CPU fetches the updated instructions, recommended after writing new bytes into code memory
		FlushInstructionCache(GetCurrentProcess(), reinterpret_cast<void*>(address), count); 

        DWORD temp;
        VirtualProtect(reinterpret_cast<void*>(address), count, oldProtect, &temp); // Restore original protection

		return Success;
    }

    // Creates a relative jump from an address to another one
	// The distance between the two addresses must be within +/- 2GB
    inline Result makeJMP(uintptr_t addressAt, const AddressOrPtr& addressDest) noexcept {
        uint8_t patch[JMP_SIZE];
        patch[0] = JMP_OPCODE;

        uint32_t addressDestCasted;

        if (std::holds_alternative<uintptr_t>(addressDest))
            addressDestCasted = std::get<uintptr_t>(addressDest);
        else
            addressDestCasted = reinterpret_cast<uintptr_t>(std::get<void*>(addressDest));

		uint32_t distance = addressDestCasted - addressAt - JMP_SIZE;

        if (distance > INT32_MAX)
			return TooFarJumpDistance; // Addresses are too far apart for a relative jump

        int32_t relativeOffset = static_cast<int32_t>(distance);

        // Fills the empty part of the array 'patch' with the value of the relative offset
        std::memcpy(&patch[1], &relativeOffset, sizeof(relativeOffset));

        DWORD oldProtect;

        if (!VirtualProtect(reinterpret_cast<void*>(addressAt), JMP_SIZE, PAGE_EXECUTE_READWRITE, &oldProtect)) // Make the page writable by changing its protection
            return ProtectionChangeFailed; // Couldn't change protection

        std::memcpy(reinterpret_cast<void*>(addressAt), patch, JMP_SIZE); // Patches

        // Flush instruction cache to ensure CPU fetches the updated instructions, recommended after writing new bytes into code memory
        FlushInstructionCache(GetCurrentProcess(), reinterpret_cast<void*>(addressAt), JMP_SIZE);

        DWORD temp;
        VirtualProtect(reinterpret_cast<void*>(addressAt), JMP_SIZE, oldProtect, &temp); // Restore original protection
        
		return Success;
    }

	// Fills a memory region with NOP instructions from an address to another one (inclusive)
    // By default, the end address is included; set inclusive = false to exclude it.
    inline Result fillNOPs(uintptr_t addressStart, uintptr_t addressEnd, bool inclusive = true) noexcept {
        if (addressEnd < addressStart)
			return InvalidRange; // Invalid range

		size_t totalBytes = addressEnd - addressStart;
        if (inclusive) ++totalBytes;

		return makeNOP(addressStart, totalBytes);
	}
}