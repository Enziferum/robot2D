#pragma once

#include "Scene.hpp"
#include <robot2D/Ecs/Scene.hpp>
#include <robot2D/Graphics/View.hpp>

namespace sandbox {
    class EditorUIScene: public Scene {
    public:
        explicit EditorUIScene(robot2D::RenderWindow&);
        ~EditorUIScene() override = default;

        virtual void setup() override;
        virtual void handleEvents(const robot2D::Event &event) override;
        virtual void update(float dt) override;
        virtual void imGuiRender() override;
        virtual void render() override;

    private:
        void windowFunction();
        void removeEntity(robot2D::ecs::Entity&& entity);
    private:
        robot2D::MessageBus m_messageBus;
        robot2D::ecs::Scene m_scene;
        std::vector<robot2D::ecs::Entity> m_attachedEntities{};
        robot2D::View m_view;
    };
}