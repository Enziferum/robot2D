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
#include <stdexcept>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <robot2D/Core/Event.hpp>

namespace editor {
    enum class EventResult { Ignored, Handled };

    /// One handler per concrete payload type. Bind/unbind outside dispatch only.
    class EventBinder {
    public:
        EventBinder() = default;
        EventBinder(const EventBinder&) = delete;
        EventBinder& operator=(const EventBinder&) = delete;
        EventBinder(EventBinder&&) = delete;
        EventBinder& operator=(EventBinder&&) = delete;
        ~EventBinder() = default;

        template<typename Payload, typename Callback>
        void bind(Callback&& callback) {
            static_assert(isPayload<Payload>(std::make_index_sequence<std::variant_size_v<robot2D::Event::Data> >{}),
                "Bind requires a concrete Event payload alternative");
            requireIdle();
            using Function = std::decay_t<Callback>;
            m_handlers[std::type_index(typeid(Payload))] =
                [function = Function(std::forward<Callback>(callback))](const robot2D::Event& event,
                const robot2D::EventContext& context) mutable {
                const auto& value = event.get<Payload>();
                if constexpr(std::is_invocable_v<Function&, const Payload&, const robot2D::EventContext&>) {
                    if constexpr(std::is_void_v<std::invoke_result_t<Function&, const Payload&,
                        const robot2D::EventContext&> >) {
                        function(value, context);
                        return EventResult::Handled;
                    }
                    else return function(value, context);
                }
                else {
                    static_assert(std::is_invocable_v<Function&, const Payload&>,
                        "Handler requires a const payload reference");
                    if constexpr(std::is_void_v<std::invoke_result_t<Function&, const Payload&> >) {
                        function(value);
                        return EventResult::Handled;
                    }
                    else return function(value);
                }
            };
        }
        template<typename Payload> void unbind() {
            requireIdle(); m_handlers.erase(std::type_index(typeid(Payload)));
        }

        EventResult dispatch(const robot2D::Event& event, const robot2D::EventContext& context) const {
            struct DispatchScope {
                unsigned& depth;
                explicit DispatchScope(unsigned& value) : depth(value) {
                    ++depth;
                }
                ~DispatchScope() {
                    --depth;
                }
            } scope{m_dispatchDepth};
            return event.visit([&](const auto& payload) {
                const auto found = m_handlers.find(std::type_index(typeid(payload)));
                return found == m_handlers.end() ? EventResult::Ignored : found -> second(event, context);
            });
        }
        EventResult dispatch(const robot2D::Event& event) const {
            return dispatch(event, event.context());
        }
    private:
        template<typename Payload, std::size_t... Index>
        static constexpr bool isPayload(std::index_sequence<Index...>) noexcept {
            return (std::is_same_v<Payload, std::variant_alternative_t<Index, robot2D::Event::Data> > || ...);
        }
        void requireIdle() const {
            if(m_dispatchDepth != 0) throw std::logic_error("Cannot modify EventBinder during dispatch");
        }
        mutable unsigned m_dispatchDepth{0};
        using Handler = std::function<EventResult (const robot2D::Event&, const robot2D::EventContext&)>;
        std::unordered_map<std::type_index, Handler> m_handlers;
    };
}
