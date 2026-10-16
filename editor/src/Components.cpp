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

#include <algorithm>
#include <editor/Components.hpp>
#include <editor/Scene.hpp>



namespace editor {

    const class_id& DrawableComponent::id() noexcept {
        static const class_id id{"DrawableComponent"};
        return id;
    }

    const class_id& TransformComponent::id() noexcept {
        static const class_id id{"TransformComponent"};
        return id;
    }

    const class_id& TextComponent::id() noexcept {
        static const class_id id{"TextComponent"};
        return id;
    }

    const class_id& CameraComponent::id() noexcept {
        static const class_id id{"CameraComponent"};
        return id;
    }

    const class_id& Collider2DComponent::id() noexcept {
        static const class_id id{"Collider2D"};
        return id;
    }

    const class_id& Physics2DComponent::id() noexcept {
        static const class_id id{"RigidBody2D"};
        return id;
    }

    const class_id& AnimatorComponent::id() noexcept {
        static const class_id id{"Animator"};
        return id;
    }



    DrawableComponent::DrawableComponent() :
        m_color(robot2D::Color::White),
        m_texture(nullptr) {
        m_vertices[0].texCoords = {0.F, 0.F};
        m_vertices[1].texCoords = {1.F, 0.F};
        m_vertices[2].texCoords = {1.F, 1.F};
        m_vertices[3].texCoords = {0.F, 1.F};
    }

    void DrawableComponent::setTexture(const robot2D::Texture& texture) {
        m_texture = &texture;
        if(m_ownedTexture.get() != &texture) m_ownedTexture.reset();
    }

    void DrawableComponent::setTexture(std::shared_ptr<robot2D::Texture> texture) {
        m_ownedTexture = std::move(texture);
        m_texture = m_ownedTexture.get();
    }

    robot2D::Texture& DrawableComponent::getTexture() {
        return const_cast<robot2D::Texture&>(*m_texture);
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

    void DrawableComponent::FlipTexture() {
        if(!m_texture)
            return;

        /// TODO(a.raag) update Vertices Texture Coords

    }

    SceneCamera::SceneCamera() {
        RecalculateProjection();
    }

    void SceneCamera::SetOrthographic(float size, float nearClip, float farClip) {
        m_ProjectionType = ProjectionType::Orthographic;
        m_OrthographicSize = size;
        m_OrthographicNear = nearClip;
        m_OrthographicFar = farClip;
        RecalculateProjection();
    }

    void SceneCamera::SetViewportSize(uint32_t width, uint32_t height) {
        // RB_CORE_ASSERT(width > 0 && height > 0);
        m_AspectRatio = (float)width / (float)height;
        RecalculateProjection();
    }

    void SceneCamera::RecalculateProjection() {
        m_AspectRatio = 16 / 9;

    }

    void TransformComponent::setPosition(const robot2D::vec2f& pos, bool needUpdateChild) {
        m_hasModification = true;
        if(needUpdateChild)
            setPosition(pos);
        else {
            Transformable::setPosition(pos);
        }
    }


    void TransformComponent::setPosition(const robot2D::vec2f& pos) {
        if(m_scene)
            m_scene -> moveChildren(m_entityID, pos - m_pos, m_context, m_contextVersion);
        Transformable::setPosition(pos);
        m_hasModification = true;
    }

    void TransformComponent::setScale(const robot2D::vec2f& factor) {
        Transformable::setScale(factor);
        m_hasModification = true;
    }

    void TransformComponent::setSize(const robot2D::vec2f& factor) {
        Transformable::setSize(factor);
        m_hasModification = true;
    }

    robot2D::FloatRect TransformComponent::getLocalBounds() const {
        float w = std::abs(m_size.x);
        float h = std::abs(m_size.y);
        auto origin = getOrigin();
        if(origin == robot2D::vec2f{})
            return {0.f, 0.f, w, h};
        /// TODO(a.raag) Make calcs
        float xCoof = 2.f;
        float yCoff = 2.f;
        return {-w / xCoof, -h / yCoff, w, h};
    }


    TextComponent::TextComponent() :
        m_text{""},
        m_characterSize{20},
        m_font{nullptr},
        m_needUpdate{false} {
    }

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
        m_bufferCache.clear();
        m_needUpdate = true;
        m_texture.reset();
        if(!font.getImages().empty()) {
            m_texture = std::make_shared<robot2D::Texture>();
            m_texture -> create(*font.getImages().front());
        }
    }

    const robot2D::Font* TextComponent::getFont() const {
        return m_font;
    }

    const robot2D::Texture& TextComponent::getTexture() const {
        return *m_texture;
    }

    std::unordered_map<int, robot2D::GlyphQuad>& TextComponent::getGlyphCache() {
        if(m_bufferCache.empty() || m_scaled) {
            m_bufferCache = m_font -> getBufferCache(1.F);
            m_scaled = false;
        }
        return m_bufferCache;
    }


    void AnimatorComponent::Play(const std::string& animationName) {
        if(isPlaying && m_animationName == animationName)
            return;
        isPlaying = true;
        m_animationName = animationName;
        m_frameID = 0;
        m_currentFrameTime = 0.f;
    }

    void AnimatorComponent::Stop(const std::string& animationName) {
        if(animationName.empty() || m_animationName == animationName)
            isPlaying = false;
    }

}
