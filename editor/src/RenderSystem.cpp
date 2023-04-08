#include <algorithm>

#include <robot2D/Graphics/RenderTarget.hpp>
#include <robot2D/Ecs/EntityManager.hpp>

#include <editor/RendererSystem.hpp>
#include <editor/TextSystem.hpp>
#include <editor/Scene.hpp>

namespace editor {

    RenderSystem::RenderSystem(robot2D::MessageBus& messageBus):
            robot2D::ecs::System(messageBus,typeid(RenderSystem)),
            m_needUpdateZBuffer{false} {
        addRequirement<TransformComponent>();
        addRequirement<DrawableComponent>();
    }

    void RenderSystem::update(float dt) {
        (void)dt;

        for(auto& entity: m_entities) {
            auto& drawable = entity.getComponent<DrawableComponent>();
            if(drawable.m_needUpdateZbuffer) {
                m_needUpdateZBuffer = true;
                drawable.m_needUpdateZbuffer = false;
            }
        }

        if(m_needUpdateZBuffer) {
            std::stable_sort(m_entities.begin(), m_entities.end(),
                      [](const robot2D::ecs::Entity& left, const robot2D::ecs::Entity& right) {
                return left.getComponent<DrawableComponent>().getDepth() <
                        right.getComponent<DrawableComponent>().getDepth();
            });

            m_needUpdateZBuffer = false;
        }
    }

    void RenderSystem::draw(robot2D::RenderTarget& target, robot2D::RenderStates states) const {
        for(auto& ent: m_entities) {
            auto& transform = ent.getComponent<TransformComponent>();
            auto& drawable = ent.getComponent<DrawableComponent>();
            auto t = transform.getTransform();

            if (ent.hasComponent<CameraComponent>() && m_activeScene && m_activeScene -> isRunning()) {
                auto camera = ent.getComponent<CameraComponent>();
                if (camera.isPrimary) {
                    auto pos = transform.getPosition();
                    auto size = transform.getScale();
                    camera.setViewport({pos.x, pos.y, size.x, size.y});
                    target.setView(camera.getView());
                }
            }

            robot2D::RenderStates renderStates;
            renderStates.transform *= t;
            if(drawable.hasTexture())
                renderStates.texture = &drawable.getTexture();
            renderStates.color = drawable.getColor();
            renderStates.layerID = drawable.getLayerIndex();

            if(getScene() -> hasSystem<TextSystem>() && ent.hasComponent<TextComponent>()) {
                auto textSystem = getScene() -> getSystem<TextSystem>();
                auto vertexArray =  textSystem -> getVertexArray();
                renderStates.shader = const_cast<robot2D::ShaderHandler*>(&textSystem -> getShader());
                renderStates.shader -> use();
                auto view = target.getView(drawable.getLayerIndex()).getTransform();
                renderStates.shader -> setMatrix("projection", view.get_matrix());
                renderStates.shader -> unUse();
                renderStates.renderInfo.indexCount = textSystem -> getIndexCount();
                target.draw(vertexArray, renderStates);
            }
            else {
                auto& vertices = drawable.getVertices();
                vertices[0].position = renderStates.transform * robot2D::vec2f {0.F, 0.F};
                vertices[1].position = renderStates.transform * robot2D::vec2f {1.F, 0.F};
                vertices[2].position = renderStates.transform * robot2D::vec2f {1.F, 1.F};
                vertices[3].position = renderStates.transform * robot2D::vec2f {0.F, 1.F};

                renderStates.entityID = ent.getIndex();

                robot2D::VertexData vertexData{};
                for(const auto& vertex: vertices)
                    vertexData.emplace_back(vertex);
                target.draw(renderStates);
                // target.draw(vertexData, renderStates);
            }

        }
    }

    void RenderSystem::setScene(Scene* scene) {
        m_activeScene = scene;
    }
}