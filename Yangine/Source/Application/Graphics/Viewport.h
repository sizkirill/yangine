#pragma once
#include <Utils/Vector2.h>
#include <Utils/Rectangle.h>

namespace yang
{
    class Viewport
    {
    public:
        Viewport();
        Viewport(yang::FVec2 center, yang::FVec2 dimensions, float scale, yang::FVec2 screenCenter);
        void SetScale(float scale) { m_viewportScale = scale; }
        float GetScale() const { return m_viewportScale; }

        auto GetCenter() const { return m_center; }
        auto GetDimensions() const { return m_dimensions; }

        yang::FVec2 ToScreenCoordinates(yang::FVec2 point) const;
        yang::FVec2 ToWorldCoordinates(yang::FVec2 pointOnScreen) const;
        yang::FVec2 Scale(yang::FVec2 dimensions) const;

        yang::FVec2 TopLeft() const;
        yang::FVec2 BottomRight() const;

        void FocusOn(yang::FVec2 point);

        void Move(yang::FVec2 amount);
    private:
        yang::FVec2 m_center;
        yang::FVec2 m_dimensions;
        float m_viewportScale;
        yang::FVec2 m_screenCenter;

    };
}