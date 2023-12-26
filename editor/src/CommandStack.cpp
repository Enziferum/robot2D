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

#include <editor/CommandStack.hpp>

namespace editor {

    void CommandStack::undo() {
        if(m_stack.empty())
            return;

        auto ptr = m_stack.top();
        m_stack.pop();
        m_commandIDs.pop_back();

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

        m_commandIDs.clear();
    }

} // namespace viewer