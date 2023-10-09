#include <robot2D/Util/Logger.hpp>
#include <editor/commands/DeleteEntitiesCommand.hpp>
#include <editor/Messages.hpp>
#include <editor/Components.hpp>

namespace editor {

    DeleteEntitiesCommand::DeleteEntitiesCommand(robot2D::MessageBus& messageBus,
                                                 std::vector<robot2D::ecs::Entity> entities):
                                                 m_messageBus{messageBus},
                                                 m_entities(entities)
                                                 {}

    void DeleteEntitiesCommand::undo() {
        RB_EDITOR_INFO("DeleteEntitiesCommand: undo command");

        for(auto ent: m_entities) {
            auto* msg = m_messageBus.postMessage<EntityRestorement>(MessageID::EntityRestore);
            msg -> entityID = ent.getComponent<IDComponent>().ID;
        }


        m_entities.clear();
    }
}