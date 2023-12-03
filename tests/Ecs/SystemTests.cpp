#include <gtest/gtest.h>
#include <memory>

#include <robot2D/Ecs/Scene.hpp>
#include <robot2D/Ecs/System.hpp>

namespace {

    struct TestComponent {};


    class TestSystem: public robot2D::ecs::System {
    public:
        TestSystem(robot2D::MessageBus& messageBus): robot2D::ecs::System(messageBus, typeid(TestSystem)) {
            addRequirement<TestComponent>();
        }

        ~TestSystem() override = default;

        void update(float dt) override {}

        void onEntityAdded(robot2D::ecs::Entity entity) override {
            m_entityAddCallCount++;
        }

        void onEntityRemoved(robot2D::ecs::Entity entity) override {
            m_entityRemoveCallCount++;
        }

        void onMessage(const robot2D::Message& message) override {

        }

        int m_entityAddCallCount = 0;
        int m_entityRemoveCallCount = 0;
    };

    class SystemTest : public ::testing::Test {
    protected:
        void SetUp() override {
            scene = std::make_unique<robot2D::ecs::Scene>(messageBus);
            scene -> addSystem<TestSystem>(messageBus);
        }

        robot2D::MessageBus messageBus{};
        std::unique_ptr<robot2D::ecs::Scene> scene;
    };

}

TEST_F(SystemTest, RequirementTest) {
    auto entity = scene -> createEntity();
    entity.addComponent<TestComponent>();

    scene -> createEntity();
    scene -> update(0.f);

    auto system = scene -> getSystem<TestSystem>();
    EXPECT_EQ(system -> m_entityAddCallCount, 1);
}

TEST_F(SystemTest, NotRequirementTest) {
    auto entity = scene -> createEntity();
    scene -> update(0.f);

    auto system = scene -> getSystem<TestSystem>();
    EXPECT_FALSE(system -> m_entityAddCallCount == 1);
}

TEST_F(SystemTest, RemoveEntity) {
    auto entity = scene -> createEntity();
    entity.addComponent<TestComponent>();
    scene -> update(0.f);
    scene -> removeEntity(entity);
    scene -> update(0.f);

    auto system = scene -> getSystem<TestSystem>();
    EXPECT_EQ(system -> m_entityRemoveCallCount, 1);
}