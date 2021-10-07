#include <robot2D/Graphics/RenderTarget.hpp>
#include <robot2D/Ecs/EntityManager.hpp>

#include <editor/Components.hpp>
#include <editor/Systems.hpp>

namespace editor {
    RenderSystem::RenderSystem(robot2D::MessageBus& messageBus):
            System(messageBus, typeid(RenderSystem)) {
        addRequirement<SpriteComponent>();
        addRequirement<TransformComponent>();
    }


    void RenderSystem::draw(robot2D::RenderTarget& target, robot2D::RenderStates states) const {
        for(auto& it: m_entities) {
            auto& transform = it.getComponent<TransformComponent>();
            auto& sprite = it.getComponent<SpriteComponent>();
            states.transform = transform.getTransform();
            states.texture = &sprite.getTexture();
            states.color = sprite.getColor();
            target.draw(states);
        }
    }

}