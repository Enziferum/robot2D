/*********************************************************************
(c) Alex Raag 2024
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

#include "EventBinder.hpp"
#include "Quad.hpp"

namespace editor {
    class ObjectManipulator: public robot2D::Drawable {
    public:
        ObjectManipulator();
        ~ObjectManipulator() override = default;

        void handleEvents(const robot2D::Event& event);
        void update(float ts);

        void setPosition(const robot2D::vec2f& position);

        void setIsShown(bool flag);
        bool isShown() const;

        void draw(robot2D::RenderTarget& target,
                  robot2D::RenderStates renderStates) const override;

    private:
        EventBinder m_eventBinder;
        mutable std::array<Quad, 4> moveQ;
        mutable std::array<Quad, 4> moveWalls;

        bool m_shown{false};
    };
}