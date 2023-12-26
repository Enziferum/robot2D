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

#include <memory>
#include <unordered_map>
#include <utility>

#include <robot2D/Core/Event.hpp>


namespace robot2D {
    template<typename T>
    const T& eventAs(const robot2D::Event& event);
}

namespace editor {

    class EventBinder {
        struct IFunc {
            using Ptr = std::shared_ptr<IFunc>;
            virtual ~IFunc() = 0;

            virtual bool execute(const robot2D::Event& event) = 0;
        };

        template<typename F,
                typename = std::enable_if_t<std::is_invocable_v<F, const robot2D::Event&>>
                >
        struct Func: IFunc {
            explicit Func(F&& func): m_func{func} {}
            ~Func() override = default;

            bool execute(const robot2D::Event& event) override {
                m_func(event);
                return true;
            }
        private:
            F m_func;
        };
    public:
        EventBinder() = default;
        EventBinder(const EventBinder& other) = delete;
        EventBinder& operator=(const EventBinder& other) = delete;
        EventBinder(EventBinder&& other) = delete;
        EventBinder& operator=(EventBinder&& other) = delete;
        ~EventBinder() = default;


        template<typename F,
                typename = std::enable_if_t<std::is_invocable_v<F, const robot2D::Event&>>>
        void bindEvent(robot2D::Event::EventType eventType, F&& func) {
            auto funcPtr = std::make_shared<Func<F>>(std::forward<F>(func));
            if(!funcPtr)
                return;
            m_events[eventType] = funcPtr;
        }

        void unBind(robot2D::Event::EventType eventType) {
            const auto& found = m_events.find(eventType);
            if(found == m_events.end())
                return;
            m_events.erase(found);
        }

        // TODO(a.raag): try to execute not whole event
        void handleEvents(const robot2D::Event& event) {
            if(m_events.find(event.type) != m_events.end())
                m_events[event.type] -> execute(event);
        }
    private:
        std::unordered_map<robot2D::Event::EventType, IFunc::Ptr> m_events{};
    };

} // namespace editor