/*********************************************************************
(c) Alex Raag 2021
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

#pragma once

#include <robot2D/Graphics/Transformable.hpp>
#include <robot2D/Graphics/Texture.hpp>
#include <robot2D/Graphics/Color.hpp>
#include <robot2D/Graphics/View.hpp>
#include <robot2D/Graphics/Matrix3D.hpp>
#include <robot2D/Graphics/Math3D.hpp>
#include <robot2D/Graphics/Vertex.hpp>
#include <robot2D/Graphics/Font.hpp>
#include <robot2D/Ecs/Entity.hpp>

#include "Uuid.hpp"

namespace editor {
    struct IDComponent
    {
        UUID ID;

        IDComponent() = default;
        IDComponent(const IDComponent&) = default;
    };

    class TransformComponent: public robot2D::Transformable {
    public:
        ~TransformComponent() override = default;
        void setPosition(const robot2D::vec2f& pos) override;

        void addChild(robot2D::ecs::Entity);
        void removeChild(robot2D::ecs::Entity);
        bool hasChildren() const;

        std::vector<robot2D::ecs::Entity>& getChildren() {
            return m_children;
        }

        const std::vector<robot2D::ecs::Entity>& getChildren() const {
            return m_children;
        }

        bool isChild() const { return (m_parent != nullptr && m_childID != -1); }

        void removeSelf();
    private:
        void removeChild(int childID);
    private:
        int m_childID = -1;
        std::vector<robot2D::ecs::Entity> m_children;
        TransformComponent* m_parent{nullptr};
    };

    // TODO: @a.raag add Rotation
    class Transform3DComponent {
    public:
        robot2D::vec3f& getPosition() { return m_position; }
        const robot2D::vec3f& getPosition() const { return m_position; }
        robot2D::vec3f& getScale() { return m_scale; }
        const robot2D::vec3f& getScale() const { return m_scale; }
        robot2D::vec3f& getRotation() { return m_rotation; }
        const robot2D::vec3f& getRotation() const { return m_rotation; }

        void setPosition(const robot2D::vec3f& vec) {
            m_position = vec;
        }

        void setRotation(const robot2D::vec3f& vec) {
            m_rotation = vec;
        }

        void setScale(const robot2D::vec3f& vec) {
            m_scale = vec;
        }

        robot2D::Matrix3D getTransform() const {
           return robot2D::translate(robot2D::mat4{}, m_position) *
                  robot2D::toMat4(robot2D::quat(m_rotation)) *
                  robot2D::scale(robot2D::mat4{}, m_scale);
        }
    private:
        robot2D::vec3f m_position;
        robot2D::vec3f m_scale;
        robot2D::vec3f m_rotation;
    };

    class TagComponent {
    public:
        TagComponent(const std::string& tag = "Untitled Entity"): m_tag(tag) {}
        ~TagComponent() = default;

        void setTag(const std::string& tag) {m_tag = tag;}
        std::string& getTag() {return m_tag;}
        const std::string& getTag() const {return m_tag;}
    private:
        std::string m_tag;
    };

    using quadVertexArray = std::array<robot2D::Vertex, 4>;

    class DrawableComponent final {
    public:
        DrawableComponent();
        ~DrawableComponent() = default;

        void setTexture(const robot2D::Texture& texture);
        robot2D::Texture& getTexture();
        const robot2D::Texture& getTexture() const;
        const robot2D::Texture* getTexturePointer() const {return m_texture;}

        void setColor(const robot2D::Color& color);
        const robot2D::Color& getColor() const;
        bool hasTexture() const { return m_texture != nullptr; }

        void setDepth(int value);
        int getDepth() const;
        int& getDepth() { return m_depth; }


        void setTexturePath(const std::string& path) { m_texturePath = path; }
        const std::string& getTexturePath() const { return m_texturePath; }

        void setLayerIndex(unsigned int value);
        unsigned int getLayerIndex() const;

        void setQuadVertexArray(const quadVertexArray& array);
        const quadVertexArray& getVertices() const;
        quadVertexArray& getVertices();

        void setReorderZBuffer(bool flag) { m_needUpdateZbuffer = flag; }
    private:
        friend class RenderSystem;

        quadVertexArray m_vertices;

        int m_depth{1};
        unsigned int m_layerIndex{1};

        const robot2D::Texture* m_texture{nullptr};
        robot2D::Color m_color;
        bool m_needUpdateZbuffer{false};
        std::string m_texturePath{""};
    };


    class CameraComponent final {
    public:
        CameraComponent();
        ~CameraComponent() = default;

        /// 2D Space Camera ///
        void setViewport(const robot2D::FloatRect& viewport);
        robot2D::View& getView() { return m_view; }
        const robot2D::View& getView() const { return m_view; }
        float getZoom() const;

        bool isPrimary{false};
    private:
        robot2D::View m_view;
        robot2D::FloatRect m_actualViewport;
    };


    class Collider2DComponent final {
    public:
        Collider2DComponent() = default;
        ~Collider2DComponent() = default;

        robot2D::vec2f offset;
        robot2D::vec2f size = { 0.5f, 0.5f };

        float density = 1.0f;
        float friction =  0.5f;
        float restitution = 0.0f;
        float restitutionThreshold = 0.5f;

        void* runtimeBody{nullptr};
    };

    class Physics2DComponent final {
    public:
        Physics2DComponent() = default;
        ~Physics2DComponent() = default;



        enum class BodyType { Static = 0, Dynamic, Kinematic};
        BodyType type = BodyType::Static;
        bool fixedRotation = false;
        void* runtimeBody{nullptr};
    };

    struct ScriptComponent {
        std::string name;
    };


    class TextComponent: public robot2D::Transformable {
    public:
        TextComponent();
        ~TextComponent() override = default;

        void setText(const std::string& text);
        void setText(std::string&& text);

        std::string& getText();
        const std::string& getText() const;

        void setCharacterSize(unsigned int value);
        unsigned int getCharacterSize();

        void setFont(const robot2D::Font& font);
        const robot2D::Font* getFont() const;

        const robot2D::Texture& getTexture() const;
        std::unordered_map<int, robot2D::GlyphQuad>& getGlyphCache();
    private:
        friend class TextSystem;

        std::string m_text;
        unsigned int m_characterSize;
        const robot2D::Font* m_font;
        bool m_needUpdate;
        std::unordered_map<int, robot2D::GlyphQuad> m_bufferCache;
        friend class TextSystem;

        bool m_scaled = false;
    };

}
