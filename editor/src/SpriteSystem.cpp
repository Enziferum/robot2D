#include <robot2D/Ecs/EntityManager.hpp>
#include <editor/Components.hpp>
#include <editor/SpriteSystem.hpp>

namespace editor {
    SpriteSystem::SpriteSystem(robot2D::MessageBus& messageBus):
        robot2D::ecs::System(messageBus, typeid(SpriteSystem)) {
        addRequirement<SpriteComponent>();
        addRequirement<DrawableComponent>();
    }

    void SpriteSystem::update([[maybe_unused]] float dt) {
        for(auto& entity: m_entities) {
            auto& sprite = entity.getComponent<SpriteComponent>();
            if(!sprite.m_hasUpdate)
                continue;

            auto& drawable = entity.getComponent<DrawableComponent>();
            const auto* texture = sprite.getTexture();
            robot2D::vec2f tx_s{};

            if (texture) {
                drawable.setTexture(*texture);
                tx_s = {sprite.getTexture() -> getSize().x, sprite.getTexture() -> getSize().y};
            } else
                tx_s = {drawable.getTexture().getSize().x, drawable.getTexture().getSize().y};

            auto &vertices = drawable.getVertices();
            auto textureRect = sprite.getTextureRect();


            if (textureRect.lx + textureRect.width > tx_s.x) {
                /// TODO(a.raag): ??
            }

            if (textureRect.ly + textureRect.height > tx_s.y) {
                /// TODO(a.raag): ??
            }

            robot2D::vec2f min = { textureRect.lx / tx_s.x, textureRect.ly / tx_s.y };
            robot2D::vec2f max = { (textureRect.lx + textureRect.width) / tx_s.x,
                                   (textureRect.ly + textureRect.height) / tx_s.y};

            vertices[0].texCoords = min;
            vertices[1].texCoords = {max.x, min.y};
            vertices[2].texCoords = max;
            vertices[3].texCoords = {min.x, max.y};

            auto& tx = entity.getComponent<TransformComponent>();
            tx.setScale({textureRect.width, textureRect.height});

            sprite.m_hasUpdate = false;
        }
    }


} // namespace editor