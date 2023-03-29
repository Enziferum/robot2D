#include <editor/CommandStack.hpp>

namespace editor {

    void CommandStack::undo() {
        if(m_stack.empty())
            return;

        auto ptr = m_stack.top();
        m_stack.pop();

        ptr -> undo();
        m_redoQueue.push(ptr);
    }

    void CommandStack::redo() {
        if(m_redoQueue.empty())
            return;

        auto ptr = m_redoQueue.front();
        m_redoQueue.pop();

        ptr -> redo();
    }

    bool CommandStack::empty() const noexcept {
        return m_stack.empty() && m_redoQueue.empty();
    }

    void CommandStack::addCommand(ICommand::Ptr&& ptr) {
        assert(ptr != nullptr && "Don't add nullptr command");
        m_stack.push(std::move(ptr));
    }

    std::size_t CommandStack::undoSize() const {
        return m_stack.size();
    }

    std::size_t CommandStack::redoSize() const {
        return m_redoQueue.size();
    }

    void CommandStack::clear() {
        while(!m_stack.empty())
            m_stack.pop();
        while(!m_redoQueue.empty())
            m_redoQueue.pop();
    }

} // namespace viewer