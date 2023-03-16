#include <gtest/gtest.h>
#include <robot2D/Ecs/EntityManager.hpp>

namespace {
    class EcsTest: public ::testing::Test {
    protected:
        robot2D::ecs::ComponentManager m_componentManager;
    };
}

TEST_F(EcsTest, EntityManager_test) {
    robot2D::ecs::EntityManager entityManager{m_componentManager};
    auto entity = entityManager.createEntity();
}

TEST_F(EcsTest, entityManager_many_entities_test) {
    constexpr int entitiesCount = 10;
    robot2D::ecs::EntityManager entityManager{m_componentManager};
    for(int i = 0; i < entitiesCount; ++i) {
        auto entity = entityManager.createEntity();
    }

}