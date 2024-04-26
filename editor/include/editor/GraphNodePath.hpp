#pragma once
#include <deque>

namespace editor {
    using GraphNodeIndex = int;

    class GraphNodePath {
    public:
        GraphNodePath() = default;
        GraphNodePath(const GraphNodePath& other) = default;
        GraphNodePath& operator=(const GraphNodePath& other) = default;
        GraphNodePath(GraphNodePath&& other) = default;
        GraphNodePath& operator=(GraphNodePath&& other) = default;
        ~GraphNodePath() = default;


        void append(GraphNodeIndex graphNodeIndex) {
            m_indices.push_front(graphNodeIndex);
        }

        void update() {}
        bool isSameLevel(const GraphNodePath& other) const { return m_indices.size() == other.m_indices.size(); }
    private:
        std::deque<GraphNodeIndex> m_indices;
    };
} // namespace editor