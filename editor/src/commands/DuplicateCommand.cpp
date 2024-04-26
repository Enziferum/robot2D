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