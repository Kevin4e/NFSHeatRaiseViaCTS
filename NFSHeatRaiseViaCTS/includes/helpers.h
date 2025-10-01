#pragma once

#include <cstdint>

/// Reads a value of type T from the specified memory address
template<typename T>
inline T readMemory(std::uintptr_t address) {
    return *reinterpret_cast<T*>(address);
}