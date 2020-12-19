#include "ApplicationGlobals.h"
#include <Utils/ThreadPool/ThreadPool.h>
#include "ApplicationConstants.h"
#include "Graphics/Viewport.h"

using namespace yang;

ThreadPool& yang::GetThreadPool()
{
    static ThreadPool g_threadPool(kNumThreads);

    return g_threadPool;
}

Viewport& yang::GetGlobalViewport()
{
    static Viewport g_viewport;
    return g_viewport;
}
