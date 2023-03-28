#pragma once

#include <robot2D/Core/Event.hpp>
#include <robot2D/Graphics/Drawable.hpp>
#include <robot2D/Graphics/Transformable.hpp>
#include <robot2D/Graphics/Color.hpp>
#include <robot2D/Graphics/Rect.hpp>
#include <robot2D/Graphics/RenderTarget.hpp>

#include "EventBinder.hpp"
#include "EditorCamera.hpp"

namespace editor {


    class Guizmo2D: public robot2D::Drawable {
    public:
        enum class Operation {
            Move, Scale, Rotate
        };
    private:
        class Manipulator: public robot2D::Drawable {
        public:
            enum class Axis {
                X, Y
            };

            Manipulator(Axis axis, robot2D::Color color): m_color{color}, m_axis{axis} {
                if(axis == Axis::X) {
                    m_rect.height = 15;
                    m_rect.width = 50;
                }
                else if(axis == Axis::Y) {
                    m_rect.height = 50;
                    m_rect.width = 15;
                }
            }
            ~Manipulator() override = default;

            bool isPressed(robot2D::vec2f point) {
                m_active = m_rect.contains(point);
                if(m_active)
                    m_offset = point - m_rect.topPoint();
                return m_active;
            }

            bool active() const { return m_active; }

            void setPosition(const robot2D::vec2f& position) {
                m_rect.lx = position.x;
                m_rect.ly = position.y;
                if(m_axis == Axis::Y)
                    m_rect.ly += m_rect.width;
            }

            void moveX(float offset) {

            }

            void moveY(float offset) {

            }

            void draw(robot2D::RenderTarget& target,
                      [[maybe_unused]] robot2D::RenderStates states) const override {

                robot2D::Transform transform;
                transform.translate(m_rect.topPoint());
                transform.scale({m_rect.width, m_rect.height});
                states.transform *= transform;
                states.color = m_color;
                target.draw(states);
            }
        private:
            robot2D::FloatRect m_rect;
            robot2D::Color m_color;
            robot2D::vec2f m_offset;
            bool m_active{false};
            Axis m_axis;
        };
    public:
        Guizmo2D();
        ~Guizmo2D() override = default;

        void handleEvents(const robot2D::Event& event);
        void update();


        void setManipulated(robot2D::Transformable* transformable);
        void setOperationType(Operation type) { m_operation = type; }
        void setCamera(IEditorCamera::Ptr camera) { m_camera = camera; }
        void setIsShow(bool flag);
        void draw(robot2D::RenderTarget& target,
                    robot2D::RenderStates states) const override;
    private:
        void processMouseMoved(robot2D::Event event);
        void processMousePressed(robot2D::Event event);
        void processMouseRelease(robot2D::Event event);
    private:
        bool m_isShown{false};
        bool m_leftMousePressed{false};
        robot2D::Transformable* m_manipulated{nullptr};
        EventBinder m_eventBinder;
        Operation m_operation{Operation::Move};
        IEditorCamera::Ptr m_camera{nullptr};

        Manipulator m_xAxisManipulator;
        Manipulator m_yAxisManipulator;
    };
}