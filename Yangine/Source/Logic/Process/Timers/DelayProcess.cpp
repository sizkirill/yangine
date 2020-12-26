#include "DelayProcess.h"
#include <Utils/TinyXml2/tinyxml2.h>

using yang::DelayProcess;
using yang::IProcess;

#pragma warning(push)
#pragma warning(disable:4307)

template<>
std::shared_ptr<IProcess> IProcess::CreateProcess<DelayProcess::GetHashName()>(std::shared_ptr<yang::Actor> pOwner)
{
    return std::make_shared<DelayProcess>(pOwner);
};

#pragma warning(pop)

yang::DelayProcess::DelayProcess(std::shared_ptr<yang::Actor> pOwner)
    :IProcess(pOwner)
    ,m_delay(0)
{
}

DelayProcess::~DelayProcess()
{
	
}

void yang::DelayProcess::Update(float deltaSeconds)
{
    m_delay -= deltaSeconds;
    if (m_delay <= 0.f)
    {
        Succeed();
    }
}

bool yang::DelayProcess::Init(tinyxml2::XMLElement* pData)
{
    m_delay = pData->FloatAttribute("delay", 0.f);
    return true;
}

bool yang::DelayProcess::PostInit()
{
    return true;
}
