#pragma once

#include <string>
#include <robot2D/Core/Vector2.hpp>
#include <robot2D/Core/Vector3.hpp>

namespace editor::ui {
    void drawVec2Control(const std::string& label, robot2D::vec2f& values,
                                float resetValue = 0.0f, float columnWidth = 100.0f);

    void drawVec3Control(const std::string& label, robot2D::vec3f& values,
                                float resetValue = 0.0f, float columnWidth = 100.0f);
}