#pragma once
#include <memory>
#include <cstddef>

namespace tinyxml2
{
    class XMLElement;
}

namespace yang
{
class ColliderComponent;

class ICollisionCallback
{
public:
    virtual ~ICollisionCallback() = default;
    virtual bool Init(tinyxml2::XMLElement* pData) = 0;
    virtual void OnCollisionStart(ColliderComponent* pFirst, ColliderComponent* pSecond) = 0;
    virtual void OnCollisionEnd(ColliderComponent* pFirst, ColliderComponent* pSecond) = 0;
    virtual void UpdateCollision(ColliderComponent* pFirst, ColliderComponent* pSecond, float deltaSeconds) = 0;

    template <uint32_t CollisionHashName, class... Args>
    static std::unique_ptr<ICollisionCallback> CreateCollisionCallback(Args... args);
};
}