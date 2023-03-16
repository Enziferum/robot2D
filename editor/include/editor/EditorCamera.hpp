#pragma once
#include <cmath>

#include <robot2D/Core/Event.hpp>

#include <robot2D/Core/Vector3.hpp>
#include <robot2D/Core/Vector2.hpp>
#include <robot2D/Graphics/Matrix3D.hpp>
#include <robot2D/Graphics/Math3D.hpp>

namespace editor {

    class EditorCamera {
    public:
        EditorCamera();
        ~EditorCamera() = default;

        void setViewportSize(robot2D::vec2f newSize);

        void handleEvents(const robot2D::Event& event);
        void update(robot2D::vec2f mousePos, float deltaTime);

        robot2D::mat4& getProjectionMatrix() { return m_projectionMatrix; }
        const robot2D::mat4& getProjectionMatrix() const  { return m_projectionMatrix; }

        robot2D::mat4 getViewMatrix() const;

        const robot2D::vec3f& getPosition() const { return m_position; }
    private:
        void updateView();

        void mousePan(robot2D::vec2f delta);
        void mouseRotate(robot2D::vec2f delta);
        void mouseZoom(float delta);

        std::pair<float, float> PanSpeed() const;
        float ZoomSpeed() const;

        robot2D::vec3f calculatePosition();
        robot2D::quat getOrientation() const;
        robot2D::vec3f getForwardOrientation() const;
        robot2D::vec3f getUpOrientation() const;
        robot2D::vec3f getRightOrientation() const;
    private:
        robot2D::mat4 m_projectionMatrix;
        robot2D::mat4 m_viewMatrix;
        robot2D::vec3f m_focalPoint;
        robot2D::vec3f m_position;

        robot2D::vec2f m_mousePos;
        robot2D::vec2f m_viewportSize;
        float m_distance = 10.f;

        float m_yaw;
        float m_pitch;
    };
}