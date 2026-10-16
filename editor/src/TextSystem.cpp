/*********************************************************************
   (c) Alex Raag 2026
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

constexpr const char* textVertShader =
    R"(
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

constexpr const char* textFragShader =
    R"(
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

    TextSystem::TextSystem(robot2D::MessageBus& messageBus) :
        robot2D::ecs::System(messageBus, typeid(TextSystem)),
        m_initialized{false} {

        addRequirement<TextComponent>();
        addRequirement<DrawableComponent>();
        addRequirement<TransformComponent>();
    }

    void TextSystem::setupGL() {
        m_textShader.createShader(robot2D::ShaderType::Vertex, textVertShader, false, "Editor.Text.vertex");
        m_textShader.createShader(robot2D::ShaderType::Fragment, textFragShader, false, "Editor.Text.fragment");
        m_textShader.use();
        m_textShader.set("textSampler", 0);
        m_textShader.unUse();

        m_initialized = true;
    }

    const std::vector<std::unique_ptr<TextSystem::Batch> >& TextSystem::getBatches(unsigned int entityIndex) const {
        static const std::vector<std::unique_ptr<Batch> > empty;
        auto found = m_batches.find(entityIndex);
        return found == m_batches.end() ? empty : found -> second;
    }

    void TextSystem::update(float dt) {
        (void)dt;
        if(!m_initialized) setupGL();
        std::unordered_map<unsigned int, bool> used;
        for(auto& entity: m_entities) {
            if(!entity.hasComponent<TextComponent>() || !entity.hasComponent<DrawableComponent>() ||
                !entity.hasComponent<TransformComponent>()) continue;
            const auto index = entity.getIndex();
            used[index] = true;
            auto& batches = m_batches[index];
            auto& text = entity.getComponent<TextComponent>();
            auto& drawable = entity.getComponent<DrawableComponent>();
            if(!text.getFont() || text.getFont() -> getImages().empty()) {
                drawable.setTexture(std::shared_ptr<robot2D::Texture>{});
                batches.clear(); continue;
            }
            drawable.setTexture(text.getTextureHandle());
            auto& tx = entity.getComponent<TransformComponent>();
            const auto& glyphs = text.getFont() -> getGlyphCharacters();
            const auto& cache = text.getGlyphCache();
            robot2D::vec2f pen{0.f, 0.f};
            std::size_t count = 0;
            Batch* batch = nullptr;
            for(unsigned char character: text.getText()) {
                if(character == '\n') {
                    pen.x = 0.f; pen.y += text.getCharacterSize(); continue;
                }
                if((character & 0xc0) == 0x80) continue;
                // The current Font atlas covers ASCII. Keep UTF-8 text intact;
                // one fallback glyph represents each unavailable code point.
                const int code = glyphs.count(character) ? character : '?';
                auto glyph = cache.find(code); auto metrics = glyphs.find(code);
                if(glyph == cache.end() || metrics == glyphs.end()) continue;
                if(!batch || batch -> needRefresh()) {
                    if(batch) batch -> processBatching();
                    if(count == batches.size()) {
                        auto next = std::make_unique<Batch>();
                        next -> initalize({{{robot2D::ElementType::Float2, "Position"},
                                {robot2D::ElementType::Float2, "UVS"}, {robot2D::ElementType::Float4, "Color"}}});
                        batches.push_back(std::move(next));
                    }
                    batch = batches[count++].get();
                    batch -> refresh(); batch -> preProcessBatching();
                }
                for(const auto& point: glyph -> second.vertices) {
                    robot2D::Vertex vertex;
                    vertex.position = tx.getTransformNoScale() * (point.position + pen);
                    vertex.texCoords = point.uvs;
                    vertex.color = drawable.getColor().toGL();
                    batch -> pack(std::move(vertex));
                }
                pen.x += metrics -> second.advance;
            }
            if(batch) batch -> processBatching();
            batches.resize(count);
        }
        for(auto it = m_batches.begin(); it != m_batches.end();) {
            if(!used.count(it -> first)) it = m_batches.erase(it);
            else ++it;
        }
    }
    robot2D::ecs::System::Ptr TextSystem::cloneSelf(robot2D::ecs::Scene* scene,
        const std::vector<robot2D::ecs::Entity>& newEntities) {
        auto clone = std::make_shared<TextSystem>(m_messageBus);
        if(!cloneBase(clone, scene, newEntities)) return nullptr;
        return clone;
    }

}
