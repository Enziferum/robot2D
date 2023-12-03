/*********************************************************************
(c) Alex Raag 2023
https://github.com/Enziferum
robot2D - Zlib license.
This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:
1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.
2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any
source distribution.
*********************************************************************/

#include <robot2D/Ecs/EntityManager.hpp>

#include <editor/TextSystem.hpp>
#include <editor/Components.hpp>

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

namespace editor {

    TextSystem::TextSystem(robot2D::MessageBus& messageBus):
        robot2D::ecs::System(messageBus, typeid(TextSystem)),
        m_initialized{false},
        m_needUpdate{false}{

        addRequirement<TextComponent>();
        addRequirement<DrawableComponent>();
        addRequirement<TransformComponent>();
    }

    void TextSystem::setupGL() {
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

        m_initialized = true;
    }

    void TextSystem::update(float dt) {
        if(!m_initialized)
            setupGL();

        (void)dt;
        for(auto& entity: m_entities) {
            auto& text = entity.getComponent<TextComponent>();

            entity.getComponent<DrawableComponent>().setTexture(text.getTexture());
            if(text.m_needUpdate) {
                text.m_needUpdate = false;
                m_needUpdate = true;
            }
        }

     //   if(m_needUpdate) {
            m_quadBatchRender.preProcessBatching();
            m_quadBatchRender.refresh();

            for(auto& ent: m_entities) {
                auto& textComp = ent.getComponent<TextComponent>();
                auto& drawable = ent.getComponent<DrawableComponent>();
                auto& tx = ent.getComponent<TransformComponent>();

                if(!textComp.getFont())
                    continue;

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
            m_needUpdate = false;
    //    }
    }
}