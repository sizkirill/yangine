#pragma once
/** \file AnimationProcess.h */
/** Animation process description */

#include <Logic/Process/BaseProcess.h>
#include <Logic/Components/Animation/AnimationComponent.h>
#include <Logic/Components/SpriteComponent.h>
#include <Utils/StringHash.h>
#include <string_view>

//! \namespace yang Contains all Yangine code
namespace yang
{
	class AnimationComponent;
	class SpriteComponent;

	/** \class AnimationProcessDescription */
	/** Handles the animation of an actor that has SpriteComponent & AnimationComponent. */
class AnimationProcessDescription
{
public:
	AnimationProcessDescription(IProcess* pOwner);
	void Update(float deltaSeconds, yang::AnimationComponent* pAnimationData, SpriteComponent* pSpriteData);
	bool Init(tinyxml2::XMLElement* pData);

	static constexpr std::string_view kName = "AnimationProcess";
	static constexpr uint32_t kHashName = StringHash32(kName.data());

	static constexpr std::string_view GetName() { return kName; }
	static constexpr uint32_t GetHashName() { return kHashName; }
private:
	float m_currentFrameTime = 0;
	IProcess* m_pOwner = nullptr;
};

/// <summary>
///  Alias for AnimationProcess
/// </summary>
using AnimationProcess = yang::BaseProcess<AnimationProcessDescription, AnimationComponent, SpriteComponent>;
}