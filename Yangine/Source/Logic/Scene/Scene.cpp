#include "Scene.h"
#include <cassert>
#include <optional>
#include <Logic/IGameLayer.h>
#include <Logic/Actor/Actor.h>
#include <Logic/Actor/ActorFactory.h>
#include <Logic/Components/TransformComponent.h>
#include <Logic/Collisions/CollisionSystem.h>
#include <Utils/TinyXml2/tinyxml2.h>
#include <Utils/Vector2.h>
#include <Utils/XMLHelpers.h>

using namespace yang;

yang::Scene::Scene(yang::IGameLayer& owner)
    :m_owner(owner)
{
}

bool yang::Scene::Init(tinyxml2::XMLElement* pData)
{
    using namespace tinyxml2;

    m_pCollisionSystem.reset(new CollisionSystem);
    m_pCollisionSystem->Init(shared_from_this());

    m_name = pData->Attribute("name");
    m_hashName = StringHash32(m_name.data());

    for (XMLElement* pActorData = pData->FirstChildElement("Actor"); pActorData != nullptr; pActorData = pActorData->NextSiblingElement("Actor"))
    {
        const char* pActorSrc = pActorData->Attribute("src");
        
        if (!pActorSrc)
        {
            LOG(Warning, "Did not find src attribute for actor while initializing scene. Skipping the actor");
            continue;
        }

        std::optional<FVec2> whereToSpawn = {};

        if (XMLElement* pLocationData = pActorData->FirstChildElement("Location"); pLocationData != nullptr)
        {
            whereToSpawn = FVectorFromXML(pLocationData);
        }

        auto pActor = SpawnActor(pActorSrc, whereToSpawn);

        if (!pActor)
        {
            LOG(Warning, "Failed spawning actor during scene initialization. Actor src file: %s. Skipping the actor", pActorSrc);
            continue;
        }

        if (XMLElement* pViewData = pActorData->FirstChildElement("View"); pViewData != nullptr)
        {
            auto pView = m_owner.GetViewFactory().CreateView(pActor, pViewData);

            if (!pView)
            {
                LOG(Warning, "Failed creating view for Actor (ID: %d) during scene initialization. Actor src file: %s. Skipping the actor", pActor->GetId(), pActorSrc);
                continue;
            }

            AddView(std::move(pView));
        }

        for (XMLElement* pProcessData = pActorData->FirstChildElement("Process"); pProcessData != nullptr; pProcessData = pActorData->NextSiblingElement("Process"))
        {
            auto pProcess = CreateProcess(pActor, pProcessData);

            if (!pProcess)
            {
                LOG(Warning, "Failed creating process for Actor (ID: %d) during scene initialization. Actor src file: %s. Skipping the actor", pActor->GetId(), pActorSrc);
                continue;
            }

            AddProcess(pProcess);
        }
    }

    return true;
}

void yang::Scene::AddView(std::unique_ptr<IView> pView)
{
    pView->SetIndex(m_pViews.size());
    m_pViews.emplace_back(std::move(pView));
}

void yang::Scene::Update(float deltaSeconds)
{
    for (auto& pActor : m_actorsToSpawn)
    {
        m_actors.emplace(pActor->GetId(), pActor);
    }
    m_actorsToSpawn.clear();
    m_queuedActorIds.clear();

    for (auto& pView : m_pViews)
    {
        pView->UpdateInput();
    }

    m_processManager.UpdateProcesses(deltaSeconds);

    for (auto& pActor : m_actors)
    {
        pActor.second->Update(deltaSeconds);
    }

    m_pCollisionSystem->Update(deltaSeconds);

    for (Id id : m_actorsToKill)
    {
        auto pActorIdPair = m_actors.find(id);
        if (pActorIdPair != m_actors.end())
        {
            auto [startIt, endIt] = m_actorIdByHashTag.equal_range(pActorIdPair->second->GetHashTag());
            for (auto it = startIt; it != endIt; ++it)
            {
                if (it->second == id)
                {
                    m_actorIdByHashTag.erase(it);
                    break;
                }
            }

            auto pActorView = pActorIdPair->second->GetOwningView();

            if (pActorView)
            {
                pActorView->DetachActor();
                DeleteView(pActorView);
            }
            m_pCollisionSystem->ClearCollisionsWithActor(pActorIdPair->second.get());
            m_processManager.AbortProcessesOnActor(id);
            m_actors.erase(id);
        }
    }
    m_actorsToKill.clear();
}

void yang::Scene::Render()
{
    for (auto& pView : m_pViews)
    {
        pView->ViewScene();
    }
}

void yang::Scene::Cleanup()
{
    for (auto& pView : m_pViews)
    {
        pView->DetachActor();
        pView.reset();
    }

    m_actors.clear();
    m_actorsToSpawn.clear();
    m_queuedActorIds.clear();
}

void yang::Scene::AddProcess(std::shared_ptr<IProcess> pProcess)
{
    m_processManager.AttachProcess(pProcess);
}

std::shared_ptr<Actor> yang::Scene::SpawnActor(const char* filepath, std::optional<FVec2> whereToSpawn)
{
    auto pActor = m_owner.GetActorFactory().CreateActor(filepath, shared_from_this());

    if (pActor)
    {
        m_actorsToSpawn.emplace_back(pActor);
        m_queuedActorIds.emplace(pActor->GetId());
        m_actorIdByHashTag.emplace(pActor->GetHashTag(), pActor->GetId());
        if (auto pTransform = pActor->GetComponent<TransformComponent>(); pTransform && whereToSpawn)
        {
            pTransform->SetPosition(*whereToSpawn);
        }
    }

    return pActor;
}

std::shared_ptr<Actor> yang::Scene::SpawnActor(std::shared_ptr<IResource> pResource, std::optional<FVec2> whereToSpawn)
{
    auto pActor = m_owner.GetActorFactory().CreateActor(pResource.get(), shared_from_this());

    if (pActor)
    {
        m_actorsToSpawn.emplace_back(pActor);
        m_queuedActorIds.emplace(pActor->GetId());
        m_actorIdByHashTag.emplace(pActor->GetHashTag(), pActor->GetId());
        if (auto pTransform = pActor->GetComponent<TransformComponent>(); pTransform && whereToSpawn)
        {
            pTransform->SetPosition(*whereToSpawn);
        }
    }

    return pActor;
}

void yang::Scene::DestroyActor(Id actorId)
{
    m_actorsToKill.emplace_back(actorId);
}

std::unique_ptr<ICollisionCallback> yang::Scene::CreateCollisionCallback(tinyxml2::XMLElement* pData)
{
    return m_owner.GetCollisionCallbackFactory().CreateCollisionCallback(pData);
}

std::shared_ptr<IProcess> yang::Scene::CreateProcess(std::shared_ptr<yang::Actor> pOwner, tinyxml2::XMLElement* pData)
{
    return m_owner.GetProcessFactory().CreateProcess(pOwner, pData);
}

bool yang::Scene::HasActor(Id id)
{
    return (m_actors.count(id) > 0) || (m_queuedActorIds.count(id) > 0);
}

void yang::Scene::DeleteView(IView* pView)
{
    size_t index = pView->GetIndex();
    assert(m_pViews[index].get() == pView);
    DeleteView(index);
}

void yang::Scene::DeleteView(size_t index)
{
    assert(index < m_pViews.size());
    std::swap(m_pViews[index], m_pViews[m_pViews.size() - 1]);
    m_pViews[index]->SetIndex(index);
    m_pViews.pop_back();
}

std::shared_ptr<Actor> yang::Scene::GetActorByIdFromSpawnActors(Id id) const
{
    assert(id < m_actorsToSpawn.size());
    return m_actorsToSpawn[id];
}

std::shared_ptr<Actor> yang::Scene::GetActorByHashTag(uint32_t hashTag) const
{
    auto [startIt, endIt] = m_actorIdByHashTag.equal_range(hashTag);

    if (startIt == m_actorIdByHashTag.end())
        return nullptr;

    return GetActorById(startIt->second);
}

std::shared_ptr<Actor> yang::Scene::GetActorById(Id id) const
{
    if (m_actors.count(id) > 0)
        return m_actors.at(id);

    if (m_queuedActorIds.count(id) > 0)
        return *(std::find_if(m_actorsToSpawn.begin(), m_actorsToSpawn.end(), [id](auto pActor) {return pActor->GetId() == id; }));

    return nullptr;
}
