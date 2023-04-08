#include <robot2D/Graphics/GL.hpp>
#include <robot2D/Graphics/RenderTarget.hpp>
#include <robot2D/Graphics/Text.hpp>

namespace robot2D {

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


    Text::Text() = default;
    Text::~Text() = default;

    void Text::setFont(const Font& font) {
        m_font = &font;
        if(!m_initialized)
            setupGL();
    }

    void Text::setText(const std::string& text) {
        m_text = text;
        m_needupdate = true;
    }

    void Text::setPosition(const robot2D::vec2f& pos) {
        Transformable::setPosition(pos);
        m_needupdate = true;
    }

    void Text::setPosition(float posX, float posY) {
        setPosition({posX, posY});
    }

    void Text::setScale(const float& scale) {
        m_needupdate = true;
        m_scaled = true;
    }

    void Text::setupGL() {
        quadBatchRender.initalize({{
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

    void Text::beforeRender() const {
        quadBatchRender.preProcessBatching();
    }

    void Text::updateGeometry() const {
        if (!m_font || !m_needupdate)
            return;

        if (m_bufferCache.empty() || m_scaled) {
            m_bufferCache = m_font -> getBufferCache(1.F);
            m_scaled = false;
        }

        const auto& glyphCharacters = m_font -> getGlyphCharacters();

        quadBatchRender.refresh();
        robot2D::vec2f copyPos = m_pos;
        for (const auto& s: m_text) {
            auto& preQuad = m_bufferCache[s];

            for (int it = 0; it < 4; ++it) {
                Vertex vertex;
                vertex.position = getTransform() * preQuad.vertices[it].position;
                vertex.texCoords = preQuad.vertices[it].uvs;
                vertex.color = m_color.toGL();
                quadBatchRender.pack(std::move(vertex));
            }

            m_pos.x += glyphCharacters.at(s).advance;
            m_update_transform = true;
        }
        m_pos = copyPos;

        quadBatchRender.processBatching();
        m_needupdate = false;
    }

    void Text::afterRender(bool hardUpdate) const {
        if (hardUpdate) {
            quadBatchRender.processBatching();
        }

        unsigned int index = 0;
        const auto& textures = m_font -> getTextures();

        for (const auto& texture: textures) {
            texture -> bind(index);
            ++index;
        }

        quadBatchRender.Bind();
        glDrawElements(GL_TRIANGLES,
                       quadBatchRender.getIndexCount(),
                       GL_UNSIGNED_INT,
                       nullptr);
        quadBatchRender.unBind();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        if (hardUpdate) {
            quadBatchRender.refresh();
        }
    }

    const std::string& Text::getText() const {
        return m_text;
    }

    void Text::draw(RenderTarget& target, RenderStates states) const {
        m_textShader.use();
        m_textShader.setMatrix("projection", target.getView().getTransform().get_matrix());
        m_textShader.unUse();

        if (m_needupdate)
            beforeRender();

        if (quadBatchRender.needRefresh()) {
            afterRender(true);
            beforeRender();
        }

        updateGeometry();

        states.shader = const_cast<ShaderHandler*>(&m_textShader);
        states.texture = m_font -> getTextures()[0].get();
        states.renderInfo.indexCount = quadBatchRender.getIndexCount();
        target.draw(quadBatchRender.getVertexArray(), states);

        //afterRender(false);
    }

    void Text::setColor(const Color& color) {
        m_color = color;
    }

    const robot2D::Color& Text::getColor() const {
        return m_color;
    }

    void Text::move(const vec2f &offset) {
        Transformable::move(offset);
        m_needupdate = true;
    }

    void Text::scale(const vec2f &factor) {
        Transformable::scale(factor);
        m_needupdate = true;
    }

    void Text::rotate(float angle) {
        Transformable::rotate(angle);
        m_needupdate = true;
    }

}
