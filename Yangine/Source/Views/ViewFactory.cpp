#include "ViewFactory.h"
#include <Utils/TinyXml2/tinyxml2.h>
#include <Utils/StringHash.h>
#include <Utils/Logger.h>
#include <Application/ApplicationLayer.h>

void yang::ViewFactory::Init(const ApplicationLayer& app)
{
    m_pAppLayer = &app;
}

void yang::ViewFactory::RegisterViewCreator(uint32_t id, ViewFunction pFunction)
{
    m_viewCreatorMap[id] = pFunction;
}

std::unique_ptr<yang::IView> yang::ViewFactory::CreateView(std::shared_ptr<Actor> pActor, tinyxml2::XMLElement* pData)
{
    const char* name = pData->Attribute("name");

    if (!name)
    {
        LOG(Error, "View doesn't have a name");
        return nullptr;
    }

    uint32_t id = StringHash32(name);

    // Check if the id is in the map
    auto createItr = m_viewCreatorMap.find(id);
    if (createItr == m_viewCreatorMap.end())
    {
        LOG(Error, "No associated creation function for view ID %d", id);
        return nullptr;
    }

    std::unique_ptr<IView> pView = createItr->second();

    if (pView == nullptr)
    {
        LOG(Error, "Failed to create view (ID: %d)", id);
        return nullptr;
    }

    pView->SetControlledActor(pActor);

    assert(m_pAppLayer);

    if (!pView->Init(*m_pAppLayer, pData))
    {
        LOG(Error, "Failed to init view (ID: %d)", id);
        return nullptr;
    }

    return pView;
}
