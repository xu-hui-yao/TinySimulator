#pragma once

#include <cstdint>
#include <cstddef>

struct float3 {
    float x, y, z;
};

struct float4 {
    float x, y, z, w;
};

struct int3 {
    int x, y, z;
};

constexpr std::uint32_t fnv1a_32(char const *s, size_t count) { // NOLINT(*-no-recursion)
    return ((count ? fnv1a_32(s, count - 1) : 2166136261u) ^ s[count]) * 16777619u;
}

constexpr std::uint32_t operator"" _hash(char const *s, size_t count) { return fnv1a_32(s, count); }
