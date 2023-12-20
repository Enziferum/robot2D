#pragma once

#include <robot2D/Ecs/System.hpp>

namespace editor {

    class SpriteSystem: public robot2D::ecs::System {
    public:
        SpriteSystem(robot2D::MessageBus& messageBus);
        ~SpriteSystem() override = default;;

        void update(float dt) override;
    };

} // namespace editor