#pragma once

#include <robot2D/Ecs/Scene.hpp>
#include "Scene.hpp"

namespace sandbox {

    class EcsScene: public Scene {
    public:
        EcsScene(robot2D::RenderWindow& );
        EcsScene(const EcsScene& other) = delete;
        EcsScene& operator=(const EcsScene& other) = delete;
        EcsScene(EcsScene&& other) = delete;
        EcsScene& operator=(EcsScene&& other) = delete;
        ~EcsScene() override = default;

        void setup() override;

        void handleEvents(const robot2D::Event& event) override;
        void update(float dt) override;
        void render() override;
    private:
        robot2D::MessageBus m_messageBus;
        robot2D::ecs::Scene m_scene;
    };

}