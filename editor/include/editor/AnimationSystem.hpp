#pragma once

#include <robot2D/Ecs/System.hpp>

namespace editor {

    class AnimationSystem: public robot2D::ecs::System {
    public:
        AnimationSystem(robot2D::MessageBus& messageBus);
        ~AnimationSystem() override = default;

        void update(float dt) override;
    private:

    };

} // namespace editor