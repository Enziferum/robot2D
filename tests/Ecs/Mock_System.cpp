#include <robot2D/Ecs/System.hpp>


namespace testing {
    class MockSystem: public robot2D::ecs::System {
    public:
        MockSystem(robot2D::MessageBus& messageBus);
        ~MockSystem() override = default;

        void update(float dt) override;

        void onEntityAdded(robot2D::ecs::Entity entity) override;
        void onEntityRemoved(robot2D::ecs::Entity entity) override;
    };
}