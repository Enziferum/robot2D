#pragma once

#include <robot2D/Ecs/System.hpp>
#include <robot2D/Ecs/Scene.hpp>

#include <robot2D/Graphics/Drawable.hpp>

namespace editor {

    class Scene;
    class RenderSystem: public robot2D::ecs::System, public robot2D::Drawable {
    public:
        RenderSystem(robot2D::MessageBus& messageBus);
        ~RenderSystem() override = default;

        void setScene(Scene* scene);
        void update(float dt) override;
        void draw(robot2D::RenderTarget& target, robot2D::RenderStates states) const override;
    private:
        bool m_needUpdateZBuffer;
        Scene* m_activeScene{nullptr};
    };

}