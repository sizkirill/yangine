#pragma once
#include <thread>

namespace yang
{
    static constexpr size_t kWindowWidth = 1280;
    static constexpr size_t kWindowHeight = 720;
    static const size_t kNumThreads = std::thread::hardware_concurrency() - 1;
}