#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <string_view>
#include <optional>
#include <Logic/Process/ProcessManager.h>
#include <Views/IView.h>
#include <Utils/Typedefs.h>
#include <Utils/Vector2.h>

namespace tinyxml2
{
    class XMLElement;
}

namespace yang
{
    class IGameLayer;
    class Actor;
    class CollisionSystem;
    class ICollisionCallback;
    class IResource;

    class Scene : public std::enable_shared_from_this<Scene>
    {
    public:
        Scene(yang::IGameLayer& owner);
        virtual ~Scene() = default;

        bool Init(tinyxml2::XMLElement* pData);

        /// Add view to the list of views
        /// \param pView - unique pointer to a View to add
        void AddView(std::unique_ptr<IView> pView);

        /// Update all modules by deltaSeconds
        /// \param deltaSeconds - time passed since last frame
        virtual void Update(float deltaSeconds);

        void Render();

        /// Cleans up memory allocations and 3rd party libraries
        void Cleanup();

        /// Adds process to a process manager
        /// \param pProcess - shared pointer to a process to add
        void AddProcess(std::shared_ptr<IProcess> pProcess);

        // TODO: Add delayed spawning. 
        /// Spawns actor in the world at next frame
        /// \param filepath - path to the XML file that describes the actor
        /// \param whereToSpawn - location where to spawn an actor
        /// \return shared pointer to the spawned actor. Can be null if spawning failed
        std::shared_ptr<Actor> SpawnActor(const char* filepath, std::optional<FVec2> whereToSpawn = {});

        /// Spawns actor in the world at next frame
        /// \param pResource - XML resource that describes the actor
        /// \param whereToSpawn - location where to spawn an actor
        /// \return shared pointer to the spawned actor. Can be null if spawning failed
        std::shared_ptr<Actor> SpawnActor(std::shared_ptr<IResource> pResource, std::optional<FVec2> whereToSpawn = {});

        /// Destroys actor at next frame
        /// \param actorId - Id of the actor to destroy
        void DestroyActor(Id actorId);

        // thing to consider:
        // Since event dispatcher is a singleton and is global for everything, we need to disable event handlers when scene is not active
        virtual void OnSceneLoad() {};     // Subscribe for events
        virtual void OnScenePause() {};    // Do stuff when scene pauses (unsubscribe from events?)
        virtual void OnSceneResume() {};   // Do stuff when scene becomes active
        virtual void OnSceneUnload() {};   // Do stuff when scene unloads?

        std::unique_ptr<ICollisionCallback> CreateCollisionCallback(tinyxml2::XMLElement* pData);
        std::shared_ptr<IProcess> CreateProcess(std::shared_ptr<yang::Actor> pOwner, tinyxml2::XMLElement* pData);

        bool HasActor(Id id);

        template <uint32_t SceneHashName, class... Args>
        static std::shared_ptr<Scene> CreateScene(yang::IGameLayer& owner, Args... args);
    private:
        std::string m_name;
        uint32_t m_hashName;
        yang::IGameLayer& m_owner;

        std::unordered_map<Id, std::shared_ptr<Actor>> m_actors;    ///< Hash table of actors, where keys are their ids
        ProcessManager m_processManager;                            ///< Instance of ProcessManager that handles all game processes
        std::vector<std::shared_ptr<Actor>> m_actorsToSpawn;        ///< Collection of actors that are going to be spawned at next frame
        std::unordered_set<Id> m_queuedActorIds;
        std::unordered_multimap<uint32_t, Id> m_actorIdByHashTag;   ///< Collection of actor Ids by their hashtags;
        std::vector<Id> m_actorsToKill;                             ///< Collection of IDs of actors that are going to be destroyed at next frame
        std::vector<std::unique_ptr<IView>> m_pViews;               ///< Collection of all views
        std::shared_ptr<CollisionSystem> m_pCollisionSystem;
    private:
        /// Internal helper function. Deletes view by it's index in the vector
        /// \param index - view's index in the vector
        void DeleteView(size_t index);
    public:
        /// \param pView - view to delete
        void DeleteView(IView* pView);

        /// Get the actor table
        std::unordered_map<Id, std::shared_ptr<Actor>>& GetActors() { return m_actors; }

        /// Get the actor table
        const std::unordered_map<Id, std::shared_ptr<Actor>>& GetActors() const { return m_actors; }

        /// Get spawning actor table size
        size_t GetSpawningActorsSize() const { return m_actorsToSpawn.size(); }

        /// Get actor by it's ID from spawning actors
        std::shared_ptr<Actor> GetActorByIdFromSpawnActors(Id id) const;

        /// Get actor by it's hash tag. If there are more than one actor that has this hashTag, it will return the first one that it meets
        std::shared_ptr<Actor> GetActorByHashTag(uint32_t hashTag) const;

        /// Get actor by it's ID
        std::shared_ptr<Actor> GetActorById(Id id) const;

        std::string_view GetName() const { return m_name; }
        uint32_t GetHashName() const { return m_hashName; }
        std::shared_ptr<CollisionSystem> GetCollisionSystem() const { return m_pCollisionSystem; }
    };
}