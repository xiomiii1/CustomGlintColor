#pragma once
#include <cstdint>
namespace glintmod::sdk {
template <typename T> inline T& field(void* base, std::size_t offset) {
    return *reinterpret_cast<T*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
template <typename T> inline const T& field(const void* base, std::size_t offset) {
    return *reinterpret_cast<const T*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
}
