/*********************************************************************
(c) Alex Raag 2023
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

#pragma once
#include <cmath>
#include <memory>

#include <robot2D/Core/Event.hpp>

#include <robot2D/Core/Vector3.hpp>
#include <robot2D/Core/Vector2.hpp>
#include <robot2D/Graphics/Matrix3D.hpp>
#include <robot2D/Graphics/Math3D.hpp>
#include <robot2D/Graphics/FrameBuffer.hpp>

#include <robot2D/Graphics/View.hpp>

namespace editor {

    enum class EditorCameraType {
        Orthographic,
        Perspective
    };

    class IEditorCamera {
    public:
        using Ptr = std::shared_ptr<IEditorCamera>;

        explicit IEditorCamera(EditorCameraType type): m_type(type) {}
        virtual ~IEditorCamera() = 0;

        virtual void setViewportSize(robot2D::vec2f newSize) = 0;
        virtual void handleEvents(const robot2D::Event& event) = 0;
        virtual void update(robot2D::vec2f mousePos, float deltaTime) = 0;

        virtual robot2D::vec2f convertPixelToCoords(robot2D::vec2f mousePos) const {}
        void setFrameBuffer(robot2D::FrameBuffer::Ptr ptr) { m_framebuffer = ptr; }
        void setViewportBounds(robot2D::vec2f bounds) { m_bounds = bounds; }

        EditorCameraType getType() const { return m_type; }

        virtual void resetDefaults() {}
        virtual float getZoom() const { return 0.f; }

        robot2D::mat4& getProjectionMatrix() { return m_projectionMatrix; }
        const robot2D::mat4& getProjectionMatrix() const  { return m_projectionMatrix; }
        const robot2D::vec3f& getPosition() const { return m_position; }
        robot2D::mat4 getViewMatrix() const { return m_viewMatrix; }
        const robot2D::View& getView() const { return m_view; }
        robot2D::View& getView() { return m_view; }
    protected:
        /// Overhead ///
        robot2D::mat4 m_projectionMatrix;
        robot2D::mat4 m_viewMatrix;
        robot2D::vec3f m_position;
        robot2D::View m_view;
        robot2D::FrameBuffer::Ptr m_framebuffer;
        robot2D::vec2f m_bounds;
    private:
        EditorCameraType m_type;
    };


    class EditorCamera2D: public IEditorCamera {
    public:
        EditorCamera2D();
        ~EditorCamera2D() override = default;

        void setViewportSize(robot2D::vec2f newSize) override;
        void handleEvents(const robot2D::Event& event) override;
        void update(robot2D::vec2f mousePos, float deltaTime) override;
        robot2D::vec2f convertPixelToCoords(robot2D::vec2f mousePos) const override {
            mousePos -= m_bounds;
            return mapPixelToCoords(mousePos);
        }

        void resetDefaults() override;
        float getZoom() const override { return m_zoom; }
    private:
        robot2D::vec2f mapPixelToCoords(const robot2D::vec2f& point) const {
            robot2D::vec2f normalized;
            auto port = getViewport(m_framebuffer -> getSpecification().size);
            robot2D::FloatRect viewport = {
                    static_cast<float>(port.lx),
                    static_cast<float>(port.lx),
                    static_cast<float>(port.width),
                    static_cast<float>(port.height)
            };
            normalized.x = -1.f + 2.f * (static_cast<float>(point.x) - viewport.lx) / viewport.width;
            normalized.y = 1.f - 2.f * (static_cast<float>(point.y) - viewport.ly) / viewport.height;

            // Then transform by the inverse of the view matrix
            return m_view.getInverseTransform().transformPoint(normalized);
        }

        robot2D::IntRect getViewport(robot2D::vec2i size) const {
            float width  = static_cast<float>(size.x);
            float height = static_cast<float>(size.y);
            const robot2D::FloatRect& viewport = m_view.getViewport();

            return {
                    static_cast<int>(0.5F + width  * viewport.lx),
                    static_cast<int>(0.5F + height * viewport.ly),
                    static_cast<int>(0.5F + width  * viewport.width),
                    static_cast<int>(0.5F + height * viewport.height)
            };
        }

    private:
        bool m_leftCntrlPressed{false};
        float m_zoom = 0.F;
        robot2D::vec2f m_mousePos;
    };

    class EditorCamera: public IEditorCamera {
    public:
        EditorCamera();
        ~EditorCamera() override = default;

        void setViewportSize(robot2D::vec2f newSize) override;
        void handleEvents(const robot2D::Event& event) override;
        void update(robot2D::vec2f mousePos, float deltaTime) override;
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

        robot2D::vec3f m_focalPoint;

        robot2D::vec2f m_mousePos;
        robot2D::vec2f m_viewportSize;
        float m_distance = 10.f;

        float m_yaw;
        float m_pitch;
    };
}