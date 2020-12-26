#include "CollisionCallbackFactory.h"
#include <Utils/TinyXml2/tinyxml2.h>
#include <Utils/StringHash.h>
#include <Utils/Logger.h>

void yang::CollisionCallbackFactory::RegisterCallbackCreator(uint32_t id, CollisionCallbackFunction pFunction)
{
    m_callbackCreatorMap[id] = pFunction;
}

std::unique_ptr<yang::ICollisionCallback> yang::CollisionCallbackFactory::CreateCollisionCallback(tinyxml2::XMLElement* pData)
{
    const char* pName = pData->Name();
    uint32_t id = StringHash32(pName);

    if (auto createIt = m_callbackCreatorMap.find(id); createIt != m_callbackCreatorMap.end())
    {
        auto pCallback = createIt->second();
        if (pCallback == nullptr || !pCallback->Init(pData))
        {
            LOG(Error, "Failed to initialize collision callback");
            return nullptr;
        }

        return pCallback;
    }

    LOG(Error, "No associated creation function found for %s", pName);
    return nullptr;
}
