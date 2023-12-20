#include <robot2D/Ecs/EntityManager.hpp>

#include <editor/AnimatorSystem.hpp>
#include <editor/Components.hpp>
#include <editor/LocalResourceManager.hpp>

namespace editor {

    AnimatorSystem::AnimatorSystem(robot2D::MessageBus& messageBus):
        robot2D::ecs::System(messageBus, typeid(AnimatorSystem)) {

        addRequirement<TransformComponent>();
        addRequirement<AnimatorComponent>();
        addRequirement<DrawableComponent>();
    }

    void AnimatorSystem::update([[maybe_unused]] float dt) {
        for(auto& entity: m_entities) {

            auto& animator = entity.getComponent<AnimatorComponent>();
            if(!animator.isPlaying)
                continue;

            auto& animationComponent = entity.getComponent<AnimationComponent>();
            auto& animation = *animationComponent.getAnimation();

            if(animator.m_animationName != animation.name) {
                animator.Stop(animator.m_animationName);
                continue;
            }

            animator.m_currentFrameTime += animation.framesPerSecond *  dt;
            animator.m_frameID = static_cast<int>(animator.m_currentFrameTime);

            if (animator.m_frameID >= animation.frames.size())
            {
                if (!animation.isLooped)
                {
                    animator.Stop(animator.m_animationName);
                    continue;
                }
                else
                {
                    animator.m_frameID = 0;
                    animator.m_currentFrameTime = 0.F;
                }
            }

            auto rect = animation.getFrame(animator.m_frameID);
            if(rect)
                animationComponent.setTextureRect(*rect);
        }

    }

} // namespace editor