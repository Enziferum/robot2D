#pragma once

#include "Event.hpp"
#include <deque>

namespace robot2D {
    /// Main-thread FIFO. Reception timestamps share this queue's monotonic epoch.
    class EventQueue {
    public:
        template<typename Payload>
        void push(Payload payload) {
            const auto time = std::chrono::duration_cast<EventTime>(Clock::now() - m_epoch);
            m_events.emplace_back(std::move(payload), EventContext{m_next, 0, time});
            ++m_next;
        }
        bool poll(Event& event) {
            if(m_events.empty())
                return false;
            event = std::move(m_events.front());
            m_events.pop_front();
            return true;
        }
        std::vector<Event> take() {
            std::vector<Event> events;
            events.reserve(m_events.size());
            while(!m_events.empty()) {
                events.push_back(std::move(m_events.front()));
                m_events.pop_front();
            }
            return events;
        }
    private:
        using Clock = std::chrono::steady_clock;
        Clock::time_point m_epoch{Clock::now()};
        std::uint64_t m_next{1};
        std::deque<Event> m_events;
    };
}
