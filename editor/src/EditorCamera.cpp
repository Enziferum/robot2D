/*********************************************************************
   (c) Alex Raag 2026
   https://github.com/Enziferum
   robot2D - Zlib license.
   This software is provided 'as-is', without any express or
   implied warranty. In no event will the authors be held
   liable for any damages arising from the use of this software.
   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute
   it freely, subject to the following restrictions:
   1. The origin of this software must not be misrepresented;
   you must not claim that you wrote the original software.
   If you use this software in a product, an acknowledgment
   in the product documentation would be appreciated but
   is not required.
   2. Altered source versions must be plainly marked as such,
   and must not be misrepresented as being the original software.
   3. This notice may not be removed or altered from any
   source distribution.
*********************************************************************/

#include <cmath>
#include <algorithm>
#include <editor/EditorCamera.hpp>
#include <robot2D/Core/Window.hpp>

#ifdef USE_GLM
    #define GLM_ENABLE_EXPERIMENTAL
    #include <glm/gtx/quaternion.hpp>
#endif

namespace editor {
#ifdef USE_GLM
    robot2D::Matrix3D fromGLM(const glm::mat4& glmMatrix) {
        return {
            glmMatrix[0][0], glmMatrix[0][1], glmMatrix[0][2], glmMatrix[0][3],
            glmMatrix[1][0], glmMatrix[1][1], glmMatrix[1][2], glmMatrix[1][3],
            glmMatrix[2][0], glmMatrix[2][1], glmMatrix[2][2], glmMatrix[2][3],
            glmMatrix[3][0], glmMatrix[3][1], glmMatrix[3][2], glmMatrix[3][3]
        };
    }
#endif

    bool isEqual(const robot2D::Matrix3D& left, const robot2D::Matrix3D& right) {
        for(int it = 0; it < 16; ++it)
            if(left[it] != right[it])
                return false;
        return true;
    }

    constexpr float Fov = 45.F;
    constexpr float zNear = 0.1f;
    constexpr float zFar = 1000.f;
    IEditorCamera::~IEditorCamera() = default;

    EditorCamera2D::EditorCamera2D() :
        IEditorCamera(EditorCameraType::Orthographic) {
    }


    void EditorCamera2D::setViewportSize(robot2D::vec2f newSize) {
        if(!std::isfinite(newSize.x) || !std::isfinite(newSize.y) || newSize.x < 1.f || newSize.y < 1.f)
            return;
        const bool firstSize = m_viewportSize.x == 0.f;
        m_viewportSize = newSize;
        if(firstSize)
            m_view.setCenter(newSize * 0.5f);
        setZoom(m_zoom);
    }

    void EditorCamera2D::handleEvents(const robot2D::Event& event) {

        if(event.getIf<robot2D::Event::MouseWheel>() && m_leftCntrlPressed) {
            setZoom(m_zoom * std::pow(1.1f, event.get<robot2D::Event::MouseWheel>().scroll_y));
        }

        if(event.getIf<robot2D::Event::LostFocus>())
            m_leftCntrlPressed = false;

        if(event.getIf<robot2D::Event::KeyPressed>() &&
            event.get<robot2D::Event::KeyboardEvent>().code == robot2D::Key::LEFT_CONTROL)
            m_leftCntrlPressed = true;

        if(event.getIf<robot2D::Event::KeyReleased>() &&
            event.get<robot2D::Event::KeyboardEvent>().code == robot2D::Key::LEFT_CONTROL)
            m_leftCntrlPressed = false;
    }

    void EditorCamera2D::update(robot2D::vec2f mousePos, [[maybe_unused]] float deltaTime) {
        constexpr float deltaOffset = 0.5F;

        robot2D::vec2f delta = (mousePos - m_mousePos) * (deltaOffset / m_zoom);
        m_mousePos = mousePos;

        if(m_window && m_window -> isMouseButtonPressed(robot2D::Mouse::MouseMiddle) && m_leftCntrlPressed) {
            auto center = m_view.getCenter();
            center.x += delta.x * 2.F;
            center.y += delta.y * 2.F;
            m_view.setCenter(center);
        }
    }

    void EditorCamera2D::resetDefaults() {
        setZoom(1.f);
        m_view.setCenter(m_viewportSize * 0.5f);
    }


    bool EditorCamera2D::setZoom(float zoom) {
        if(!std::isfinite(zoom) || zoom <= 0.f || m_viewportSize.x <= 0.f || m_viewportSize.y <= 0.f)
            return false;
        m_zoom = std::clamp(zoom, 0.05f, 8.f);
        // Use the scalar overload: it keeps View's cached rectangle in sync.
        m_view.setSize(m_viewportSize.x / m_zoom, m_viewportSize.y / m_zoom);
        return true;
    }

    bool EditorCamera2D::frameBounds(const robot2D::FloatRect& bounds) {
        if(!std::isfinite(bounds.lx) || !std::isfinite(bounds.ly) || !std::isfinite(bounds.width)
            || !std::isfinite(bounds.height) || bounds.width < 0.f || bounds.height < 0.f)
            return false;
        const auto center = bounds.centerPoint();
        if(!std::isfinite(center.x) || !std::isfinite(center.y))
            return false;
        const float zoom = std::min(m_viewportSize.x / std::max(1.f, bounds.width) / 1.25f,
                m_viewportSize.y / std::max(1.f, bounds.height) / 1.25f);
        if(!setZoom(zoom))
            return false;
        m_view.setCenter(center);
        return true;
    }

    EditorCamera::EditorCamera() :
        IEditorCamera(EditorCameraType::Perspective),
        m_viewportSize{},
        m_distance{10.f},
        m_yaw{0.f},
        m_pitch{0.f} {
        updateView();
    }

    void EditorCamera::setViewportSize(robot2D::vec2f newSize) {
        m_viewportSize = newSize;
        m_projectionMatrix = robot2D::projectionPerspective(Fov, newSize.x / newSize.y, zNear, zFar);
    }

    void EditorCamera::handleEvents(const robot2D::Event& event) {
        if(event.type() == robot2D::Event::Type::MouseWheel) {
            float delta = event.get<robot2D::Event::MouseWheelEvent>().scroll_y * 0.1f;
            mouseZoom(delta);
            updateView();
        }
    }

    void EditorCamera::update(robot2D::vec2f mousePos, float dt) {

        if(m_window && m_window -> isKeyPressed(robot2D::Key::LEFT_CONTROL)) {
            robot2D::vec2f delta = (mousePos - m_mousePos) * 0.003f;
            m_mousePos = mousePos;

            if(m_window -> isMouseButtonPressed(robot2D::Mouse::MouseMiddle))
                mousePan(delta);
            if(m_window -> isMouseButtonPressed(robot2D::Mouse::MouseLeft))
                mouseRotate(delta);
            if(m_window -> isMouseButtonPressed(robot2D::Mouse::MouseRight))
                mouseZoom(delta.y);
        }

        updateView();
    }


    std::pair<float, float> EditorCamera::PanSpeed() const {
        float x = std::min(m_viewportSize.x / 1000.0f, 2.4f); // max = 2.4f
        float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

        float y = std::min(m_viewportSize.y / 1000.0f, 2.4f); // max = 2.4f
        float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

        return { xFactor, yFactor };
    }

    void EditorCamera::mousePan(robot2D::vec2f delta) {
        auto [xSpeed, ySpeed] = PanSpeed();
        m_focalPoint += -getRightOrientation() * delta.x * xSpeed * m_distance;
        m_focalPoint += getUpOrientation() * delta.y * ySpeed * m_distance;
    }

    void EditorCamera::mouseRotate(robot2D::vec2f delta) {
        float yawSign = getUpOrientation().y < 0 ? -1.0f : 1.0f;
        m_yaw += yawSign * delta.x * 0.8f;
        m_pitch += delta.y * 0.8f;
    }

    float EditorCamera::ZoomSpeed() const {
        float distance = m_distance * 0.2f;
        distance = std::max(distance, 0.0f);
        float speed = distance * distance;
        speed = std::min(speed, 100.0f); // max speed = 100
        return speed;
    }

    void EditorCamera::mouseZoom(float delta) {
        m_distance -= delta * ZoomSpeed();
        if(m_distance < 1.0f) {
            m_focalPoint += getForwardOrientation();
            m_distance = 1.0f;
        }
    }

    robot2D::vec3f EditorCamera::calculatePosition() {
        return m_focalPoint - getForwardOrientation() * m_distance;
    }

    robot2D::quat EditorCamera::getOrientation() const {
        return {{-m_pitch, -m_yaw, 0.f}};
    }

    robot2D::vec3f EditorCamera::getForwardOrientation() const {
        return rotate(getOrientation(), {0.f, 0.0f, -1.f});
    }

    robot2D::vec3f EditorCamera::getUpOrientation() const {
        return rotate(getOrientation(), {0.f, 1.0f, 0.f});
    }

    robot2D::vec3f EditorCamera::getRightOrientation() const {
        return rotate(getOrientation(), {1.f, 0.0f, 0.f});
    }


    void EditorCamera::updateView() {
        m_position = calculatePosition();
        robot2D::quat orientation = getOrientation();
        m_viewMatrix = robot2D::translate(robot2D::mat4{}, m_position) * toMat4(orientation);
        m_viewMatrix = matrixInverse(m_viewMatrix);
#ifdef USE_GLM
        glm::quat q = glm::quat{{-m_pitch, -m_yaw, 0.f}};
        glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
        m_ViewMatrix = glm::translate(glm::mat4(1.0f), {m_position.x, m_position.y, m_position.z}) * glm::toMat4(q);
        m_ViewMatrix = glm::inverse(m_ViewMatrix);

        m_viewMatrix = fromGLM(m_ViewMatrix);
#endif
    }
}