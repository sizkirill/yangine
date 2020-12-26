#pragma once
#include <stdint.h>

// Constexpr string hashing from:
// https://gist.github.com/underscorediscovery/81308642d0325fd386237cfa3b44785c#file-hash_fnv1a_constexpr-h
// With some changes to naming according to GAP coding standards

// FNV1a c++11 constexpr compile time hash functions, 32 and 64 bit
// str should be a null terminated string literal, value should be left out 
// e.g hash_32_fnv1a_const("example")
// code license: public domain or equivalent
// post: https://notes.underscorediscovery.com/constexpr-fnv1a/

constexpr uint32_t kVal32 = 0x811c9dc5;
constexpr uint32_t kPrime32 = 0x1000193;
constexpr uint64_t kVal64 = 0xcbf29ce484222325;
constexpr uint64_t kPrime64 = 0x100000001b3;

inline constexpr uint32_t StringHash32(const char* const str, const uint32_t value = kVal32) noexcept {
    return (str[0] == '\0') ? value : StringHash32(&str[1], (value ^ uint32_t(str[0])) * kPrime32);
}

inline constexpr uint64_t StringHash64(const char* const str, const uint64_t value = kVal64) noexcept {
    return (str[0] == '\0') ? value : StringHash64(&str[1], (value ^ uint64_t(str[0])) * kPrime64);
}
