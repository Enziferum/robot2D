#include <gtest/gtest.h>
#include <robot2D/Ecs/ComponentContainer.hpp>
#include <robot2D/Ecs/EntityManager.hpp>
#include <memory>
#include <iostream>

namespace {

    struct TestComponent {
        float x;
        float y;
    };

    class TestComponentCustomDestroy: public robot2D::ecs::CustomDestroyComponent {
    public:
        ~TestComponentCustomDestroy() override {}
        void destroy() override {
            std::cout << "TestComponentCustomDestroy::Custom Destroy" << std::endl;
        }
    };

    class ComponentContainerTest: public ::testing::Test {
    protected:
        robot2D::ecs::ComponentManager m_componentManager;
    };
}


TEST_F(ComponentContainerTest, has_entity) {
    robot2D::ecs::EntityManager entityManager{m_componentManager, nullptr};
    auto entity = entityManager.createEntity(false);
    TestComponentCustomDestroy testComponentCustomDestroy;

    auto container =
            std::make_shared<robot2D::ecs::ComponentContainer<TestComponentCustomDestroy>>(
                    m_componentManager.getID<TestComponentCustomDestroy>());

    container -> operator[](entity.getIndex()) = testComponentCustomDestroy;
    ASSERT_TRUE(container -> hasEntity(entity.getIndex()));
}

TEST_F(ComponentContainerTest, remove_with_custom_destroy) {

    robot2D::ecs::EntityManager entityManager{m_componentManager, nullptr};
    auto entity = entityManager.createEntity(false);
    TestComponentCustomDestroy testComponentCustomDestroy;

    auto container =
            std::make_shared<robot2D::ecs::ComponentContainer<TestComponentCustomDestroy>>(
                    m_componentManager.getID<TestComponentCustomDestroy>());


    container -> operator[](entity.getIndex()) = testComponentCustomDestroy;

    EXPECT_EQ(container -> getSize(), 1);
    container -> removeEntity(entity.getIndex());
    EXPECT_EQ(container -> getSize(), 0);
}

TEST_F(ComponentContainerTest, duplicate) {
    robot2D::ecs::EntityManager entityManager{m_componentManager, nullptr};
    auto entity = entityManager.createEntity(false);
    auto entity1 = entityManager.createEntity(false);

    TestComponent testComponent{100, 500};

    auto container =
            std::make_shared<robot2D::ecs::ComponentContainer<TestComponent>>(
                    m_componentManager.getID<TestComponent>());

    container -> operator[](entity.getIndex()) = testComponent;
    container -> duplicate(entity.getIndex(), entity1.getIndex());
    auto duplicatedComponent = container -> operator[](entity1.getIndex());

    EXPECT_EQ(duplicatedComponent.x, 100);
    EXPECT_EQ(duplicatedComponent.y, 500);
}