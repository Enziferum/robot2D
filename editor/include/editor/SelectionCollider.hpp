#pragma once
#include "Quad.hpp"

namespace editor {

    class SelectionCollider: public robot2D::Drawable {
    public:
        SelectionCollider() = default;
        ~SelectionCollider() override = default;

        void setPosition(const robot2D::vec2f& position) {
            m_aabb.lx = position.x;
            m_aabb.ly = position.y;
        }

        robot2D::vec2f getPosition() const {
            return { m_aabb.lx, m_aabb.ly };
        }

        void reset() {
            m_aabb.width = 0.F;
            m_aabb.height = 0.F;
        }

        void setSize(const robot2D::vec2f& size) {
            m_aabb.width = size.x;
            m_aabb.height = size.y;
        }
        void setIsShown(bool flag) { m_show = flag;}

        const robot2D::FloatRect& getRect() const { return m_aabb; }

        bool isShown() const { return m_show; }

        void draw(robot2D::RenderTarget& target,
                  robot2D::RenderStates states) const override {
            std::array<robot2D::Transform, 4> quads;
            quads[0].translate(m_aabb.lx, m_aabb.ly);
            quads[0].scale(m_aabb.width, borderSize);

            quads[1].translate(m_aabb.lx, m_aabb.ly);
            quads[1].scale(borderSize, m_aabb.height);

            quads[2].translate(m_aabb.lx, m_aabb.ly + m_aabb.height);
            quads[2].scale(m_aabb.width,borderSize);

            quads[3].translate(m_aabb.lx + m_aabb.width, m_aabb.ly);
            quads[3].scale(borderSize, m_aabb.height);

            for(const auto& transform: quads) {
                states.transform = transform;
                states.color = borderColor;
                target.draw(states);
            }
        }

    private:
        robot2D::FloatRect m_aabb;
        mutable std::array<Quad, 4> walls;
        bool m_show = false;
        float borderSize = 1.F;
        robot2D::Color borderColor{robot2D::Color::Green};
    };

} // namespace editor