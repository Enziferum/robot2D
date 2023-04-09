#pragma once

#include "ICommand.hpp"

namespace editor {

    class DeleteEntitiesCommand: public ICommand {
    public:
        DeleteEntitiesCommand();
        ~DeleteEntitiesCommand() = default;

        void undo() override;
    private:
    };

} // namespace editor