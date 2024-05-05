
#pragma once

#include <robot2D/Ecs/System.hpp>

namespace editor {

    class AnimationSystem: public robot2D::ecs::System {
    public:
        AnimationSystem(robot2D::MessageBus& messageBus);
        AnimationSystem(const AnimationSystem& other) = delete;
        AnimationSystem& operator=(const AnimationSystem& other) = delete;
        AnimationSystem(AnimationSystem&& other) = delete;
        AnimationSystem& operator=(AnimationSystem&& other) = delete;
        ~AnimationSystem() override = default;

        void update(float dt) override;
    };

} // namespace editor