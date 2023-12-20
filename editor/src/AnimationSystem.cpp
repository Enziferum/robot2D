#include <robot2D/Ecs/EntityManager.hpp>

#include <editor/AnimationSystem.hpp>
#include <editor/Components.hpp>

namespace editor {

    AnimationSystem::AnimationSystem(robot2D::MessageBus& messageBus):
        robot2D::ecs::System(messageBus, typeid(AnimationSystem)) {

        addRequirement<TransformComponent>();
        addRequirement<AnimationComponent>();
        addRequirement<DrawableComponent>();
    }

    void AnimationSystem::update(float dt) {
        for(auto& entity: m_entities) {

            auto& animationComponent = entity.getComponent<AnimationComponent>();
            if(!animationComponent.isPlaying)
                continue;

            auto& sprite = entity.getComponent<SpriteComponent>();
            auto& animations = sprite.getAnimations();
            auto& animation = animations[animationComponent.m_animationID];

            if(animations.empty()
               || animationComponent.m_animationID >= animations.size()) {
                animationComponent.Stop();
                continue;
            }

            animationComponent.m_currentFrameTime += animation.speed *  dt;
            animationComponent.m_frameID = static_cast<int>(animationComponent.m_currentFrameTime);

            if (animationComponent.m_frameID >= animation.frames.size())
            {
                if (!animation.isLooped)
                {
                    animationComponent.Stop();
                    continue;
                }
                else
                {
                    animationComponent.m_frameID = 0;
                    animationComponent.m_currentFrameTime = 0.F;
                }
            }

            robot2D::IntRect rect{};
            if(!animation.isFlipped)
                rect = animation.frames[animationComponent.m_frameID];
            else
                rect = animation.flip_frames[animationComponent.m_frameID];

            sprite.setTextureRect(rect);
        }

    }

} // namespace editor