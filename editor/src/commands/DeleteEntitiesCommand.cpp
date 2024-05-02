/*********************************************************************
(c) Alex Raag 2024
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

    const class_id& DeleteEntitiesCommand::id() noexcept {
        static const class_id id{"DeleteEntitiesCommand"};
        return id;
    }

    void DeleteEntitiesCommand::undo() {
        RB_EDITOR_INFO("DeleteEntitiesCommand: undo command");

        if(!m_interactor)
            return;

        m_interactor -> restoreDeletedEntities(m_deletedEntitiesRestoreInformation, m_deletedEntitiesRestoreUIInformation);
    }
}