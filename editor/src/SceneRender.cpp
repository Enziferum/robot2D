#include <algorithm>
#include <robot2D/Ecs/EntityManager.hpp>
#include <editor/SceneRender.hpp>
#include <editor/Components.hpp>


namespace editor {

    constexpr const char* textVertShader = R"(
        #version 330 core
        layout(location = 0) in vec2 position;
        layout(location = 1) in vec2 uv;
        layout(location = 2) in vec4 color;

        out vec2 textureCoords;
        out vec4 outputColor;
        uniform mat4 projection;

        void main() {
            outputColor = color;
            textureCoords = uv;
            gl_Position = projection * vec4(position, 0.0, 1.0);
        }
    )";

    constexpr const char* textFragShader = R"(
        #version 330 core

        out vec4 fragColor;
        in vec2 textureCoords;
        in vec4 outputColor;

        uniform sampler2D textSampler;

        void main() {
            vec4 texel = texture(textSampler, textureCoords);
            texel = vec4(texel.r, texel.r, texel.r, texel.r);
            if(texel.a == 0.0) {
                discard;
            }
            fragColor = texel * outputColor;
        }
    )";


    void SceneRender::setup() {
        m_quadBatchRender.initalize({{
                                             {robot2D::ElementType::Float2, "Position"},
                                             {robot2D::ElementType::Float2, "UVS"},
                                             {robot2D::ElementType::Float4, "Color"},
                                     }});

        m_textShader.createShader(robot2D::ShaderType::Vertex, textVertShader, false);
        m_textShader.createShader(robot2D::ShaderType::Fragment, textFragShader, false);
        m_textShader.use();
        m_textShader.set("textSampler", 0);
        m_textShader.unUse();
    }


    void SceneRender::update(std::vector<robot2D::ecs::Entity>& entities) {
        for(auto& entity: entities) {
            auto& drawable = entity.getComponent<DrawableComponent>();
            if(drawable.m_needUpdateZbuffer) {
                m_needUpdateZBuffer = true;
                drawable.m_needUpdateZbuffer = false;
            }

            if(entity.hasComponent<TextComponent>()) {
                auto& text = entity.getComponent<TextComponent>();
                entity.getComponent<DrawableComponent>().setTexture(text.getTexture());
                if(text.m_needUpdate) {
                    text.m_needUpdate = false;
                    m_needUpdateText = true;
                }
            }

        }

        if(m_needUpdateZBuffer) {
            std::stable_sort(entities.begin(), entities.end(),
                             [](const robot2D::ecs::Entity& left, const robot2D::ecs::Entity& right) {
                                 return left.getComponent<DrawableComponent>().getDepth() <
                                        right.getComponent<DrawableComponent>().getDepth();
                             });

            m_needUpdateZBuffer = false;
        }

        if(m_needUpdateText) {
            m_quadBatchRender.preProcessBatching();
            m_quadBatchRender.refresh();

            for(auto& ent: entities) {
                if(!ent.hasComponent<TextComponent>())
                    continue;

                auto& textComp = ent.getComponent<TextComponent>();
                if(!textComp.getFont())
                    continue;

                auto& drawable = ent.getComponent<DrawableComponent>();
                auto& tx = ent.getComponent<TransformComponent>();

                auto& m_bufferCache = textComp.getGlyphCache();
                const auto& glyphCharacters = textComp.getFont() -> getGlyphCharacters();

                robot2D::vec2f copyPos = tx.getPosition();
                robot2D::vec2f m_pos = tx.getPosition();

                for (const auto& s: textComp.getText()) {
                    const auto& preQuad = m_bufferCache[s];

                    for (int i = 0; i < 4; ++i) {
                        robot2D::Vertex vertex;
                        vertex.position = tx.getTransform() * preQuad.vertices[i].position;
                        vertex.texCoords = preQuad.vertices[i].uvs;
                        vertex.color = drawable.getColor().toGL();
                        m_quadBatchRender.pack(std::move(vertex));
                    }

                    m_pos.x += glyphCharacters.at(s).advance;
                    tx.setPosition(m_pos);
                }
                tx.setPosition(copyPos);
            }

            m_quadBatchRender.processBatching();
            m_needUpdateText = false;
        }
    }

    void SceneRender::render(robot2D::RenderTarget& target,
                             std::vector<robot2D::ecs::Entity>& entities) {

        if(m_mainCamera && m_runtimeRender) {}

        for(auto& ent: entities) {
            if(!ent.hasComponent<DrawableComponent>())
                continue;

            auto& transform = ent.getComponent<TransformComponent>();
            auto& drawable = ent.getComponent<DrawableComponent>();
            auto t = transform.getTransform();

            robot2D::RenderStates renderStates;
            renderStates.transform *= t;
            if(drawable.hasTexture())
                renderStates.texture = &drawable.getTexture();
            renderStates.color = drawable.getColor();
            renderStates.layerID = drawable.getLayerIndex();

            if(ent.hasComponent<TextComponent>()) {
                if(!ent.getComponent<TextComponent>().getFont())
                    continue;
                auto vertexArray =  m_quadBatchRender.getVertexArray();
                renderStates.shader = const_cast<robot2D::ShaderHandler*>(&m_textShader);
                renderStates.shader -> use();
                auto view = target.getView(drawable.getLayerIndex()).getTransform();
                renderStates.shader -> setMatrix("projection", view.get_matrix());
                renderStates.shader -> unUse();
                renderStates.renderInfo.indexCount = m_quadBatchRender.getIndexCount();
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
            }

            std::vector<robot2D::ecs::Entity> childV;
            for(auto& child: ent.getComponent<TransformComponent>().getChildren())
                childV.emplace_back(child);
            if(!childV.empty())
                render(target, childV);
        }

    }


}