#include <cmath>
#include <editor/EditorCamera.hpp>

namespace editor {
    constexpr float radiansCoof = 3.141592f / 180.0f;

    mat4 translate(mat4 const& matrix, robot2D::vec3f const& to) {
        mat4 mat{matrix};

        float vec4[4][1] = {
                {to.x},
                {to.y},
                {to.z},
                {1.f}
        };

        float res[4][1];
        for(short it=0; it < 4; ++it){
            for(short ij=0; ij < 1; ++ij){
                res[it][ij] = 0;
                for(short ik = 0; ik < 4; ++ik)
                    res[it][ij] += mat(it, ik) * vec4[ik][ij];
            }
        }

        mat(0, 3) = res[0][0];
        mat(1, 3) = res[1][0];
        mat(2, 3) = res[2][0];
        mat[15] = res[3][0];

        mat = mat.transpose();
        return mat;
    }

    mat4 rotate(mat4 const& matrix, float const& angle, const robot2D::vec3f& axis) {
        return {};
    }

    mat4 scale(mat4 const& matrix, const robot2D::vec3f& scaleFactor) {
        mat4 scaleMatrix{};
        scaleMatrix[0] = matrix[0] * scaleFactor.x;
        scaleMatrix[5] = matrix[5] * scaleFactor.x;
        scaleMatrix[10] = matrix[10] * scaleFactor.x;
        auto old = matrix;
        return scaleMatrix * old;
    }

    float to_radians(const float& degrees) {
        return degrees * radiansCoof;
    }

    mat4 projectionPerspective(float zoom, float aspectRatio, float zNear, float zFar) {
        float y_scale = (float) ((1.f / std::tan(to_radians(zoom / 2.f))) * aspectRatio);
        float x_scale = y_scale / aspectRatio;
        float frustum_length = zFar - zNear;

        mat4 matrix;
        matrix[0] = x_scale;
        matrix[5] = y_scale;
        matrix[10] = -((zFar + zNear) / frustum_length);;
        matrix[11] = -1.f;
        matrix[14] = -((2 * zNear * zFar) / frustum_length);;
        matrix[15] = 0.f;
        return matrix;
    }

    mat4 ortographPerspective() {}

    robot2D::vec3f normalize(const robot2D::vec3f& vector) {
        float inv_length = 1.F /
                static_cast<float>(std::sqrt(vector.x * vector.x +
                                                     vector.y * vector.y +
                                                     vector.z * vector.z));
        return {
            vector.x * inv_length,
            vector.y * inv_length,
            vector.z * inv_length
        };
    }

    robot2D::vec3f cross(const robot2D::vec3f& left, const robot2D::vec3f& right) {
        return {
            left.y * right.z - left.z * right.y,
            left.z * right.x - left.x * right.z,
            left.x * right.y - left.y * right.x
        };
    }

    mat4 lookAt(const robot2D::vec3f pos, const robot2D::vec3f target, const robot2D::vec3f up) {
        robot2D::vec3f zaxis = normalize(pos - target);
        robot2D::vec3f xaxis = normalize(cross(normalize(up), zaxis));
        robot2D::vec3f yaxis = cross(zaxis, xaxis);

        mat4 translation;
        translation[3] = -pos.x;
        translation[7] = -pos.y;
        translation[11] = -pos.z;

        mat4 rotation;
        rotation[0] = xaxis.x;
        rotation[1] = xaxis.y;
        rotation[2] = xaxis.z;
        rotation[4] = yaxis.x;
        rotation[5] = yaxis.y;
        rotation[6] = yaxis.z;
        rotation[8] = zaxis.x;
        rotation[9] = zaxis.y;
        rotation[10] = zaxis.z;

        return rotation * translation;
    }

    mat4 crossMatrix(const mat4& mat, const robot2D::vec3f& vec) {
        float vec4[4][1] = {
                {vec.x},
                {vec.y},
                {vec.z},
                {1.f}
        };

        float res[4][1];
        for(short it=0; it < 4; ++it){
            for(short ij=0; ij < 1; ++ij){
                res[it][ij] = 0;
                for(short ik = 0; ik < 4; ++ik) {
                   //res[it][ij] += mat.mat[it][ik] * vec4[ik][ij];
                }
            }
        }

        return mat4();
    }


    constexpr float SPEED       =  520.5f;
    constexpr  float SENSITIVITY =  0.01f;

    EditorCamera::EditorCamera() {
        m_projectionMatrix = projectionPerspective(45.0f, 1.778f, 0.1f, 1000.0f);
        m_front = {0.f, 0.f, -1.f};

        m_position = {33.f, 150.f, -910.F};
        m_worldUp = {0.f, 1.f, 0.f};
        m_zoom = 45.F;
        m_yaw = 45.F;
        m_pitch = 0.F;

        cameraMove = SPEED;
        cameraSensitive = SENSITIVITY;

        robot2D::vec3f front;

        front.x = cos(to_radians(m_yaw)) * cos(to_radians(m_pitch));
        front.y = sin(to_radians(m_pitch));
        front.z = sin(to_radians(m_yaw)) * cos(to_radians(m_pitch));

        m_front = normalize(front);
        m_right = normalize(cross(m_front, m_worldUp));
        m_up = normalize(cross(m_right, m_front));
    }

    float lastX = 1280, lastY = 760;

    void EditorCamera::handleEvents(const robot2D::Event& event) {
        if(event.type == robot2D::Event::MouseMoved
            && robot2D::Keyboard::isKeyPressed(robot2D::Key(robot2D::Key::LEFT_CONTROL))) {
            float xoffset = event.move.x - lastX;
            float yoffset = lastY - event.move.y; // reversed since y-coordinates go from bottom to top

            lastX = event.move.x;
            lastY = event.move.y;
            processMouse({xoffset, yoffset});
        }

        if(event.type == robot2D::Event::MouseWheel) {
            processMouseScroll(event.wheel.scroll_y);
        }
    }

    void EditorCamera::update(float dt) {
        float velocity = cameraMove * dt;

        if(robot2D::Keyboard::isKeyPressed(robot2D::Key::W))
            m_position += m_front * velocity;
        if(robot2D::Keyboard::isKeyPressed(robot2D::Key::S))
            m_position -= m_front * velocity;
        if(robot2D::Keyboard::isKeyPressed(robot2D::Key::A))
            m_position -= m_right * velocity;
        if(robot2D::Keyboard::isKeyPressed(robot2D::Key::D))
            m_position += m_right * velocity;
    }

    float& EditorCamera::getZoom() {
        return m_zoom;
    }

    const float& EditorCamera::getZoom() const {
        return m_zoom;
    }

    void EditorCamera::processMouse(robot2D::vec2f offset, bool constraintPitch) {
        cameraSensitive = 0.1f;

        offset.x *= cameraSensitive;
        offset.y *= cameraSensitive;

        m_yaw += offset.x;
        m_pitch += offset.y;

        if(constraintPitch) {
            if (m_pitch > 90.F)
                m_pitch = 90.F;
            if (m_pitch < -89.f)
                m_pitch = -89.f;
        }

        robot2D::vec3f front;

        front.x = cos(to_radians(m_yaw)) * cos(to_radians(m_pitch));
        front.y = sin(to_radians(m_pitch));
        front.z = sin(to_radians(m_yaw)) * cos(to_radians(m_pitch));

        m_front = normalize(front);
        m_right = normalize(cross(m_front, m_worldUp));
        m_up = normalize(cross(m_right, m_front));
    }

    void EditorCamera::processMouseScroll(float offset) {
        m_zoom -= offset;

        if (m_zoom < 1.0f)
            m_zoom = 1.0f;
        if (m_zoom > 45.0f)
            m_zoom = 45.0f;
    }

    const Matrix3D& EditorCamera::getViewMatrix() const {
        mat4 eye = lookAt(m_position, m_position + m_front, m_up);
        m_cameraViewMatrix = eye.transpose();
        return m_cameraViewMatrix;
    }


}