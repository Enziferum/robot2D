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

#include <robot2D/Core/Event.hpp>
#include <robot2D/Graphics/Drawable.hpp>
#include <robot2D/Graphics/Transformable.hpp>
#include <robot2D/Graphics/Color.hpp>
#include <robot2D/Graphics/Rect.hpp>
#include <robot2D/Graphics/RenderTarget.hpp>
#include <robot2D/Graphics/Texture.hpp>
#include <robot2D/Graphics/Sprite.hpp>

#include "EventBinder.hpp"
#include "EditorCamera.hpp"
#include "Quad.hpp"
#include <functional>
#include "Components.hpp"

namespace editor {

    class DebugDrawable {
    public:
        virtual ~DebugDrawable() = 0;

        void debugDraw(robot2D::RenderTarget& target, robot2D::RenderStates states) const {
            std::array<robot2D::Transform, 4> quads;
            quads[0].translate(m_debugRect.lx, m_debugRect.ly);
            quads[0].scale(m_debugRect.width, m_borderSize);

            quads[1].translate(m_debugRect.lx, m_debugRect.ly);
            quads[1].scale(m_borderSize, m_debugRect.height);

            quads[2].translate(m_debugRect.lx, m_debugRect.ly + m_debugRect.height);
            quads[2].scale(m_debugRect.width, m_borderSize);

            quads[3].translate(m_debugRect.lx + m_debugRect.width, m_debugRect.ly);
            quads[3].scale(m_borderSize, m_debugRect.height);

            for(const auto& transform: quads) {
                states.transform = transform;
                states.color = m_borderColor;
                target.draw(states);
            }
        }

    protected:
        mutable robot2D::FloatRect m_debugRect;
        float m_borderSize = 1.f;
        robot2D::Color m_borderColor = robot2D::Color::Green;
    };


    class Guizmo2D : public robot2D::Drawable {
    public:
        enum class Operation {
            Move, Scale, Rotate
        };
    private:
        class Manipulator : public robot2D::Drawable, DebugDrawable {
        public:
            enum class Axis {
                X, Y, BOTH
            };

            Manipulator(Axis axis, robot2D::Color color) : m_color{color}, m_axis{axis} {
                if(axis == Axis::X) {
                    m_rect.width = 20;
                    m_rect.height = 5;
                    m_sprite.setSize({10, 50});
                }
                else if(axis == Axis::Y) {
                    m_rect.width = 5;
                    m_rect.height = 20;
                    m_sprite.setSize({10, 50});
                }
                else if(axis == Axis::BOTH) {
                    m_rect.width = 20;
                    m_rect.height = 20;
                }
            }
            Manipulator(const Manipulator& other) = delete;
            Manipulator& operator=(const Manipulator& other) = delete;
            Manipulator(Manipulator&& other) = delete;
            Manipulator& operator=(Manipulator&& other) = delete;
            ~Manipulator() override = default;

            robot2D::vec2f getSize() const;

            bool isPressed(robot2D::vec2f point);
            bool contains(robot2D::vec2f point) const;
            void setSize(robot2D::vec2f size);
            void setScaleHandle(bool scale) {
                m_scaleHandle = scale;
            }

            bool active() const;
            void reset() {
                m_active = false;
            }

            void setPosition(const robot2D::vec2f& position);
            void setColor(robot2D::Color color);

            void moveX(float offset);
            void moveY(float offset);

            void draw(robot2D::RenderTarget& target,
                [[maybe_unused]] robot2D::RenderStates states) const override;
            robot2D::Sprite m_sprite;
        private:
            robot2D::FloatRect m_rect;
            robot2D::Color m_color;
            robot2D::vec2f m_offset;
            bool m_active{false};
            bool m_scaleHandle{false};
            Axis m_axis;
        };

    public:
        Guizmo2D();
        Guizmo2D(const Guizmo2D& other) = delete;
        Guizmo2D& operator=(const Guizmo2D& other) = delete;
        Guizmo2D(Guizmo2D&& other) = delete;
        Guizmo2D& operator=(Guizmo2D&& other) = delete;
        ~Guizmo2D() override = default;

        void handleEvents(const robot2D::Event& event);
        void update();
        void updateHover(robot2D::vec2f mousePosition, bool overViewport);

        void setManipulated(TransformComponent* transformable);
        void setManipulated(std::vector<TransformComponent*>& transformables);
        // Move selected roots once; the full selection still determines gizmo placement.
        void setMovementRoots(const std::vector<TransformComponent*>& transforms);
        void cancelDrag();
        void setModificationCallback(std::function<void()> callback) {
            m_onModified = std::move(callback);
        }

        void setOperationType(Operation type);
        Operation getOperationType() const {
            return m_operation;
        }
        void setCamera(IEditorCamera::Ptr camera);
        void setIsShow(bool flag);
        void draw(robot2D::RenderTarget& target,
            robot2D::RenderStates states) const override;

        bool hasManipulated() const {
            return !m_manipulateds.empty();
        }
        bool isActive() const {
            return m_leftMousePressed && !m_manipulateds.empty() && m_isCollide;
        }
    private:
        void processMouseMoved(const robot2D::Event::MouseMoved& event);
        void processMousePressed(const robot2D::Event::MousePressed& event);
        void processMouseRelease(const robot2D::Event::MouseReleased& event);

        void moveManipulators(const robot2D::vec2f& diff);
        void updateHandleSizes();
        void updateColors();
        bool containsRotation(robot2D::vec2f point) const;
    private:
        bool m_isShown{false};
        bool m_isCollide{false};
        bool m_leftMousePressed{false};
        enum class Handle { None, X, Y, XY, Rotation };
        Handle m_hovered {Handle::None};
        robot2D::vec2f m_unitsPerPixel {1.f, 1.f};

        std::vector<TransformComponent*> m_manipulateds;
        std::vector<TransformComponent*> m_movementRoots;
        std::function<void()> m_onModified;

        EventBinder m_eventBinder;
        Operation m_operation{Operation::Move};
        IEditorCamera::Ptr m_camera{nullptr};

        Manipulator m_xAxisManipulator;
        Manipulator m_yAxisManipulator;
        Manipulator m_XYAxisManipulator;
        robot2D::Sprite m_rotateSprite;

        robot2D::vec2f m_manipulatorLastPos;

    };
}
