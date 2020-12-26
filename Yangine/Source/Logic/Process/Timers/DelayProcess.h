#pragma once
/** \file DelayProcess.h */
/** Delay process description */

#include "..\IProcess.h"
#include <Utils/StringHash.h>
#include <string_view>

//! \namespace yang Contains all Yangine code
namespace yang
{
/** \class DelayProcess */
/** Executes Succeed callback after some amount of seconds */
class DelayProcess
	: public IProcess
{
public:
	// --------------------------------------------------------------------- //
	// Public Member Variables
	// --------------------------------------------------------------------- //


	// --------------------------------------------------------------------- //
	// Public Member Functions
	// --------------------------------------------------------------------- //

    /// Constructor
    /// \param pOwner - actor that owns this process
    /// \param delay - seconds of delay
	DelayProcess(std::shared_ptr<yang::Actor> pOwner);

	/** Default Destructor */
	virtual ~DelayProcess();

    /// Updates the process by deltaSeconds. Effectively subtracts deltaSeconds from delay
    /// \param deltaSeconds - amount of seconds passed since last frame
    virtual void Update(float deltaSeconds) override final;

	virtual bool Init(tinyxml2::XMLElement* pData) override final;
	virtual bool PostInit() override final;

	static constexpr std::string_view kName = "DelayProcess";
	static constexpr uint32_t kHashName = StringHash32(kName.data());

	static constexpr std::string_view GetName() { return kName; }
	static constexpr uint32_t GetHashName() { return kHashName; }
private:
	// --------------------------------------------------------------------- //
	// Private Member Variables
	// --------------------------------------------------------------------- //
    float m_delay;  ///< amount of seconds to wait before executing succeed callback

	// --------------------------------------------------------------------- //
	// Private Member Functions
	// --------------------------------------------------------------------- //


public:
	// --------------------------------------------------------------------- //
	// Accessors & Mutators
	// --------------------------------------------------------------------- //
	void SetDelay(float delay) { m_delay = delay; }

};
}