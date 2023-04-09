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

#include <stack>
#include <queue>
#include <cassert>

#include <robot2D/Util/Logger.hpp>
#include "commands/ICommand.hpp"

namespace editor {
    class CommandStack {
    public:
        CommandStack() = default;
        ~CommandStack() = default;

        template<typename T, typename ...Args>
        [[maybe_unused]]
        T* addCommand(Args&&... args);

        [[maybe_unused]]
        void addCommand(ICommand::Ptr&& ptr);

        void undo();
        void redo();

        std::size_t undoSize() const;
        std::size_t redoSize() const;

        void clear();

        [[nodiscard]]
        bool empty() const noexcept;
    private:
        std::stack<ICommand::Ptr> m_stack {};
        std::queue<ICommand::Ptr> m_redoQueue {};
    };

    template<typename T, typename ...Args>
    T* CommandStack::addCommand(Args&&...args) {
        static_assert(std::is_base_of_v<ICommand, T>);
        auto ptr = std::make_shared<T>(std::forward<Args>(args)...);
        assert(ptr != nullptr && "Command of Type T is nullptr");
        m_stack.push(ptr);

        return static_cast<T*>(ptr.get());
    }


} // namespace viewer