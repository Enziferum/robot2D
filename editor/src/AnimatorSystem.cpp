/*********************************************************************
   (c) Alex Raag 2026
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

#include <cmath>
#include <robot2D/Ecs/EntityManager.hpp>

#include <editor/AnimatorSystem.hpp>
#include <editor/Components.hpp>
#include <editor/LocalResourceManager.hpp>

namespace editor {

    AnimatorSystem::AnimatorSystem(robot2D::MessageBus& messageBus) :
        robot2D::ecs::System(messageBus, typeid(AnimatorSystem)) {

        addRequirement<TransformComponent>();
        addRequirement<AnimatorComponent>();
        addRequirement<AnimationComponent>();
        addRequirement<DrawableComponent>();
    }

    void AnimatorSystem::update(float dt) {
        if(!std::isfinite(dt) || dt < 0.f)
            return;
        for(auto& entity: m_entities) {
            auto& animator = entity.getComponent<AnimatorComponent>();
            if(!animator.isPlaying)
                continue;
            auto& component = entity.getComponent<AnimationComponent>();
            const auto* clip = component.getAnimation();
            if(!clip || clip -> frames.empty() || clip -> framesPerSecond <= 0 ||
                animator.m_animationName != clip -> name) {
                animator.Stop({});
                continue;
            }
            const double count = static_cast<double>(clip -> frames.size());
            animator.m_currentFrameTime += static_cast<double>(clip -> framesPerSecond) * dt;
            if(clip -> isLooped) {
                animator.m_currentFrameTime = std::fmod(animator.m_currentFrameTime, count);
            }
            else if(animator.m_currentFrameTime >= count) {
                animator.m_currentFrameTime = count - 1.0;
                animator.Stop({});
            }
            animator.m_frameID = static_cast<std::uint32_t>(animator.m_currentFrameTime);
            auto* animation = component.getAnimation();
            if(const auto* rect = animation -> getFrame(static_cast<int>(animator.m_frameID)))
                component.setTextureRect(*rect);
        }
    }

    robot2D::ecs::System::Ptr AnimatorSystem::cloneSelf(robot2D::ecs::Scene* scene,
        const std::vector<robot2D::ecs::Entity>& entities) {
        auto copy = std::make_shared<AnimatorSystem>(m_messageBus);
        if(!cloneBase(copy, scene, entities))
            return nullptr;
        // Preview playback is transient editor state, not the runtime's initial state.
        for(auto entity: copy -> m_entities)
            entity.getComponent<AnimatorComponent>() = AnimatorComponent{};
        return copy;
    }

} // namespace editor
