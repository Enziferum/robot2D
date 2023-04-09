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

#include <algorithm>
#include <editor/Components.hpp>



namespace editor {
    DrawableComponent::DrawableComponent():
    m_color(robot2D::Color::White),
    m_texture(nullptr) {}

    void DrawableComponent::setTexture(const robot2D::Texture &texture) {
        m_texture = &texture;
    }

    robot2D::Texture& DrawableComponent::getTexture() {
        return const_cast<robot2D::Texture &>(*m_texture);
    }

    const robot2D::Texture& DrawableComponent::getTexture() const {
        return *m_texture;
    }

    void DrawableComponent::setColor(const robot2D::Color& color) {
        m_color = color;
    }

    const robot2D::Color& DrawableComponent::getColor() const {
        return m_color;
    }


    void DrawableComponent::setDepth(int value) {
        m_depth = value;
        m_needUpdateZbuffer = true;
    }

    int DrawableComponent::getDepth() const {
        return m_depth;
    }

    void DrawableComponent::setLayerIndex(unsigned int value) {
        m_layerIndex = value;
    }

     unsigned int DrawableComponent::getLayerIndex() const {
        return m_layerIndex;
    }

    void DrawableComponent::setQuadVertexArray(const quadVertexArray& array) {
        m_vertices = array;
    }

    const quadVertexArray& DrawableComponent::getVertices() const {
        return m_vertices;
    }

    quadVertexArray& DrawableComponent::getVertices() {
        return m_vertices;
    }

    SceneCamera::SceneCamera()
    {
        RecalculateProjection();
    }

    void SceneCamera::SetOrthographic(float size, float nearClip, float farClip)
    {
        m_ProjectionType = ProjectionType::Orthographic;
        m_OrthographicSize = size;
        m_OrthographicNear = nearClip;
        m_OrthographicFar = farClip;
        RecalculateProjection();
    }

    void SceneCamera::SetViewportSize(uint32_t width, uint32_t height)
    {
       // HZ_CORE_ASSERT(width > 0 && height > 0);
        m_AspectRatio = (float)width / (float)height;
        RecalculateProjection();
    }

    void SceneCamera::RecalculateProjection()
    {
        m_AspectRatio = 16 / 9;
        float orthoLeft = -m_OrthographicSize * m_AspectRatio * 0.5f;
        float orthoRight = m_OrthographicSize * m_AspectRatio * 0.5f;
        float orthoBottom = -m_OrthographicSize * 0.5f;
        float orthoTop = m_OrthographicSize * 0.5f;

        m_Projection = glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, m_OrthographicNear, m_OrthographicFar);

    }


    void TransformComponent::setPosition(const robot2D::vec2f& pos) {
        for(auto child: m_children) {
            auto& transform = child.getComponent<TransformComponent>();
            robot2D::vec2f offset = m_pos - child.getComponent<TransformComponent>().getPosition();
            transform.setPosition(pos - offset);
        }
        Transformable::setPosition(pos);
    }

    void TransformComponent::addChild(robot2D::ecs::Entity entity) {
        auto found = std::find_if(m_children.begin(), m_children.end(), [&entity](robot2D::ecs::Entity ent) {
            return entity == ent;
        });
        if(found == m_children.end()) {
            entity.getComponent<TransformComponent>().m_childID = entity.getIndex();
            entity.getComponent<TransformComponent>().m_parent = this;
            m_children.emplace_back(entity);
        }
    }

    void TransformComponent::removeChild(robot2D::ecs::Entity entity, bool removeFromScene ) {
        auto found = std::remove_if(m_children.begin(), m_children.end(), [&entity](robot2D::ecs::Entity ent) {
            return entity == ent;
        });

        if(found == m_children.end())
            return;

        if(removeFromScene)
            found -> removeSelf();
        m_children.erase(found, m_children.end());
    }

    bool TransformComponent::hasChildren() const {
        return !m_children.empty();
    }

    void TransformComponent::removeSelf(bool removeFromScene) {
        if(!isChild())
            return;

        m_parent -> removeChild(m_childID, removeFromScene);
    }

    void TransformComponent::removeChild(int childID, bool removeFromScene) {
        auto found = std::find_if(m_children.begin(), m_children.end(), [&childID](robot2D::ecs::Entity entity) {
            return entity.getIndex() == childID;
        });
        if(found == m_children.end())
            return;
        removeChild(*found, removeFromScene);
    }


    TextComponent::TextComponent():
            m_text{""},
            m_characterSize{20},
            m_font{nullptr},
            m_needUpdate{false}{}

    void TextComponent::setText(const std::string& text) {
        m_text = text;
        m_needUpdate = true;
    }

    void TextComponent::setText(std::string&& text) {
        m_text = std::move(text);
        m_needUpdate = true;
    }

    std::string& TextComponent::getText() {
        return m_text;
    }

    const std::string& TextComponent::getText() const {
        return m_text;
    }

    void TextComponent::setCharacterSize(unsigned int value) {
        m_characterSize = value;
        m_needUpdate = true;
    }

    unsigned int TextComponent::getCharacterSize() {
        return m_characterSize;
    }

    void TextComponent::setFont(const robot2D::Font& font) {
        m_font = &font;
        m_texture.create(*(m_font -> getImages()[0]));
    }

    const robot2D::Font* TextComponent::getFont() const {
        return m_font;
    }

    const robot2D::Texture& TextComponent::getTexture() const {
        return m_texture;
    }

    std::unordered_map<int, robot2D::GlyphQuad>& TextComponent::getGlyphCache() {
        if (m_bufferCache.empty() || m_scaled) {
            m_bufferCache = m_font -> getBufferCache(1.F);
            m_scaled = false;
        }
        return m_bufferCache;
    }
}