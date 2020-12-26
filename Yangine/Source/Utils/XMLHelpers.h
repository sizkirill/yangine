#pragma once
#include <Utils/Vector2.h>
#include <Utils/Color.h>

namespace tinyxml2
{
    class XMLElement;
}

namespace yang
{
    /// Helper function to read Vector2 from XML element
    /// \param pData - XML element to read from
    /// \return Vector2 from the xml element
    Vector2<float> VectorFromXML(tinyxml2::XMLElement* pData, FVec2 defaultVal = { 0,0 });

    /// Helper function to read angle from XML element. Converts degrees to radians
    /// \param pData - XML element to read from
    /// \param attributeName - name of the attribute that contains angle
    /// \return angle in radians
    float AngleFromXML(tinyxml2::XMLElement* pData, const char* attributeName);

    /// Helper function to read IColor from XML element
    /// \param pData - XML element to read from
    /// \return IColor from the xml element
    IColor IColorFromXML(tinyxml2::XMLElement* pData, IColor defaultVal = IColor::kWhite);

    /// Helper function to read FColor from XML element
    /// \param pData - XML element to read from
    /// \return FColor from the xml element
    FColor FColorFromXML(tinyxml2::XMLElement* pData, FColor defaultVal = { 1,1,1,1 });
}