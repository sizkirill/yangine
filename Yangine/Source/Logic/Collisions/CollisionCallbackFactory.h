#pragma once
#include <functional>
#include <unordered_map>
#include <memory>
#include <Logic/Collisions/ICollisionCalback.h>

namespace tinyxml2
{
    class XMLElement;
}

namespace yang
{

class CollisionCallbackFactory
{
public:
    using CollisionCallbackFunction = std::function<std::unique_ptr<ICollisionCallback>()>;

    CollisionCallbackFactory() = default;

    void RegisterCallbackCreator(uint32_t id, CollisionCallbackFunction pFunction);

    template<class CollisionCallback, class... Args>
    void RegisterCollisionCallback(Args... args);

    std::unique_ptr<ICollisionCallback> CreateCollisionCallback(tinyxml2::XMLElement* pData);
private:
    std::unordered_map<uint32_t, CollisionCallbackFunction> m_callbackCreatorMap;
};

template<class CollisionCallback, class ...Args>
inline void CollisionCallbackFactory::RegisterCollisionCallback(Args ...args)
{
    static_assert(std::is_base_of_v<ICollisionCallback, CollisionCallback>, "ICollisionCallback should be a base class of CollisionCallback");
    RegisterCallbackCreator(CollisionCallback::GetHashName(),
        [args...]()
    {
        return ICollisionCallback::CreateCollisionCallback<CollisionCallback::GetHashName()>(args...);
    });
}
}
