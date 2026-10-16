
#pragma once

#include <robot2D/Ecs/System.hpp>

namespace editor {

    class AnimationSystem : public robot2D::ecs::System {
    public:
        AnimationSystem(robot2D::MessageBus& messageBus);
        AnimationSystem(const AnimationSystem& other) = delete;
        AnimationSystem& operator=(const AnimationSystem& other) = delete;
        AnimationSystem(AnimationSystem&& other) = delete;
        AnimationSystem& operator=(AnimationSystem&& other) = delete;
        ~AnimationSystem() override = default;

        void update(float dt) override;
    protected:
        robot2D::ecs::System::Ptr cloneSelf(robot2D::ecs::Scene* scene,
            const std::vector<robot2D::ecs::Entity>& entities) override;
    };

} // namespace editor