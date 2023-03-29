#pragma once

#include "ICommand.hpp"

namespace editor {
    class CopyCommand final: public ICommand {
    private:
        CopyCommand();
        ~CopyCommand() override = default;

        void undo() override;
    };
} // namespace editor