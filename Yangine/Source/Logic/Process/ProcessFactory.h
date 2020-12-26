#pragma once
#include <functional>
#include <unordered_map>
#include <memory>
#include <Logic/Process/IProcess.h>

namespace tinyxml2
{
    class XMLElement;
}

namespace yang
{
    class Actor;

    class ProcessFactory
    {
    public:
        using ProcessFunction = std::function<std::shared_ptr<IProcess>(std::shared_ptr<Actor> pOwner)>;

        ProcessFactory() = default;

        void Init();

        void RegisterProcessCreator(uint32_t id, ProcessFunction pFunction);

        template<class Process, class... Args>
        void RegisterProcess(Args... args);

        std::shared_ptr<IProcess> CreateProcess(std::shared_ptr<Actor> pActor, tinyxml2::XMLElement* pData);
    private:
        std::unordered_map<uint32_t, ProcessFunction> m_processCreatorMap;
    };

    template<class Process, class ...Args>
    inline void ProcessFactory::RegisterProcess(Args ...args)
    {
        static_assert(std::is_base_of_v<IProcess, Process>, "IProcess should be a base class of Process");
        RegisterProcessCreator(Process::GetHashName(),
            [args...](std::shared_ptr<Actor> pOwner)
        {
            return IProcess::CreateProcess<Process::GetHashName()>(pOwner, args...);
        });
    }
}
