#include <cmath>
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
                glmMatrix[0][0],glmMatrix[0][1],glmMatrix[0][2],glmMatrix[0][3],
                glmMatrix[1][0],glmMatrix[1][1],glmMatrix[1][2],glmMatrix[1][3],
                glmMatrix[2][0],glmMatrix[2][1],glmMatrix[2][2],glmMatrix[2][3],
                glmMatrix[3][0],glmMatrix[3][1],glmMatrix[3][2],glmMatrix[3][3]
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

    EditorCamera2D::EditorCamera2D(): IEditorCamera(EditorCameraType::Orthographic) {}


    void EditorCamera2D::setViewportSize(robot2D::vec2f newSize) {
        m_view.reset({0, 0, newSize.x, newSize.y});
    }

    void EditorCamera2D::handleEvents(const robot2D::Event& event) {

    }

    void EditorCamera2D::update(robot2D::vec2f mousePos, float deltaTime) {

    }





    EditorCamera::EditorCamera():
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
        if(event.type == robot2D::Event::MouseWheel) {
            float delta = event.wheel.scroll_y * 0.1f;
            mouseZoom(delta);
            updateView();
        }
    }

    void EditorCamera::update(robot2D::vec2f mousePos, float dt) {

        if(robot2D::Keyboard::isKeyPressed(robot2D::Key::LEFT_CONTROL)) {
            robot2D::vec2f delta = (mousePos - m_mousePos) * 0.003f;
            m_mousePos = mousePos;

            if(robot2D::isMousePressed(robot2D::Mouse::MouseMiddle))
                mousePan(delta);
            if(robot2D::isMousePressed(robot2D::Mouse::MouseLeft))
                mouseRotate(delta);
            if(robot2D::isMousePressed(robot2D::Mouse::MouseRight))
                mouseZoom(delta.y);
        }

        updateView();
    }


    std::pair<float, float> EditorCamera::PanSpeed() const
    {
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

    float EditorCamera::ZoomSpeed() const
    {
        float distance = m_distance * 0.2f;
        distance = std::max(distance, 0.0f);
        float speed = distance * distance;
        speed = std::min(speed, 100.0f); // max speed = 100
        return speed;
    }

    void EditorCamera::mouseZoom(float delta) {
        m_distance -= delta * ZoomSpeed();
        if (m_distance < 1.0f) {
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