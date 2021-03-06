#include "ColliderComponent.h"
#include <Utils/tinyxml2/tinyxml2.h>
#include <Utils/StringHash.h>
#include <Logic/Collisions/CollisionSystem.h>
#include <Logic/Components/TransformComponent.h>
#include <Logic/Actor/Actor.h>
#include <Logic/Scene/Scene.h>
#include <cassert>

using yang::ColliderComponent;
using yang::IComponent;

#pragma warning(push)
#pragma warning(disable:4307)

template<>
std::unique_ptr<IComponent> yang::IComponent::CreateComponent<StringHash32(ColliderComponent::GetName())>(yang::Actor* pOwner) 
{ 
    return std::make_unique<ColliderComponent>(pOwner); 
};

#pragma warning(pop)

ColliderComponent::ColliderComponent(yang::Actor* pOwner)
    :IComponent(pOwner, GetName())
    , m_pColliderShape(nullptr)
    , m_pCollisionCallback(nullptr)
    ,m_type(Type::kCollider)
    , m_active{ true }
{
}

bool ColliderComponent::Init(tinyxml2::XMLElement* pData)
{
    using namespace tinyxml2;

    if (auto pScene = GetOwner()->GetOwnerScene(); pScene != nullptr)
    {
        m_pCollisionSystem = pScene->GetCollisionSystem();
        if (auto pCollisionSystem = m_pCollisionSystem.lock(); pCollisionSystem != nullptr)
        {
            pCollisionSystem->RegisterCollider(0, this);
        }
        else
        {
            LOG(Error, "Collision system is not available");
            return false;
        }
    }
    else
    {
        LOG(Error, "Actor's owning scene is not available");
        return false;
    }

    XMLElement* pShape = pData->FirstChildElement("Shape");

    if (!pShape)
    {
        return false;
    }

    const char* pName = pShape->Attribute("name");
    if (!pName)
    {
        return false;
    }

    m_pColliderShape = IShape::CreateShape(pName, pShape);

    if (!m_pColliderShape)
    {
        return false;
    }

    if (XMLElement* pCollisionCallback = pData->FirstChildElement("CollisionCallback"); pCollisionCallback != nullptr)
    {
        if (auto pCollisionSystem = m_pCollisionSystem.lock(); pCollisionSystem != nullptr)
        {
            m_pCollisionCallback = pCollisionSystem->CreateCollisionCallback(pCollisionCallback->FirstChildElement());
        }
      
        if (!m_pCollisionCallback)
        {
            LOG(Warning, "Failed to initialize CollisionCallback");
        }
    }
    
    return true;
}

bool yang::ColliderComponent::PostInit()
{
    m_pTransform = GetOwner()->GetComponent<TransformComponent>();

    return m_pTransform != nullptr;
}

bool yang::ColliderComponent::Collide(ColliderComponent* pOther)
{
    assert(pOther);
    return m_pColliderShape->Collide(pOther->GetShape());
}

#ifdef DEBUG
bool yang::ColliderComponent::Render(IGraphics* pGraphics)
{
    // DEBUG
    return m_pColliderShape->DebugDraw(pGraphics);

}
#endif

void yang::ColliderComponent::Update(float deltaSeconds)
{
    m_pColliderShape->Update(m_pTransform);
}

void yang::ColliderComponent::OnCollisionStart(ColliderComponent* pOther)
{
    // DEBUG
#ifdef DEBUG
    m_pColliderShape->SetColor(IColor(255, 0, 0, 255));
#endif

    if (m_pCollisionCallback && m_active)
    {
        m_pCollisionCallback->OnCollisionStart(this, pOther);
    }
}

void yang::ColliderComponent::OnCollisionEnd(ColliderComponent* pOther)
{
    // DEBUG
#ifdef DEBUG
    m_pColliderShape->SetColor(IColor(0, 0, 255, 255));
#endif

    if (m_pCollisionCallback && m_active)
    {
        m_pCollisionCallback->OnCollisionEnd(this, pOther);
    }
}

void yang::ColliderComponent::UpdateCollision(ColliderComponent* pOther, float deltaSeconds)
{
    // Again, DEBUG stuff. Collisions should work properly, but, debug color highlighting doesnt work properly
    //      unless we're updating the color. (Because it changes at OnCollisionStart & OnCollisionEnd =>
    //      after one collision ended, it changes color to blue and then it will be red only after another collision started.
    //      After last OnCollisionEnd it should become blue.
#ifdef DEBUG
    m_pColliderShape->SetColor(IColor(255, 0, 0, 255));
#endif

    if (m_pCollisionCallback && m_active)
    {
        m_pCollisionCallback->UpdateCollision(this, pOther, deltaSeconds);
    }
}

yang::ColliderComponent::ColliderComponent(yang::Actor* pOwner, Type type, const char* pName)
    :IComponent(pOwner, pName)
    , m_pColliderShape(nullptr)
    ,m_type(type)
    , m_pCollisionCallback(nullptr)
    ,m_pTransform(nullptr)
    , m_active{true}
{
}
