#include <gtest/gtest.h>
#include <memory>

#include <robot2D/Ecs/Entity.hpp>
#include <robot2D/Ecs/EntityManager.hpp>
#include <robot2D/Ecs/Scene.hpp>

namespace  {
    class EntityTest : public ::testing::Test {
    protected:
        void SetUp() override {
            scene = std::make_unique<robot2D::ecs::Scene>(messageBus);
        }

        robot2D::MessageBus messageBus{};
        std::unique_ptr<robot2D::ecs::Scene> scene;
    };

    struct TestComponent {};
}


TEST_F(EntityTest, Ecs_EntityCreate_Test) {
    robot2D::ecs::Entity entity = scene -> createEntity();
    EXPECT_TRUE(entity);
}

TEST_F(EntityTest, Ecs_EntityMore_Test) {
//    robot2D::ecs::Entity entity = scene -> createEntity();
//    robot2D::ecs::Entity lessEntity = scene -> createEntity();
//    EXPECT_TRUE(!(entity < lessEntity));
}

TEST_F(EntityTest, Ecs_EntityLess_Test) {
    robot2D::ecs::Entity entity = scene -> createEntity();
    robot2D::ecs::Entity moreEntity = scene -> createEntity();
    EXPECT_TRUE(entity < moreEntity);
}

TEST_F(EntityTest, Ecs_EntityHasComponent_Test) {
    robot2D::ecs::Entity entity = scene -> createEntity();
    entity.addComponent<TestComponent>();
    EXPECT_TRUE(entity.hasComponent<TestComponent>());
}

TEST_F(EntityTest, Ecs_EntityNotHasComponent_Test) {
    robot2D::ecs::Entity entity = scene -> createEntity();
    EXPECT_FALSE(entity.hasComponent<TestComponent>());
}