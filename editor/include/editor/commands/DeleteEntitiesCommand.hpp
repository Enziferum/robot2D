#pragma once
#include <vector>
#include <editor/UIInteractor.hpp>
#include <editor/DeletedEntitesRestoreInformation.hpp>

#include "ICommand.hpp"

namespace editor {

    class DeleteEntitiesCommand: public ICommand {
    public:
        DeleteEntitiesCommand(UIInteractor::Ptr interactor,
                              DeletedEntitiesRestoreInformation deletedEntitiesRestoreInformation,
                              DeletedEntitiesRestoreUIInformation deletedEntitiesRestoreUIInformation);
        ~DeleteEntitiesCommand() = default;

        DECLARE_CLASS_ID(DeleteEntitiesCommand)

        void undo() override;
    private:
        UIInteractor::Ptr m_interactor;
        DeletedEntitiesRestoreInformation m_deletedEntitiesRestoreInformation;
        DeletedEntitiesRestoreUIInformation m_deletedEntitiesRestoreUIInformation;
    };

} // namespace editor