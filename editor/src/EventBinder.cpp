#include <editor/EventBinder.hpp>

namespace editor {
    EventBinder::EventBinder(const robot2D::Event& event)
    : m_event{event} {}

    bool EventBinder::Dispatch(const robot2D::Event::EventType& eventType,
                                       std::function<void(const robot2D::Event&)>&& function) {
        if(m_event.type == eventType)
            function(m_event);
        return true;
    }

    bool EventBinder::Dispatch(const robot2D::Event::EventType& eventType,
                                       const std::function<void(const robot2D::Event&)>& function) {
        if(m_event.type == eventType)
            function(m_event);
        return true;
    }
}