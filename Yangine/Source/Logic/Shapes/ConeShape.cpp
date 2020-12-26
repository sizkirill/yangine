#include "ConeShape.h"
#include "RectangleShape.h"
#include <Application/Graphics/IGraphics.h>
#include <Utils/TinyXml2/tinyxml2.h>
#include <Utils/XMLHelpers.h>
#include <Logic/Components/TransformComponent.h>
#include <Utils/Matrix.h>

#pragma warning(push)
#pragma warning(disable:4307)

template<>
std::unique_ptr<yang::IShape> yang::IShape::CreateShape<yang::ConeShape::GetHashName()>(tinyxml2::XMLElement* pData) { return std::make_unique<ConeShape>(pData); };

#pragma warning(pop)

yang::ConeShape::ConeShape(FVec2 center, float radius, float fov, float orientation, IColor color)
    :CircleShape(center, radius, color)
    ,m_fieldOfViewAngle(fov)
    ,m_initialOrientationAngle(orientation)
{
}

yang::ConeShape::ConeShape(tinyxml2::XMLElement* pData)
    :CircleShape(pData)
{
    Init(pData);
}

bool yang::ConeShape::Init(tinyxml2::XMLElement* pData)
{
    using namespace tinyxml2;

    m_radius = pData->FloatAttribute("radius");
    m_initialOrientationAngle = yang::AngleFromXML(pData, "orientation");
    m_orientationAngle = m_initialOrientationAngle;
    m_fieldOfViewAngle = yang::AngleFromXML(pData, "fov");

    m_localCenter = yang::VectorFromXML(pData->FirstChildElement("Center"));

    XMLElement* pColor = pData->FirstChildElement("Color");

    if (pColor)
    {
        m_color = yang::IColor(pColor->IntAttribute("r"), pColor->IntAttribute("g"), pColor->IntAttribute("b"), pColor->IntAttribute("a", 255));
    }

    m_initialVertices[0] = { 0 , 0 };

    // firstPoint
    m_initialVertices[1] = yang::FVec2::FromAngle(- m_fieldOfViewAngle / 2);

    // centerPoint
    m_initialVertices[2] = { 1,0 };

    // secondPoint
    m_initialVertices[3] = yang::FVec2::FromAngle(m_fieldOfViewAngle / 2);

    yang::Matrix transform;
    transform.Rotate(m_initialOrientationAngle);
    std::transform(m_initialVertices, m_initialVertices + 4, m_initialVertices, [transform](yang::FVec2& vertex)
        {
            vertex.Normalize();
            return transform.TransformPoint(vertex);
        });

    return true;
}

bool yang::ConeShape::Render(IGraphics* pGraphics)
{
    return true;
}

#ifdef DEBUG
bool yang::ConeShape::DebugDraw(IGraphics* pGraphics)
{
#ifdef YANG_COLLIDER_DEBUG_VERTICES
    for (auto& vertex : m_debugVertices)
    {
        pGraphics->FillCircle(vertex, 2, {255,0,0,255});
    }
#endif

    yang::FVec2 toDraw[4];
    yang::Matrix m;
    m.Scale({ m_radius, m_radius }, m_vertices[0]);

    std::transform(m_vertices, m_vertices + 4, toDraw, [m](yang::FVec2 v)
        {
            return m.TransformPoint(v);
        });

    // Chaaaange!
    return pGraphics->DrawPolygon({ toDraw[0], toDraw[1], toDraw[2], toDraw[3] }, m_color);
}
#endif

bool yang::ConeShape::Collide(IShape* pOther)
{
    return pOther->Collide(this);
}

bool yang::ConeShape::Collide(CircleShape* pOther)
{
    // Early out: if circles don't collide - false. It is not necessary but easy to calculate, so if circles don't collide, just early out
    if ((m_vertices[0] - pOther->GetCenter()).SqrdLength() > (m_radius + pOther->GetRadius()) * (m_radius + pOther->GetRadius()))
    {
        return false;
    }

    // if cone center or other 2 cone points are inside the circle then they're definitely colliding.
    //  1.) We have to check the cone center, because the next algortihm won't detect that
    //  2.) We have to check the other two points, because it is also possible that we won't detect some cases
    if (pOther->Contains(m_vertices[0]) || pOther->Contains(m_vertices[1]) || pOther->Contains(m_vertices[3]))
    {
        return true;
    }

#ifdef YANG_COLLIDER_DEBUG_VERTICES
    m_debugVertices.clear();
#endif

    auto it = std::find_if(m_vertices + 1, m_vertices + 4, [this, pOther](yang::FVec2 vertex)
        {
            if (vertex == m_vertices[2]) return false;

            yang::FVec2 result = vertex - m_vertices[0];
            result *= pOther->GetRadius();
            result = { -result.y, result.x };
#ifdef YANG_COLLIDER_DEBUG_VERTICES
            m_debugVertices.push_back(result + pOther->GetCenter());
            m_debugVertices.push_back(-result + pOther->GetCenter());
#endif
            return Contains(result + pOther->GetCenter()) || Contains(-result + pOther->GetCenter());
        });

    // Closest point on circle (the point on the line between two centers that is also on the circle)
    yang::FVec2 closestPointOnCircle = m_vertices[0] - pOther->GetCenter();
    closestPointOnCircle.Normalize();
    closestPointOnCircle *= pOther->GetRadius();
    yang::FVec2 perpendicularOne = { -closestPointOnCircle.y, closestPointOnCircle.x };
    yang::FVec2 perpendicularTwo = -perpendicularOne;
    closestPointOnCircle += pOther->GetCenter();

    if ((it != m_vertices + 4)) return true;

    // Border unit vector
    yang::FVec2 borderOne = m_vertices[1] - m_vertices[0];
    // Border unit vector
    yang::FVec2 borderTwo = m_vertices[3] - m_vertices[0];
    // Center unit vector
    yang::FVec2 center = m_vertices[2] - m_vertices[0];
    
    perpendicularOne += pOther->GetCenter();
    perpendicularTwo += pOther->GetCenter();

    float radius = (perpendicularOne - m_vertices[0]).Length();
    yang::FVec2 comparable2 = borderOne * radius + m_vertices[0];
    yang::FVec2 comparable3 = borderTwo * radius + m_vertices[0];

#ifdef YANG_COLLIDER_DEBUG_VERTICES
    m_debugVertices.push_back(perpendicularOne);
    m_debugVertices.push_back(perpendicularTwo);
    m_debugVertices.push_back(comparable2);
    m_debugVertices.push_back(comparable3);
#endif

    return pOther->Contains(comparable2) || pOther->Contains(comparable3);
}

bool yang::ConeShape::Collide(RectangleShape* pOther)
{
    for (auto& vertex : pOther->GetVertices())
    {
        if (Contains(vertex))
        {
            return true;
        }
    }

    yang::Matrix m;
    m.Scale({ m_radius, m_radius }, m_vertices[0]);

    auto it = std::find_if(m_vertices, m_vertices + 4, [this, pOther, m](yang::FVec2 vertex)
        {
            yang::FVec2 result = m.TransformPoint(vertex);
            return pOther->Contains(result);
        });

    return it != m_vertices + 4;
}

bool yang::ConeShape::Collide(ConeShape* pOther)
{
    // Whatever. Let's suppose cones don't collide haha
    return false;
}

bool yang::ConeShape::Contains(FVec2 point)
{
    // If our circle doesn't contain the point then definitely false
    if ((m_vertices[0] - point).SqrdLength() > m_radius * m_radius)
    {
        return false;
    }

    // Border unit vector
    yang::FVec2 border = m_vertices[1] - m_vertices[0];
    // Center unit vector
    yang::FVec2 center = m_vertices[2] - m_vertices[0];
    // Unit vector to the point we are checking
    yang::FVec2 comparable = point - m_vertices[0];
    comparable.Normalize();

    // If dot product of comparable with center is greater than dot product of comparable with border - it is inside our cone.
    return comparable * center > border * center;
}

void yang::ConeShape::Update(TransformComponent* pTransform)
{
    CircleShape::Update(pTransform);

    m_orientationAngle = pTransform->GetRotation();

    UpdateVertices();
}

void yang::ConeShape::UpdateVertices()
{
    yang::Matrix m;
    m.Translate(m_localCenter).Rotate(m_orientationAngle).Translate(m_center);

    std::transform(m_initialVertices, m_initialVertices + 4, m_vertices, [m, this](yang::FVec2 vertex)
        {
            return m.TransformPoint(vertex);
        });
}

void yang::ConeShape::SetOrientation(float orientation)
{
    m_orientationAngle = std::fmod(orientation + 2 * Math::kPi, 2 * Math::kPi);
}

void yang::ConeShape::SetAngle(float angle)
{
    m_fieldOfViewAngle = std::fmod(angle, 2 * Math::kPi);
}
