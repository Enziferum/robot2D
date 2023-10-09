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

#include <cstdint>
#include <memory>

namespace editor {
    using CommandID = std::uint32_t;

    class class_id {
    public:
        class_id(const char* name);

        int getIndex() const { return m_index; }
    private:
        const char* const m_name;
        int m_index;
    };

#define DECLARE_CLASS_ID(name) static class_id id()  { \
    static class_id id(#name);                                \
    return id;                                               \
}


    class ICommand {
    public:
        using Ptr = std::shared_ptr<ICommand>;
    public:
        virtual ~ICommand() = 0;

        [[nodiscard]]
        const CommandID& getCommandID() const { return m_commandID; }
        void setCommandID(CommandID commandId) { m_commandID = commandId;}

        virtual void undo() = 0;
        virtual void redo();
    protected:
        CommandID m_commandID{0};
    };

} // namespace viewer