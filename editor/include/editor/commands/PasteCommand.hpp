#pragma once

#include "ICommand.hpp"

namespace editor {
    class PasteCommand final: public ICommand {
    private:
        PasteCommand();
        ~PasteCommand() override = default;


        void undo() override;
    };
} // namespace editor