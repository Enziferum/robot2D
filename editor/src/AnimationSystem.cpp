#include <robot2D/Ecs/EntityManager.hpp>
#include <editor/Components.hpp>
#include <editor/AnimationSystem.hpp>

namespace editor {
    AnimationSystem::AnimationSystem(robot2D::MessageBus& messageBus):
        robot2D::ecs::System(messageBus, typeid(AnimationSystem)) {
        addRequirement<AnimationComponent>();
        addRequirement<DrawableComponent>();
    }

    void AnimationSystem::update([[maybe_unused]] float dt) {
        for(auto& entity: m_entities) {
            auto& animation = entity.getComponent<AnimationComponent>();
            if(!animation.m_hasUpdate)
                continue;

            auto& drawable = entity.getComponent<DrawableComponent>();
            const auto* texture = animation.getTexture();
            robot2D::vec2f tx_s{};

            if (texture) {
                drawable.setTexture(*texture);
                auto texSize = animation.getTexture() -> getSize();
                tx_s = texSize.as<float>();
            }
            else {
                auto texSize = drawable.getTexture().getSize();
                tx_s = texSize.as<float>();
            }
               

            auto& vertices = drawable.getVertices();
            auto textureRect = animation.getTextureRect();


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
            //tx.setScale({textureRect.width, textureRect.height});

            animation.m_hasUpdate = false;
        }
    }


} // namespace editor