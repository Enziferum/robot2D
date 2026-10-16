#pragma once
#include <algorithm>
#include <limits>
#include <deque>
#include <vector>
#include "InputRouter.hpp"

namespace editor {
    struct RecordedEvent {
        robot2D::Event event;
        RouteDecision decision;
    };

    /// Diagnostic ring; never used as the runtime delivery queue. Main thread only.
    class EventHistory {
    public:
        explicit EventHistory(std::size_t capacity = 2048, std::size_t payloadBudget = 2* 1024* 1024) :
            m_capacity(capacity), m_budget(std::min(payloadBudget, std::numeric_limits<std::size_t>::max() / 2)) {
        }
        void record(const robot2D::Event& event, RouteDecision decision) {
            if(!m_recording || m_capacity == 0) return;
            const auto bytes = payloadSize(event);
            if(bytes > m_budget) {
                ++m_oversized; return;
            }
            // Copy before evicting, so an allocation failure preserves old records.
            m_events.push_back({event, decision});
            m_bytes += bytes;
            while(m_events.size() > m_capacity || m_bytes > m_budget) {
                m_bytes -= payloadSize(m_events.front().event);
                m_events.pop_front();
                ++m_overwritten;
            }
        }
        std::vector<RecordedEvent> snapshot() const {
            return {m_events.begin(), m_events.end()};
        }
        const std::deque<RecordedEvent>& events() const noexcept {
            return m_events;
        }
        void setRecording(bool enabled) noexcept {
            m_recording = enabled;
        }
        bool recording() const noexcept {
            return m_recording;
        }
        std::uint64_t overwrittenCount() const noexcept {
            return m_overwritten;
        }
        std::uint64_t oversizedCount() const noexcept {
            return m_oversized;
        }
        void clear() noexcept {
            m_events.clear(); m_overwritten = 0; m_oversized = 0; m_bytes = 0;
        }
    private:
        std::size_t payloadSize(const robot2D::Event& event) const noexcept {
            std::size_t bytes = sizeof(robot2D::Event);
            if(bytes > m_budget) return m_budget + 1;
            if(const auto* drop = event.getIf<robot2D::Event::FilesDropped>()) {
                for(const auto& path : drop -> paths) {
                    if(sizeof(std::string) > m_budget - bytes) return m_budget + 1;
                    bytes += sizeof(std::string);
                    if(path.size() > m_budget - bytes) return m_budget + 1;
                    bytes += path.size();
                }
            }
            return bytes;
        }
        std::size_t m_capacity;
        std::size_t m_budget;
        std::size_t m_bytes{0};
        std::uint64_t m_oversized{0};
        std::deque<RecordedEvent> m_events;
        std::uint64_t m_overwritten{0};
        bool m_recording{true};
    };
}
