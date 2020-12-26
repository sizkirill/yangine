#include "ProcessFactory.h"
#include <Utils/TinyXml2/tinyxml2.h>
#include <Utils/StringHash.h>
#include <Utils/Logger.h>
#include <Application/ApplicationLayer.h>
#include <Logic/Actor/Actor.h>

using namespace yang;

void yang::ProcessFactory::Init()
{
}

void yang::ProcessFactory::RegisterProcessCreator(uint32_t id, ProcessFunction pFunction)
{
    m_processCreatorMap[id] = pFunction;
}

std::shared_ptr<yang::IProcess> yang::ProcessFactory::CreateProcess(std::shared_ptr<Actor> pActor, tinyxml2::XMLElement* pData)
{
    const char* name = pData->Attribute("name");

    if (!name)
    {
        LOG(Error, "Process doesn't have a name");
        return nullptr;
    }

    uint32_t id = StringHash32(name);

    // Check if the id is in the map
    auto createItr = m_processCreatorMap.find(id);
    if (createItr == m_processCreatorMap.end())
    {
        LOG(Error, "No associated creation function for process ID %d", id);
        return nullptr;
    }

    std::shared_ptr<IProcess> pProcess = createItr->second(pActor);

    if (pProcess == nullptr)
    {
        LOG(Error, "Failed to create process (ID: %d)", id);
        return nullptr;
    }

    if (!pProcess->Init(pData))
    {
        LOG(Error, "Failed to init process (ID: %d)", id);
        return nullptr;
    }

    return pProcess;
}
