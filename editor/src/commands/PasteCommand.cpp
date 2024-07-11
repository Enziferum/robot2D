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
#include <editor/Components.hpp>
#include <editor/Messages.hpp>
#include <editor/commands/PasteCommand.hpp>

namespace editor {

    const class_id& PasteCommand::id() noexcept {
        static const class_id id{"PasteCommand"};
        return id;
    }


    PasteCommand::PasteCommand(robot2D::MessageBus& messageBus,
                               std::vector<SceneEntity>&& entities,
                               EditorInteractor::Ptr interactor):
                               m_messageBus{messageBus},
                               m_entities(entities),
                               m_interactor{interactor}
    {

    }



    void PasteCommand::undo() {
        if(m_entities.empty())
            return;
        constexpr bool clearContainer = true;
        m_interactor -> removeEntities(m_entities, clearContainer);
    }
} // namespace editor