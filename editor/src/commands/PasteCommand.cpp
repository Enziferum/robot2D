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
#include <editor/Components.hpp>
#include <editor/Messages.hpp>
#include <editor/commands/PasteCommand.hpp>

namespace editor {

    PasteCommand::PasteCommand(robot2D::MessageBus& messageBus,
                               std::vector<robot2D::ecs::Entity> entities,
                               UIInteractor::Ptr interactor):
                               m_messageBus{messageBus},
                               m_entities(entities),
                               m_interactor{interactor}
                               {

    }

    void PasteCommand::undo() {
        for(auto ent: m_entities) {
            if(!ent && ent.destroyed())
                continue;
            if(m_interactor)
                m_interactor -> removeEntity(ent);
            auto* msg = m_messageBus.postMessage<EntityRemovement>(MessageID::EntityRemove);
            msg -> entityID = ent.getComponent<IDComponent>().ID;
        }
    }
} // namespace editor