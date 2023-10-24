#include <robot2D/Util/Logger.hpp>
#include <editor/commands/DeleteEntitiesCommand.hpp>

namespace editor {

    DeleteEntitiesCommand::DeleteEntitiesCommand(UIInteractor::Ptr interactor,
                                                 DeletedEntitiesRestoreInformation deletedEntitiesRestoreInformation,
                                                 DeletedEntitiesRestoreUIInformation deletedEntitiesRestoreUIInformation):
                                                 m_interactor{interactor},
                                                 m_deletedEntitiesRestoreInformation(deletedEntitiesRestoreInformation),
                                                 m_deletedEntitiesRestoreUIInformation(deletedEntitiesRestoreUIInformation)
                                                 {}

    void DeleteEntitiesCommand::undo() {
        RB_EDITOR_INFO("DeleteEntitiesCommand: undo command");

        if(!m_interactor)
            return;

        m_interactor -> restoreDeletedEntities(m_deletedEntitiesRestoreInformation, m_deletedEntitiesRestoreUIInformation);
    }
}