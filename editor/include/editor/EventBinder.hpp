#pragma once


#include <functional>
#include <robot2D/Core/Event.hpp>

namespace editor {
    class EventBinder {
    public:
        EventBinder(const robot2D::Event& event);
        ~EventBinder() = default;

        bool Dispatch(const robot2D::Event::EventType& eventType,
                      std::function<void(const robot2D::Event& event)>&& function);
        bool Dispatch(const robot2D::Event::EventType& eventType,
                      const std::function<void(const robot2D::Event& event)>& function);
    private:
        const robot2D::Event& m_event;
    };
}