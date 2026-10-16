#pragma once

#include <robot2D/Ecs/System.hpp>

namespace editor {

    class AnimatorSystem : public robot2D::ecs::System {
    public:
        explicit AnimatorSystem(robot2D::MessageBus& messageBus);
        AnimatorSystem(const AnimatorSystem& other) = delete;
        AnimatorSystem& operator=(const AnimatorSystem& other) = delete;
        AnimatorSystem(AnimatorSystem&& other) = delete;
        AnimatorSystem& operator=(AnimatorSystem&& other) = delete;
        ~AnimatorSystem() override = default;

        void update(float dt) override;
    protected:
        robot2D::ecs::System::Ptr cloneSelf(robot2D::ecs::Scene* scene,
            const std::vector<robot2D::ecs::Entity>& entities) override;
    };

} // namespace editor