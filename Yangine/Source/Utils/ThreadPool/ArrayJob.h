#pragma once
#include "ThreadPool.h"
#include <vector>
#include <future>

namespace yang
{

    template <class ContainerT>
    class ArrayJob
    {
    public:
        template <class F>
        ArrayJob(ContainerT& container, F&& func, ThreadPool& threadPool, size_t numJobs);

        void WaitFor();
    private:
        std::vector<std::future<void>> m_results;

    };

    template<class ContainerT>
    inline void ArrayJob<ContainerT>::WaitFor()
    {
        for (auto& res : m_results)
        {
            res.get();
        }
    }

    template<class ContainerT>
    template<class F>
    inline ArrayJob<ContainerT>::ArrayJob(ContainerT& container, F&& func, ThreadPool& threadPool, size_t numJobs)
    {
        m_results.reserve(numJobs);

        size_t jobSize = container.size() / numJobs;
        for (size_t i = 0; i < numJobs; ++i)
        {
            size_t startIndex = i * jobSize;
            size_t endIndex = (i == numJobs - 1 ? container.size() : (i + 1) * jobSize);
            m_results.emplace_back(threadPool.enqueue([startIndex, endIndex, &container, &func]()
                {
                    for (size_t i = startIndex; i < endIndex; ++i)
                    {
                        func(i, container[i]);
                    }
                }));
        }
    }

}