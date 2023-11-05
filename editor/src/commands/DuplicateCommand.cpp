#include <editor/commands/DuplicateCommand.hpp>
#include <editor/Messages.hpp>

namespace editor {
    DuplicateCommand::DuplicateCommand(robot2D::MessageBus& messageBus,
                                       Scene::Ptr activeScene,
                                       robot2D::ecs::Entity entity):
    m_messageBus{messageBus},
    m_activeScene{activeScene},
    m_duplicateEntity{std::move(entity)}
    {
    }

    const class_id& DuplicateCommand::id() noexcept {
        static const class_id id{"DuplicateCommand"};
        return id;
    }

    void DuplicateCommand::undo() {
        if(!m_activeScene)
            return;
        m_activeScene -> removeDuplicate(m_duplicateEntity);
        auto* msg = m_messageBus.postMessage<EntityRemovement>(MessageID::EntityRemove);
        msg -> entityID = m_duplicateEntity.getComponent<IDComponent>().ID;
    }


    void DuplicateCommand::redo() {
        if(!m_activeScene)
            return;

        auto* msg = m_messageBus.postMessage<EntityDuplication>(MessageID::EntityDuplicate);
        msg -> entityID = m_duplicateEntity.getComponent<IDComponent>().ID;
    }

} // namespace editor