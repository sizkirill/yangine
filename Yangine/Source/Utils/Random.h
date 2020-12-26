#pragma once
#include <stdint.h>
#include <cassert>

namespace yang
{
class Random
{
public:
    Random(uint64_t seed);
    Random();

    template<typename Integer>
    Integer Rand(Integer start, Integer end);

    template<typename Integer>
    Integer Rand(Integer max);

    template<typename FloatingPoint>
    FloatingPoint FRand(FloatingPoint start, FloatingPoint end);

    template<typename FloatingPoint>
    FloatingPoint FRand(FloatingPoint max);

    template<typename FloatingPoint>
    FloatingPoint FRand();

    static Random GlobalRNG;
private:
    uint64_t m_state;

    uint64_t GetNext();
};

template<typename Integer>
inline Integer Random::Rand(Integer start, Integer end)
{
    static_assert(std::is_integral_v<Integer>, "Type must be integral");
    return start + static_cast<Integer>(GetNext()) % (end - start);
}

template<typename Integer>
inline Integer Random::Rand(Integer max)
{
    static_assert(std::is_integral_v<Integer>, "Type must be integral");
    return Rand(static_cast<Integer>(0), max);
}

template<typename FloatingPoint>
inline FloatingPoint Random::FRand(FloatingPoint start, FloatingPoint end)
{
    static_assert(std::is_floating_point_v<FloatingPoint>, "Type must be floating point");

    assert(end > start);

    return start + FRand<FloatingPoint>() * (end - start);
}

template<typename FloatingPoint>
inline FloatingPoint Random::FRand(FloatingPoint max)
{
    static_assert(std::is_floating_point_v<FloatingPoint>, "Type must be floating point");
    return FRand<FloatingPoint>() * max;
}

template<typename FloatingPoint>
inline FloatingPoint Random::FRand()
{
    static_assert(std::is_floating_point_v<FloatingPoint>, "Type must be floating point");
    return static_cast<FloatingPoint>(GetNext()) / std::numeric_limits<uint64_t>::max();
}

}
