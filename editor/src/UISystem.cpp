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

#include <robot2D/Ecs/EntityManager.hpp>

#include <editor/UISystem.hpp>
#include <editor/Components.hpp>
#include <editor/components/UIHitBox.hpp>

namespace editor {

    UISystem::UISystem(robot2D::MessageBus& bus):
            robot2D::ecs::System{bus, typeid(UISystem)} {
        addRequirement<TransformComponent>();
        addRequirement<UIHitbox>();
    }

    void UISystem::handleEvents(const robot2D::Event& event) {

        if(event.type == robot2D::Event::MousePressed) {

            if(event.mouse.btn == mouse2int(robot2D::Mouse::MouseLeft))
                m_mouseDownEvents.emplace_back(Flags::LeftMouse);
            if(event.mouse.btn == mouse2int(robot2D::Mouse::MouseRight))
                m_mouseDownEvents.emplace_back(Flags::RightMouse);
            if(event.mouse.btn == mouse2int(robot2D::Mouse::MouseMiddle))
                m_mouseDownEvents.emplace_back(Flags::MiddleMouse);

            m_eventPosition = m_editorCamera -> convertPixelToCoords( { 
                static_cast<float>(event.mouse.x), 
                static_cast<float>(event.mouse.y)}
            ).as<int>();
        }

        if(event.type == robot2D::Event::MouseReleased) {
            if(event.mouse.btn == mouse2int(robot2D::Mouse::MouseLeft))
                m_mouseUpEvents.emplace_back(Flags::LeftMouse);
            if(event.mouse.btn == mouse2int(robot2D::Mouse::MouseRight))
                m_mouseUpEvents.emplace_back(Flags::RightMouse);
            if(event.mouse.btn == mouse2int(robot2D::Mouse::MouseMiddle))
                m_mouseUpEvents.emplace_back(Flags::MiddleMouse);

            m_eventPosition = m_editorCamera -> convertPixelToCoords({
                static_cast<float>(event.mouse.x), static_cast<float>(event.mouse.y)
            }).as<int>();
        }

        if(event.type == robot2D::Event::MouseMoved) {
            m_eventPosition = m_editorCamera -> convertPixelToCoords({
                static_cast<float>(event.mouse.x), static_cast<float>(event.mouse.y)
            }).as<int>();
        }

        if(event.type == robot2D::Event::MouseWheel) {
            // no-op //
        }

        if(event.type == robot2D::Event::KeyPressed) {
            // no-op //
        }
    }

    std::uint32_t UISystem::addMousePressedCallback(MousePressedCallback&& callback) {
        m_mousePressedCallbacks.emplace_back(callback);
        return static_cast<std::uint32_t>(m_mousePressedCallbacks.size() - 1);
    }

    std::uint32_t UISystem::addMouseHoverCallback(MouseHoverCallback&& callback) {
        m_mouseHoverCallbacks.emplace_back(callback);
        return static_cast<std::uint32_t>(m_mousePressedCallbacks.size() - 1);
    }

    std::uint32_t UISystem::addMouseUnHoverCallback(MouseHoverCallback&& callback) {
        m_mouseUnHoverCallbacks.emplace_back(callback);
        return static_cast<std::uint32_t>(m_mousePressedCallbacks.size() - 1);
    }


    void UISystem::update(float dt) {
        for(auto& ent: m_entities) {
            bool contains = false;
            auto& hitbox = ent.getComponent<UIHitbox>();


            if((contains = hitbox.m_area.contains( {
                static_cast<float>(m_eventPosition.x), static_cast<float>(m_eventPosition.y)
            } ))) {
                hitbox.isHovered = true;
            }
            else
                hitbox.isHovered = false;

            if (contains) {
                if(hitbox.isHovered) {
                    if(hitbox.onHoverOnce && !hitbox.wasHovered) {
                        auto index = hitbox.callbackIDs[UIHitbox::CallbackID::MouseMoved];
                        if (index >= 0)
                            m_mouseHoverCallbacks[index](ent);
                    }
                    hitbox.wasHovered = true;
                }

                for(auto downEvent: m_mouseDownEvents) {
                    auto index = hitbox.callbackIDs[UIHitbox::CallbackID::MouseDown];
                    if(m_mousePressedCallbacks[index])
                        m_mousePressedCallbacks[index](ent, downEvent);
                }
            }

            if(!hitbox.isHovered) {
                auto index = hitbox.callbackIDs[UIHitbox::CallbackID::MouseMoved];
                if(index >= 0)
                    m_mouseUnHoverCallbacks[index](ent);

                hitbox.wasHovered = false;
            }

        }

        m_mouseDownEvents.clear();
        m_mouseUpEvents.clear();
    }

}