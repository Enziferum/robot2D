#pragma once
#include <robot2D/Core/Event.hpp>

#include <robot2D/Core/Vector3.hpp>
#include <robot2D/Core/Vector2.hpp>
#include <robot2D/Graphics/Matrix3D.hpp>

namespace editor {

    using namespace robot2D;

    mat4 translate(mat4 const& matrix, robot2D::vec3f const& to);

    mat4 rotate(mat4 const& matrix, float const& angle, const robot2D::vec3f& axis);

    mat4 scale(mat4 const& matrix, const robot2D::vec3f& scaleFactor);

    mat4 projectionPerspective(float zoom, float aspectRatio, float zNear, float zFar);

    mat4 ortographPerspective();

    mat4 lookAt(const robot2D::vec3f pos, const robot2D::vec3f target, const robot2D::vec3f up);

    mat4 crossMatrix(const mat4& mat, const robot2D::vec3f& vec);

    class EditorCamera {
    public:
        EditorCamera();
        ~EditorCamera() = default;

        void handleEvents(const robot2D::Event& event);
        void update(float deltaTime);

        Matrix3D& getProjectionMatrix() { return m_projectionMatrix; }
        Matrix3D& getCameraView() { return m_cameraViewMatrix; }

        const Matrix3D& getProjectionMatrix() const  { return m_projectionMatrix; }
        const Matrix3D& getViewMatrix() const;

        float& getZoom();
        const float& getZoom() const;

        const robot2D::vec3f& getPosition() const { return m_position; }
        const robot2D::vec3f& getFront() const {return m_front; }
        const robot2D::vec3f& getUp() const {return m_up; }
    private:
        void processMouse(robot2D::vec2f position,
                          bool constraintPitch = true);
        void processMouseScroll(float offset);
    private:
        Matrix3D m_projectionMatrix;
        mutable Matrix3D m_cameraViewMatrix;

        robot2D::vec3f m_position;
        robot2D::vec3f m_worldUp;
        robot2D::vec3f m_front;
        robot2D::vec3f m_up;
        robot2D::vec3f m_right;

        float cameraSensitive;
        float cameraMove;

        //Eulers angles
        float m_yaw;
        float m_pitch;
        float m_zoom;
    };
}