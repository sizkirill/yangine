#pragma once
/** \file IGameLayer.h */
/** IGameLayer class description */
#include <Views/IView.h>
#include <Logic/Scripting/LuaManager.h>
#include <Logic/Scene/Scene.h>

#include <Logic/Actor/ActorFactory.h>
#include <Views/ViewFactory.h>
#include <Logic/Collisions/CollisionCallbackFactory.h>
#include <Logic/Process/ProcessFactory.h>

#include <memory>
#include <vector>
#include <array>
#include <unordered_map>
#include <optional>

//! \namespace yang Contains all Yangine code
namespace yang
{
    class ApplicationLayer;
    class ProcessManager;
/** \class IGameLayer */
/** Interface for the game logic. Must be inherited in the Game project */
class IGameLayer
{
public:
	// --------------------------------------------------------------------- //
	// Public Member Variables
	// --------------------------------------------------------------------- //


	// --------------------------------------------------------------------- //
	// Public Member Functions
	// --------------------------------------------------------------------- //
	/** Default Constructor */
	IGameLayer() = default;

	/** Default Destructor */
	virtual ~IGameLayer() = default;

    /// Get the game name for the window title
	virtual const char* GetGameName() const = 0;

    /// Initialize all modules
    /// \param app - application layer
    /// \return true if successful
    virtual bool Init(const ApplicationLayer& app);

    /// Add view to the list of views
    /// \param pView - unique pointer to a View to add
    virtual void AddView(std::unique_ptr<IView>&& pView, std::optional<uint32_t> sceneIdHint = {});

    /// Update all modules by deltaSeconds
    /// \param deltaSeconds - time passed since last frame
    virtual void Update(float deltaSeconds);

    /// Cleans up memory allocations and 3rd party libraries
    virtual void Cleanup();

    /// Adds process to a process manager
    /// \param pProcess - shared pointer to a process to add
    virtual void AddProcess(std::shared_ptr<IProcess> pProcess, std::optional<uint32_t> sceneIdHint = {});

    // TODO: Add delayed spawning. 
    /// Spawns actor in the world at next frame
    /// \param filepath - path to the XML file that describes the actor
    /// \param sceneIdHint - optional id of scene where to spawn (if not provided - uses current scene)
    /// \param whereToSpawn - location where to spawn an actor
    /// \return shared pointer to the spawned actor. Can be null if spawning failed
    virtual std::shared_ptr<Actor> SpawnActor(const char* filepath, std::optional<uint32_t> sceneIdHint = {}, std::optional<FVec2> whereToSpawn = {});

    /// Spawns actor in the world at next frame
    /// \param pResource - XML Resource that describes the actor
    /// \param sceneIdHint - optional id of scene where to spawn (if not provided - uses current scene)
    /// \param whereToSpawn - location where to spawn an actor
    /// \return shared pointer to the spawned actor. Can be null if spawning failed
    std::shared_ptr<Actor> SpawnActor(std::shared_ptr<IResource> pResource, std::optional<uint32_t> sceneIdHint = {}, std::optional<FVec2> whereToSpawn = {});

    /// Destroys actor at next frame
    /// \param actorId - Id of the actor to destroy
    void DestroyActor(Id actorId, std::optional<uint32_t> sceneIdHint = {});

	// TODO: rethink
    /// Registers everything that is needed to be registered to Lua environment.
    /// \param pState - lua_State object to register to
	void RegisterWithLua(lua_State* pState);

    /// Registers this class member function to the Lua environment
    /// \param manager - LuaManager instance to handle the registration
	static void RegisterToLua(const LuaManager& manager);
protected:
    // --------------------------------------------------------------------- //
    // Protected Member Variables
    // --------------------------------------------------------------------- //
    ActorFactory m_actorFactory;                                ///< Instance of ActorFactory that handle actor spawning. \see yang::ActorFactory
    ViewFactory m_viewFactory;                                  ///< Instance of ViewFactory that handles creating views. \see yang::ViewFactory
    CollisionCallbackFactory m_collisionCallbackFactory;        ///< Instance of CollisionCallbackFactory that creates collision callbacks from XML.
    ProcessFactory m_processFactory;                            ///< Instance of ProcessFactory that creates processes and attaches them to actors.
    LuaManager m_luaManager;                                    ///< Instance of a LuaManager

    // --------------------------------------------------------------------- //
    // Protected Member Functions
    // --------------------------------------------------------------------- //

    /// Internal helper function. Registers component creator functions
    void RegisterComponents();

    /// Internal helper function. Registers process creator functions
    void RegisterProcesses();

    /// \param pView - view to delete
    void DeleteView(IView* pView, std::optional<uint32_t> sceneIdHint = {});
public:
    enum class SceneStatus
    {
        kActive,
        kPaused,
        kUnload,
        kMaxStatus
    };

    std::shared_ptr<Scene> LoadSceneAndSwitch(std::string_view pathToXml, SceneStatus shouldUnload = SceneStatus::kUnload);
    std::shared_ptr<Scene> SwitchScene(uint32_t sceneId, SceneStatus shouldUnload = SceneStatus::kUnload);
    std::shared_ptr<Scene> SwitchScene(std::shared_ptr<Scene> pScene, SceneStatus shouldUnload = SceneStatus::kUnload);
    std::shared_ptr<Scene> LoadScene(std::string_view pathToXml, SceneStatus initialStatus);
    std::shared_ptr<Scene> FindScene(uint32_t sceneId);
    void PauseScene(uint32_t sceneId);
    void PauseScene(std::shared_ptr<Scene> pScene);
    void UnloadScene(uint32_t sceneId);
    void UnloadScene(std::shared_ptr<Scene> pScene);
    std::shared_ptr<Scene> GetCurrentScene() const { return m_pCurrentScene; }
protected:
    std::array<std::vector<std::shared_ptr<Scene>>, static_cast<size_t>(SceneStatus::kMaxStatus)> m_scenes;
    std::unordered_map<uint32_t, std::shared_ptr<Scene>> m_loadedScenes;
    std::shared_ptr<Scene> m_pCurrentScene;
    void UnloadScenes();

    using SceneFunction = std::function<std::shared_ptr<Scene>(IGameLayer& owner)>;

    void RegisterSceneCreator(uint32_t id, SceneFunction pFunction);

    template<class ActualScene, class... Args>
    void RegisterScene(Args... args);

    std::unordered_map<uint32_t, SceneFunction> m_sceneCreatorMap;
private:
	// --------------------------------------------------------------------- //
	// Private Member Variables
	// --------------------------------------------------------------------- //

	// --------------------------------------------------------------------- //
	// Private Member Functions
	// --------------------------------------------------------------------- //

    std::shared_ptr<Scene> FindSceneByActorId(Id id) const;
public:
	// --------------------------------------------------------------------- //
	// Accessors & Mutators
	// --------------------------------------------------------------------- //

    /// Get the actor table
    std::unordered_map<Id, std::shared_ptr<Actor>>& GetActors(std::optional<uint32_t> sceneIdHint = {});

	/// Get actor by it's ID
    std::shared_ptr<Actor> GetActorById(Id id, std::optional<uint32_t> sceneIdHint = {}) const;


	/// Get the instance of lua manager
    const LuaManager* GetLuaManager() const { return &m_luaManager; }

    ActorFactory& GetActorFactory() { return m_actorFactory; }
    ViewFactory& GetViewFactory() { return m_viewFactory; }
    CollisionCallbackFactory& GetCollisionCallbackFactory() { return m_collisionCallbackFactory; }
    ProcessFactory& GetProcessFactory() { return m_processFactory; }
};
template<class ActualScene, class ...Args>
inline void IGameLayer::RegisterScene(Args ...args)
{
    static_assert(std::is_base_of_v<Scene, ActualScene>, "ActualScene should be a base class of Scene");
    RegisterSceneCreator(ActualScene::GetHashName(),
        [args...](yang::IGameLayer& owner)
    {
        return Scene::CreateScene<ActualScene::GetHashName()>(owner, args...);
    });
}
}