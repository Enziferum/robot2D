#pragma once

#include <cstdint>
#include <memory>

namespace editor {
    using CommandID = std::uint32_t;

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