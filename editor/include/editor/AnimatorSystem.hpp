#pragma once

#include <robot2D/Ecs/System.hpp>

namespace editor {

    class AnimatorSystem: public robot2D::ecs::System {
    public:
        AnimatorSystem(robot2D::MessageBus& messageBus);
        ~AnimatorSystem() override = default;

        void update(float dt) override;
    private:

    };

} // namespace editor