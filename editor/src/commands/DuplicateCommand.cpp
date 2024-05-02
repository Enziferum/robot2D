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

#include <editor/commands/DuplicateCommand.hpp>
#include <editor/Messages.hpp>

namespace editor {
    DuplicateCommand::DuplicateCommand(robot2D::MessageBus& messageBus,
                                       Scene::Ptr activeScene,
                                       std::vector<SceneEntity> entities):
    m_messageBus{messageBus},
    m_activeScene{activeScene},
    m_entities{std::move(entities)}
    {
    }

    const class_id& DuplicateCommand::id() noexcept {
        static const class_id id{"DuplicateCommand"};
        return id;
    }

    void DuplicateCommand::undo() {
        if(!m_activeScene)
            return;

        for (auto& entity : m_entities) {
            m_activeScene -> removeDuplicate(entity);
            auto* msg = m_messageBus.postMessage<EntityRemovement>(MessageID::EntityRemove);
            msg -> entityID = entity.getUUID();
        }

      
    }


    void DuplicateCommand::redo() {
        if(!m_activeScene)
            return;
        for (auto& entity : m_entities) {
            auto* msg = m_messageBus.postMessage<EntityDuplication>(MessageID::EntityDuplicate);
            msg -> entityID = entity.getUUID();
        }
    }

} // namespace editor