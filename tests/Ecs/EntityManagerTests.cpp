#include <gtest/gtest.h>
#include <robot2D/Ecs/EntityManager.hpp>
#include "robot2D/Ecs/Scene.hpp"

namespace {
    class EcsTest: public ::testing::Test {
    protected:
        robot2D::ecs::ComponentManager m_componentManager;
        robot2D::MessageBus m_messagebus;
    };
}

TEST_F(EcsTest, EntityManager_test) {
    robot2D::ecs::Scene scene{m_messagebus};
    robot2D::ecs::EntityManager entityManager{m_componentManager, &scene};
    auto entity = entityManager.createEntity();
}

TEST_F(EcsTest, entityManager_many_entities_test) {
    constexpr int entitiesCount = 10;
    robot2D::ecs::Scene scene{m_messagebus};

    robot2D::ecs::EntityManager entityManager{m_componentManager, &scene};
    for(int i = 0; i < entitiesCount; ++i) {
        auto entity = entityManager.createEntity();
    }

}