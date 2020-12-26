#include "IProcess.h"

using yang::IProcess;

IProcess::IProcess(std::shared_ptr<yang::Actor> pOwner)
    :m_state(State::kUninitialized)
    ,m_pOwner(pOwner)
{
	
}

IProcess::~IProcess()
{
	
}

bool yang::IProcess::Init(tinyxml2::XMLElement* pData)
{ 
    return true;
}

bool yang::IProcess::PostInit()
{
    return true;
}

void yang::IProcess::OnAbort()
{
    if (m_abortCallback)
    {
        m_abortCallback();
    }
}

void yang::IProcess::OnSucceed()
{
    if (m_succeedCallback)
    {
        m_succeedCallback();
    }
}

void yang::IProcess::OnFail()
{
    if (m_failCallback)
    {
        m_failCallback();
    }
}

std::shared_ptr<IProcess> yang::IProcess::RemoveChild()
{
    auto pChild = m_pChild;
    m_pChild = nullptr;
    return pChild;
}
