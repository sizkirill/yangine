#include "Random.h"
#include <random>

yang::Random yang::Random::GlobalRNG = yang::Random();

yang::Random::Random(uint64_t seed)
    :m_state(seed)
{
}

yang::Random::Random()
{
    std::random_device rd;
    m_state = rd();
}

uint64_t yang::Random::GetNext()
{
    uint64_t x = m_state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    m_state = x;
    return m_state;
}
