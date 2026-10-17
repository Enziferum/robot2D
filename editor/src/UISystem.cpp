/*********************************************************************
   (c) Alex Raag 2026
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
#include <robot2D/Core/Mouse.hpp>
#include <editor/Components.hpp>
#include <editor/components/UIHitBox.hpp>

namespace editor {

    UISystem::UISystem(robot2D::MessageBus& bus) :
        robot2D::ecs::System{bus, typeid(UISystem)} {
        addRequirement<TransformComponent>();
        addRequirement<UIHitbox>();
    }

    void UISystem::handleEvents(const robot2D::Event& event) {
        using Event = robot2D::Event;
        if(event.getIf<Event::LostFocus>()) {
            m_mouseDownEvents.clear();
            m_mouseUpEvents.clear();
            return;
        }
        if(!m_editorCamera) return;
        if(const auto* button = event.getIf<Event::MouseButtonEvent>()) {
            const auto position = m_editorCamera -> convertPixelToCoords({button -> x, button -> y}).as<int>();
            m_eventPosition = position;
            Flags flag;
            if(button -> btn == mouse2int(robot2D::Mouse::MouseLeft)) flag = Flags::LeftMouse;
            else if(button -> btn == mouse2int(robot2D::Mouse::MouseRight)) flag = Flags::RightMouse;
            else if(button -> btn == mouse2int(robot2D::Mouse::MouseMiddle)) flag = Flags::MiddleMouse;
            else return;
            if(event.getIf<Event::MousePressed>()) m_mouseDownEvents.push_back({flag, position});
            else m_mouseUpEvents.push_back(flag);
        }
        else if(const auto* move = event.getIf<Event::MouseMoved>())
            m_eventPosition = m_editorCamera -> convertPixelToCoords({move -> x, move -> y}).as<int>();
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

            if(contains) {
                if(hitbox.isHovered) {
                    if(hitbox.onHoverOnce && !hitbox.wasHovered) {
                        auto index = hitbox.callbackIDs[UIHitbox::CallbackID::MouseMoved];
                        if(index >= 0)
                            m_mouseHoverCallbacks[index](ent);
                    }
                    hitbox.wasHovered = true;
                }

            }
            if(!hitbox.isHovered) {
                auto index = hitbox.callbackIDs[UIHitbox::CallbackID::MouseMoved];
                if(index >= 0)
                    m_mouseUnHoverCallbacks[index](ent);

                hitbox.wasHovered = false;
            }

        }

        // Deliver presses chronologically across hitboxes, not grouped by entity.
        for(const auto& downEvent : m_mouseDownEvents) {
            for(auto& ent : m_entities) {
                auto& hitbox = ent.getComponent<UIHitbox>();
                if(!hitbox.m_area.contains({static_cast<float>(downEvent.position.x),
                                            static_cast<float>(downEvent.position.y)}))
                    continue;
                const auto index = hitbox.callbackIDs[UIHitbox::CallbackID::MouseDown];
                if(index >= 0 && static_cast<std::size_t>(index) < m_mousePressedCallbacks.size() &&
                    m_mousePressedCallbacks[index])
                    m_mousePressedCallbacks[index](ent, downEvent.button);
            }
        }

        m_mouseDownEvents.clear();
        m_mouseUpEvents.clear();
    }

    robot2D::ecs::System::Ptr UISystem::cloneSelf(robot2D::ecs::Scene* scene,
        const std::vector<robot2D::ecs::Entity>& newEntities) {
        auto clone = std::make_shared<UISystem>(m_messageBus);
        if(!cloneBase(clone, scene, newEntities)) return nullptr;
        clone -> m_editorCamera = m_editorCamera;
        clone -> m_mousePressedCallbacks = m_mousePressedCallbacks;
        clone -> m_mouseHoverCallbacks = m_mouseHoverCallbacks;
        clone -> m_mouseUnHoverCallbacks = m_mouseUnHoverCallbacks;
        return clone;
    }

}