#pragma once
#include <unordered_map>
#include <set>
#include <vector>
#include <memory>
#include <Logic/Collisions/Collision.h>

namespace tinyxml2
{
    class XMLElement;
}

namespace yang
{
class Actor;
class Scene;
class ICollisionCallback;

class CollisionSystem
{
public:
    CollisionSystem() = default;
    void Init(std::shared_ptr<Scene> pOwner);
    void RegisterCollider(int layer, ColliderComponent* pComponent);
    void Update(float deltaSeconds);

    using CollisionPair = std::pair<ColliderComponent*, ColliderComponent*>;

    struct CollisionPairHelper
    {
        struct Hash
        {
            std::size_t operator() (const CollisionPair& pair) const
            {
                return std::hash<ColliderComponent*>()(pair.first) ^ std::hash<ColliderComponent*>()(pair.second);
            }
        };
        struct Equals
        {
            bool operator()(const CollisionPair& p1, const CollisionPair& p2) const
            {
                return Hash()(p1) == Hash()(p2);
            }
        };
    };

    using ActiveCollisions = std::unordered_map <CollisionPair, std::unique_ptr<Collision>, CollisionPairHelper::Hash, CollisionPairHelper::Equals>;

    const std::vector<Collision*>& GetCollisionsOnCollider(ColliderComponent* pCollider);
    void ClearCollisionsWithActor(yang::Actor* pActor);
    void ClearCollisionsWithCollider(ColliderComponent* pCollider);

    std::unique_ptr<ICollisionCallback> CreateCollisionCallback(tinyxml2::XMLElement* pData);

private:
    std::weak_ptr<Scene> m_pOwnerScene;
    std::unordered_map<int, std::vector<ColliderComponent*>> m_colliders;
    ActiveCollisions m_activeCollisions;
    std::unordered_map<ColliderComponent*, std::vector<Collision*>> m_activeCollisionsByActor;
    std::vector<Collision*> m_recentCollisions;
};
}