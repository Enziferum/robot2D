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

#include <functional>
#include <unordered_map>

#include <robot2D/Core/Keyboard.hpp>

namespace editor {

    class ShortCut {
    public:
        ShortCut() = default;
        ShortCut(const std::initializer_list<robot2D::Key>& keys) {
            for(auto& key: keys)
                m_keys[key] = false;
        }

        void setCallback(std::function<void()>&& callback) {
            m_callback = std::move(callback);
        }
        void updateKeys(const std::initializer_list<robot2D::Key>& keys) {
            m_keys.clear();
            for(auto& key: keys)
                m_keys[key] = false;
        }

        void handleEvents(const robot2D::Event& event) {
            switch(event.type) {
                default:
                    break;
                case robot2D::Event::KeyPressed:
                case robot2D::Event::KeyReleased:
                    if(m_keys.find(event.key.code) != m_keys.end())
                        m_keys[event.key.code] = (event.type == robot2D::Event::KeyPressed);
                    if(event.type == robot2D::Event::KeyReleased)
                        m_wasExec = false;
                    break;
            }
        }

        bool isDown() const {
            for(const auto& [key, value]: m_keys)
                if(!value)
                    return false;
            return true;
        }

        bool isRuntimeDown() const {
            for(const auto& [key, value]: m_keys)
                if(!robot2D::Window::isKeyboardPressed(key))
                    return false;
            return true;
        }

        void operator()() const {
            if(m_callback && !m_wasExec) {
                m_callback();
                m_wasExec = true;
            }
        }
    private:
        std::unordered_map<robot2D::Key, bool> m_keys{};
        std::function<void()> m_callback{nullptr};
        mutable bool m_wasExec{false};
    };


    template<typename ID>
    class ShortCutManager {
    public:
        ShortCutManager() = default;
        ~ShortCutManager() = default;

        void handleEvents(const robot2D::Event& event) {
            for(auto& [key, shortcut]: m_shortCuts)
                shortcut.handleEvents(event);
        }

        void update() {
            for(const auto& [id, shortCut]: m_shortCuts) {
                if(shortCut.isDown())
                    shortCut();
            }
        }

        void runTimeUpdate() {
            for(const auto& [id, shortCut]: m_shortCuts) {
                if(shortCut.isRuntimeDown())
                    shortCut();
            }
        }

        bool has(const ID& id) const {
            return m_shortCuts.find(id) != m_shortCuts.end();
        }

        void bind(const std::pair<ID, ShortCut>& shortCut) {
            if(has(shortCut.first))
                return;
            m_shortCuts.insert(shortCut);
        }

        void remove(const ID& id) {
            m_shortCuts.erase(id);
        }
    private:
        std::unordered_map<ID, ShortCut> m_shortCuts;
    };
}