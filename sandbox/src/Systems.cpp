#include <sandbox/Systems.hpp>
#include <sandbox/Components.hpp>
#include <robot2D/Ecs/EntityManager.hpp>

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


DemoMoveSystem::DemoMoveSystem(robot2D::MessageBus& messageBus):
        robot2D::ecs::System(messageBus, typeid(DemoMoveSystem)),
        m_speed{10.F}{
    addRequirement<TransformComponent>();
}

void DemoMoveSystem::update(float dt) {
    for(auto& it: m_entities) {
        auto& transform = it.getComponent<TransformComponent>();
        transform.move(robot2D::vec2f(m_speed * dt, 0.f));
    }
}
