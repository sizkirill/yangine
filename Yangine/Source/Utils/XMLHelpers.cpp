#include "XMLHelpers.h"
#include <Utils/TinyXml2/tinyxml2.h>

using yang::FVec2;

FVec2 yang::VectorFromXML(tinyxml2::XMLElement* pData, FVec2 defaultVal)
{
    FVec2 result = defaultVal;
    if (pData)
    {
        result.x = pData->FloatAttribute("x");
        result.y = pData->FloatAttribute("y");
    }

    return result;
}

float yang::AngleFromXML(tinyxml2::XMLElement* pData, const char* attributeName)
{
    return Math::ToRadians(pData->FloatAttribute(attributeName));
}

yang::IColor yang::IColorFromXML(tinyxml2::XMLElement* pData, IColor defaultVal)
{
    IColor result = defaultVal;

    if (pData)
    {
        int r = pData->IntAttribute("r", 0xff);
        int g = pData->IntAttribute("g", 0xff);
        int b = pData->IntAttribute("b", 0xff);
        int a = pData->IntAttribute("a", 0xff);
        result = IColor(r, g, b, a);
    }

    return result;
}

yang::FColor yang::FColorFromXML(tinyxml2::XMLElement* pData, FColor defaultVal)
{
    FColor result = defaultVal;

    if (pData)
    {
        float r = pData->FloatAttribute("r", 1.f);
        float g = pData->FloatAttribute("g", 1.f);
        float b = pData->FloatAttribute("b", 1.f);
        float a = pData->FloatAttribute("a", 1.f);
        result = FColor(r, g, b, a);
    }

    return result;
}

