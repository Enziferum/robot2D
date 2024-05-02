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