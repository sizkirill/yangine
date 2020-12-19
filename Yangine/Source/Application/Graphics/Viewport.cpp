#include "Viewport.h"
#include "Application/ApplicationConstants.h"

using namespace yang;

yang::Viewport::Viewport(yang::FVec2 center, yang::FVec2 dimensions, float scale, FVec2 screenCenter)
    :m_center(center)
    ,m_dimensions(dimensions)
    ,m_viewportScale(scale)
    ,m_screenCenter(screenCenter)
{
}

yang::FVec2 yang::Viewport::ToScreenCoordinates(yang::FVec2 point) const
{
    return m_screenCenter + (point - m_center) * m_viewportScale;
}

yang::FVec2 yang::Viewport::ToWorldCoordinates(yang::FVec2 pointOnScreen) const
{
    return m_center + (pointOnScreen - m_screenCenter) / m_viewportScale;
}

yang::FVec2 yang::Viewport::Scale(yang::FVec2 dimensions) const
{
    return dimensions * m_viewportScale;
}

yang::FVec2 yang::Viewport::TopLeft() const
{
    return ToWorldCoordinates({ 0,0 });
}

yang::FVec2 yang::Viewport::BottomRight() const
{
    return ToWorldCoordinates(m_dimensions);
}

void yang::Viewport::FocusOn(yang::FVec2 point)
{
    m_center = point;
}

void yang::Viewport::Move(yang::FVec2 amount)
{
    m_center += amount;
}


yang::Viewport::Viewport()
    :m_center(kWindowWidth/2.f, kWindowHeight/2.f)
    ,m_dimensions((float)kWindowWidth, (float)kWindowHeight)
    ,m_viewportScale(1.f)
    ,m_screenCenter(kWindowWidth / 2.f, kWindowHeight / 2.f)
{
}
