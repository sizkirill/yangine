#include "IGameLayer.h"
#include <Logic/Actor/Actor.h>
#include <Application/ApplicationLayer.h>
#include <Application/Resources/ResourceCache.h>

#include <Logic/Components/TransformComponent.h>
#include <Logic/Components/SpriteComponent.h>
#include <Logic/Components/MoveComponent.h>
#include <Logic/Components/ControllerComponent.h>
#include <Logic/Components/MouseInputListener.h>
#include <Logic/Components/TextComponent.h>
#include <Logic/Components/Animation/AnimationComponent.h>
#include <Logic/Components/RotationComponent.h>
#include <Logic/Components/Colliders/ColliderComponent.h>
#include <Logic/Components/Kinematic/KinematicComponent.h>
#include <Logic/Components/Particle/ParticleEmitterComponent.h>

#include <Logic/Process/Timers/DelayProcess.h>
#include <Logic/Process/Animation/AnimationProcess.h>

#include <Logic/Collisions/CollisionSystem.h>
#include <Logic/Event/Events/CreateActorEvent.h>
#include <Logic/Event/Events/DestroyActorEvent.h>
#include <Logic/Event/EventDispatcher.h>

#include <Utils/Random.h>
#include <Utils/StringHash.h>
#include <Utils/TinyXml2/tinyxml2.h>

// Still need it here because of
#include <Lua/lua.hpp>

#include <assert.h>

using yang::IGameLayer;

bool yang::IGameLayer::Init(const yang::ApplicationLayer& app)
{
	LOG_CATEGORY(Lua, 0, Cyan, Light);

    EventDispatcher::Get()->AddEventListener(DestroyActorEvent::kEventId, [this](IEvent* pEvent)
        {
            DestroyActor(static_cast<DestroyActorEvent*>(pEvent)->GetActorId());
        });

    EventDispatcher::Get()->AddEventListener(CreateActorEvent::kEventId, [this](IEvent* pEvent)
        {
            CreateActorEvent* pCreateActorEvent = static_cast<CreateActorEvent*>(pEvent);
            auto pFilepath = pCreateActorEvent->GetXmlFilepath();
            auto maybeLocation = pCreateActorEvent->GetSpawnPosition();
            SpawnActor(pFilepath, {}, maybeLocation);
        });

    m_viewFactory.Init(app);
    m_processFactory.Init();

    RegisterComponents();
    RegisterProcesses();

	if (!LuaState::GetInstance())
	{
		LOG(Error, "Failed to initialize Lua State");
		return false;
	}

	// Init scripting
	RegisterWithLua(LuaState::GetInstance()->GetRaw());

    return true;
}

void yang::IGameLayer::AddView(std::unique_ptr<IView>&& pView, std::optional<uint32_t> sceneIdHint)
{
    if (sceneIdHint && m_loadedScenes.count(*sceneIdHint))
    {
        m_loadedScenes[*sceneIdHint]->AddView(std::move(pView));
        return;
    }

    if (Id id = pView->GetActorId(); id != kInvalidValue<Id>)
    {
        if (auto pScene = FindSceneByActorId(id); pScene != nullptr)
        {
            pScene->AddView(std::move(pView));
        }
        else
        {
            LOG(Error, "Owner scene for actor ID: %d was not found", id);
        }
    }
    else
    {
        LOG(Error, "Trying to add view with invalid actor ID. Unable to find appropriate scene");
    }
}

void yang::IGameLayer::Update(float deltaSeconds)
{
    EventDispatcher::Get()->ProcessEvents();

    for (auto pScene : m_scenes[(size_t)SceneStatus::kActive])
    {
        pScene->Update(deltaSeconds);
    }

    m_pCurrentScene->Render();

    for (auto pScene : m_scenes[(size_t)SceneStatus::kUnload])
    {
        m_loadedScenes.erase(pScene->GetHashName());
        pScene->Cleanup();
    }
    m_scenes[(size_t)SceneStatus::kUnload].clear();
}

void yang::IGameLayer::Cleanup()
{
    for (auto& sceneContainer : m_scenes)
    {
        for (auto pScene : sceneContainer)
        {
            pScene->Cleanup();
        }

        sceneContainer.clear();
    }
    m_loadedScenes.clear();
}

void yang::IGameLayer::AddProcess(std::shared_ptr<IProcess> pProcess, std::optional<uint32_t> sceneIdHint)
{
    if (sceneIdHint && m_loadedScenes.count(*sceneIdHint))
    {
        m_loadedScenes[*sceneIdHint]->AddProcess(pProcess);
        return;
    }

    if (Id id = pProcess->GetOwner()->GetId(); id != kInvalidValue<Id>)
    {
        if (auto pScene = FindSceneByActorId(id); pScene != nullptr)
        {
            pScene->AddProcess(pProcess);
        }
        else
        {
            LOG(Error, "Owner scene for actor ID: %d was not found", id);
        }
    }
    else
    {
        LOG(Error, "Trying to add process with invalid actor ID. Unable to find appropriate scene");
    }
}

std::shared_ptr<yang::Actor> yang::IGameLayer::SpawnActor(const char* filepath, std::optional<uint32_t> sceneIdHint, std::optional<FVec2> maybeLocation)
{
    if (sceneIdHint && m_loadedScenes.count(*sceneIdHint))
    {
        return m_loadedScenes[*sceneIdHint]->SpawnActor(filepath, maybeLocation);
    }

    if (m_pCurrentScene)
    {
        return m_pCurrentScene->SpawnActor(filepath, maybeLocation);
    }
    else
    {
        LOG(Error, "No current scene selected, nowhere to spawn actor");
    }

    return nullptr;
}

std::shared_ptr<yang::Actor> yang::IGameLayer::SpawnActor(std::shared_ptr<IResource> pResource, std::optional<uint32_t> sceneIdHint, std::optional<FVec2> maybeLocation)
{
    if (sceneIdHint && m_loadedScenes.count(*sceneIdHint))
    {
        return m_loadedScenes[*sceneIdHint]->SpawnActor(pResource, maybeLocation);
    }

    if (m_pCurrentScene)
    {
        return m_pCurrentScene->SpawnActor(pResource, maybeLocation);
    }
    else
    {
        LOG(Error, "No current scene selected, nowhere to spawn actor");
    }

    return nullptr;
}

void yang::IGameLayer::DestroyActor(Id actorId, std::optional<uint32_t> sceneIdHint)
{
    if (sceneIdHint && m_loadedScenes.count(*sceneIdHint))
    {
        m_loadedScenes[*sceneIdHint]->DestroyActor(actorId);
        return;
    }

    if (auto pScene = FindSceneByActorId(actorId); pScene != nullptr)
    {
        pScene->DestroyActor(actorId);
    }
    else
    {
        LOG(Error, "Owner scene for actor ID: %d was not found", actorId);
    }
}

void yang::IGameLayer::RegisterWithLua(lua_State* pState)
{
	// Let's create a global reference to our IGameLayer...
	lua_pushlightuserdata(pState, this);
	lua_setglobal(pState, "GameLayer");

	// I guess there is no other way to do that 
	lua_register(pState, "Log", Lua::Logger_Log);

	Actor::RegisterToLua(m_luaManager);
    AnimationComponent::RegisterToLua(m_luaManager);
    MouseInputListener::RegisterToLua(m_luaManager);
	TransformComponent::RegisterToLua(m_luaManager);
	SpriteComponent::RegisterToLua(m_luaManager);
	MoveComponent::RegisterToLua(m_luaManager);
	IGameLayer::RegisterToLua(m_luaManager);

}

void yang::IGameLayer::RegisterToLua(const LuaManager& manager)
{
	manager.ExposeToLua("GetActorById", &IGameLayer::GetActorById);
	manager.ExposeToLua<std::shared_ptr<Actor>(IGameLayer::*)(const char*, std::optional<uint32_t>, std::optional<FVec2>)>("SpawnActor", &IGameLayer::SpawnActor);
	manager.ExposeToLua("DestroyActorById", &IGameLayer::DestroyActor);
}

void yang::IGameLayer::RegisterComponents()
{
    m_actorFactory.RegisterComponent<TransformComponent>();
    m_actorFactory.RegisterComponent<SpriteComponent>();
    m_actorFactory.RegisterComponent<MoveComponent>();
    m_actorFactory.RegisterComponent<ControllerComponent>();
    m_actorFactory.RegisterComponent<MouseInputListener>();
    m_actorFactory.RegisterComponent<TextComponent>();
    m_actorFactory.RegisterComponent<AnimationComponent>();
    m_actorFactory.RegisterComponent<RotationComponent>();
    m_actorFactory.RegisterComponent<ColliderComponent>();
    m_actorFactory.RegisterComponent<KinematicComponent>();
    m_actorFactory.RegisterComponent<ParticleEmitterComponent>();
}

void yang::IGameLayer::RegisterProcesses()
{
    m_processFactory.RegisterProcess<DelayProcess>();
    m_processFactory.RegisterProcess<AnimationProcess>();
}

void yang::IGameLayer::DeleteView(IView* pView, std::optional<uint32_t> sceneIdHint)
{
    assert(pView);
    if (sceneIdHint && m_loadedScenes.count(*sceneIdHint))
    {
        m_loadedScenes[*sceneIdHint]->DeleteView(pView);
        return;
    }

    if (Id id = pView->GetActorId(); id != kInvalidValue<Id>)
    {
        if (auto pScene = FindSceneByActorId(id); pScene != nullptr)
        {
            pScene->DeleteView(pView);
        }
        else
        {
            LOG(Error, "Owner scene for actor ID: %d was not found", id);
        }
    }
    else
    {
        LOG(Error, "Trying to delete view with invalid actor ID. Unable to find owning scene");
    }
}

std::shared_ptr<yang::Scene> yang::IGameLayer::LoadSceneAndSwitch(std::string_view pathToXml, SceneStatus shouldUnload)
{
    auto pScene = LoadScene(pathToXml, SceneStatus::kActive);
    return SwitchScene(pScene, shouldUnload);
}

std::shared_ptr<yang::Scene> yang::IGameLayer::SwitchScene(uint32_t sceneId, SceneStatus shouldUnload)
{
    if (m_pCurrentScene && m_pCurrentScene->GetHashName() == sceneId)
    {
        return m_pCurrentScene;
    }

    if (!m_loadedScenes.count(sceneId))
    {
        LOG(Error, "Trying to switch to a scene that is not in a loaded list. Scene ID: %d", sceneId);
        return nullptr;
    }

    auto pScene = FindScene(sceneId);

    auto& pausedScenes = m_scenes[(size_t)SceneStatus::kPaused];
    auto pSceneIt = std::find(pausedScenes.begin(), pausedScenes.end(), pScene);

    if (pSceneIt != pausedScenes.end())
    {
        (*pSceneIt)->OnSceneResume();
        m_scenes[(size_t)SceneStatus::kActive].push_back(pScene);
        std::iter_swap(pSceneIt, pausedScenes.end() - 1);
        pausedScenes.pop_back();
    }

    if (m_pCurrentScene)
    {
        switch (shouldUnload)
        {
        case SceneStatus::kActive:
        {
            break;
        }
        case SceneStatus::kPaused:
        {
            PauseScene(m_pCurrentScene);
            break;
        }
        case SceneStatus::kUnload:
        {
            UnloadScene(m_pCurrentScene);
            break;
        }
        default:
        {
            LOG(Error, "Reached default in IGameLayer::SwitchScene, SceneStatus was %d", (int)(shouldUnload));
            break;
        }
        }
    }

    m_pCurrentScene = pScene;

    return m_pCurrentScene;
}

std::shared_ptr<yang::Scene> yang::IGameLayer::SwitchScene(std::shared_ptr<Scene> pScene, SceneStatus shouldUnload)
{
#ifdef DEBUG
    if (m_pCurrentScene && (m_pCurrentScene == pScene))
    {
        return m_pCurrentScene;
    }

    if (m_pCurrentScene && (m_pCurrentScene->GetHashName() == pScene->GetHashName()))
    {
        LOG(Error, "Scene hash names are equivalent, but pointers are different. Scenes should have unique names, so something went wrong. Scene names: %s, %s", m_pCurrentScene->GetName().data(), pScene->GetName().data());
        return nullptr;
    }
#endif

    return SwitchScene(pScene->GetHashName(), shouldUnload);
}

std::shared_ptr<yang::Scene> yang::IGameLayer::LoadScene(std::string_view pathToXml, SceneStatus initialStatus)
{
    using namespace tinyxml2;

    assert((size_t)initialStatus < (size_t)SceneStatus::kMaxStatus);

    auto pResource = ResourceCache::Get()->Load<IResource>(pathToXml.data());

    XMLDocument doc;
    XMLError error = doc.Parse(reinterpret_cast<const char*>(pResource->GetData().data()), pResource->GetData().size());

    if (error != tinyxml2::XML_SUCCESS)
    {
        LOG(Error, "Failed to load scene: %s -- %s", pResource->GetName().c_str(), XMLDocument::ErrorIDToName(error));
        return nullptr;
    }

    // Loaded the file! Lets grab the node and pass it to an actor
    XMLElement* pRoot = doc.RootElement();
    const char* name = pRoot->Attribute("name");
    if (!name)
    {
        LOG(Error, "Scene need to have a name");
        return nullptr;
    }

    uint32_t hashName = StringHash32(name);
    if (m_loadedScenes.count(hashName))
    {
        return FindScene(hashName);
    }

    // Check if the id is in the map
    auto createItr = m_sceneCreatorMap.find(hashName);
    if (createItr == m_sceneCreatorMap.end())
    {
        LOG(Error, "No associated creation function for Scene ID %d", hashName);
        return nullptr;
    }

    auto pScene = createItr->second(*this);

    if (pScene == nullptr)
    {
        LOG(Error, "Failed to create Scene (ID: %d)", hashName);
        return nullptr;
    }

    pScene->Init(pRoot);
    pScene->OnSceneLoad();
    m_loadedScenes.emplace(pScene->GetHashName(), pScene);

    if (initialStatus == SceneStatus::kActive)
    {
        pScene->OnSceneResume();
    }

    m_scenes[(size_t)initialStatus].push_back(pScene);
    return pScene;
}

std::shared_ptr<yang::Scene> yang::IGameLayer::FindScene(uint32_t sceneId)
{
    if (auto sceneIt = m_loadedScenes.find(sceneId); sceneIt != m_loadedScenes.end())
    {
        return sceneIt->second;
    }

    return nullptr;
}

void yang::IGameLayer::PauseScene(uint32_t sceneId)
{
    if (!m_loadedScenes.count(sceneId))
    {
        LOG(Error, "Trying to find a scene with scene ID: %d, but it is not in a loaded set", sceneId);
        return;
    }

    auto& activeScenes = m_scenes[(size_t)SceneStatus::kActive];
    auto pSceneIt = std::find_if(activeScenes.begin(), activeScenes.end(), [sceneId](auto pScene)
        {
            return pScene->GetHashName() == sceneId;
        });

    if (pSceneIt == activeScenes.end())
    {
        LOG(Warning, "Trying to pause not active scene. Scene ID: %d", sceneId);
        return;
    }

    (*pSceneIt)->OnScenePause();

    m_scenes[(size_t)SceneStatus::kPaused].push_back(*pSceneIt);
    std::iter_swap(pSceneIt, activeScenes.end() - 1);
    activeScenes.pop_back();
}

void yang::IGameLayer::PauseScene(std::shared_ptr<Scene> pScene)
{
    PauseScene(pScene->GetHashName());
}

void yang::IGameLayer::UnloadScene(uint32_t sceneId)
{
    auto pSceneIt = m_scenes[0].begin();
    auto pSceneContainerIt = std::find_if(m_scenes.begin(), m_scenes.end() - 1,
        [sceneId, &pSceneIt](auto& sceneContainer)
        {
            pSceneIt = std::find_if(sceneContainer.begin(), sceneContainer.end(),
                [sceneId](auto pScene)
                {
                    return pScene->GetHashName() == sceneId;
                });
            return pSceneIt != sceneContainer.end();
        });

    if (pSceneContainerIt == m_scenes.end() - 1)
    {
        LOG(Error, "Internal engine error. Trying to unload a scene ID: %d, but it was not found", sceneId);
        return;
    }

    (*pSceneIt)->OnSceneUnload();

    m_scenes[(size_t)SceneStatus::kUnload].push_back(*pSceneIt);
    std::iter_swap(pSceneIt, pSceneContainerIt->end() - 1);
    pSceneContainerIt->pop_back();
}

void yang::IGameLayer::UnloadScene(std::shared_ptr<Scene> pScene)
{
    UnloadScene(pScene->GetHashName());
}

void yang::IGameLayer::UnloadScenes()
{
    for (auto pScene : m_scenes[(size_t)SceneStatus::kUnload])
    {
        size_t erasedCount = m_loadedScenes.erase(pScene->GetHashName());

        if (erasedCount == 0)
        {
            LOG(Error, "Internal engine error. Scene: %s ID: %d was not found in loaded list", pScene->GetName().data(), pScene->GetHashName());
        }

        pScene->OnSceneUnload();
        pScene->Cleanup();
    }

    m_scenes[(size_t)SceneStatus::kUnload].clear();
}

void yang::IGameLayer::RegisterSceneCreator(uint32_t id, SceneFunction pFunction)
{
    m_sceneCreatorMap[id] = pFunction;
}

std::shared_ptr<yang::Scene> yang::IGameLayer::FindSceneByActorId(Id id) const
{
    for (auto [sceneId, pScene] : m_loadedScenes)
    {
        if (pScene->HasActor(id))
        {
            return pScene;
        }
    }

    return nullptr;
}

std::unordered_map<yang::Id, std::shared_ptr<yang::Actor>>& yang::IGameLayer::GetActors(std::optional<uint32_t> sceneIdHint)
{
    if (sceneIdHint && m_loadedScenes.count(*sceneIdHint))
    {
        return m_loadedScenes.at(*sceneIdHint)->GetActors();
    }

    assert(m_pCurrentScene != nullptr);
    return m_pCurrentScene->GetActors();
}

std::shared_ptr<yang::Actor> yang::IGameLayer::GetActorById(Id id, std::optional<uint32_t> sceneIdHint) const
{
    if (sceneIdHint && m_loadedScenes.count(*sceneIdHint))
    {
        return m_loadedScenes.at(*sceneIdHint)->GetActorById(id);
    }

    if (auto pScene = FindSceneByActorId(id); pScene != nullptr)
    {
        return pScene->GetActorById(id);
    }
    else
    {
        LOG(Error, "Owner scene for actor ID: %d was not found", id);
    }

    return nullptr;
}