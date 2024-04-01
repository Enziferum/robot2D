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

#pragma once
#include <string>
#include <unordered_map>

#include <robot2D/Graphics/Transformable.hpp>
#include <robot2D/Graphics/Texture.hpp>
#include <robot2D/Graphics/Color.hpp>
#include <robot2D/Graphics/View.hpp>
#include <robot2D/Graphics/Matrix3D.hpp>
#include <robot2D/Graphics/Math3D.hpp>
#include <robot2D/Graphics/Vertex.hpp>
#include <robot2D/Graphics/Font.hpp>
#include <robot2D/Ecs/Entity.hpp>

#include "editor/panels/ITreeItem.hpp"
#include <editor/Animation.hpp>
#include "Uuid.hpp"
#include "Property.hpp"
#include "ClassID.hpp"
#include "SceneEntity.hpp"

namespace editor {
    struct IDComponent
    {
        IDComponent() = default;
        ~IDComponent() = default;
        IDComponent(const IDComponent& other) = default;
        IDComponent& operator=(const IDComponent& other) = default;
        IDComponent(IDComponent&& other) = default;
        IDComponent& operator=(IDComponent&& other) = default;

        UUID ID;
    };

    class TransformComponent: public robot2D::Transformable {
    public:
        DECLARE_COMPONENT_ID()

        ~TransformComponent() override = default;
        void setPosition(const robot2D::vec2f& pos, bool needUpdateChild);
        void setPosition(const robot2D::vec2f& pos) override;

        void addChild(robot2D::ecs::Entity parent, robot2D::ecs::Entity child);
        void removeChild(robot2D::ecs::Entity, bool removeFromScene = true);
        void clearChildren() { m_children.clear(); }
        bool hasChildren() const;

        robot2D::FloatRect getLocalBounds() const {
            float w = std::abs(m_size.x);
            float h = std::abs(m_size.y);
            return {0.f, 0.f, w, h};
        }

        robot2D::FloatRect getGlobalBounds() const {
            return getTransform().transformRect(getLocalBounds());
        }

        std::vector<robot2D::ecs::Entity>& getChildren() {
            return m_children;
        }

        const std::vector<robot2D::ecs::Entity>& getChildren() const {
            return m_children;
        }

        bool isChild() const { return ( (m_parent && !m_parent.destroyed()) && m_childID != -1); }

        void removeSelf(bool removeFromScene = true);

        robot2D::ecs::Entity getParent() { return m_parent; }
    private:
        void removeChild(int childID, bool removeFromScene);
    private:
        int m_childID = -1;
        std::vector<robot2D::ecs::Entity> m_children;
        robot2D::ecs::Entity m_parent;
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
        DECLARE_COMPONENT_ID()

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
        bool isUtil{false};

        void FlipTexture();
    private:
        friend class RenderSystem;
        friend class SceneRender;

        quadVertexArray m_vertices;

        int m_depth{1};
        unsigned int m_layerIndex{1};

        const robot2D::Texture* m_texture{nullptr};
        robot2D::Color m_color;
        bool m_needUpdateZbuffer{false};
        std::string m_texturePath{""};

    };


    class SceneCamera
    {
    public:
        enum class ProjectionType { Perspective = 0, Orthographic = 1 };
    public:
        SceneCamera();
        virtual ~SceneCamera() = default;

        void SetPerspective(float verticalFOV, float nearClip, float farClip);
        void SetOrthographic(float size, float nearClip, float farClip);
        void SetViewportSize(uint32_t width, uint32_t height);

        float GetOrthographicSize() const { return m_OrthographicSize; }
        void SetOrthographicSize(float size) { m_OrthographicSize = size; RecalculateProjection(); }
        float GetOrthographicNearClip() const { return m_OrthographicNear; }
        void SetOrthographicNearClip(float nearClip) { m_OrthographicNear = nearClip; RecalculateProjection(); }
        float GetOrthographicFarClip() const { return m_OrthographicFar; }
        void SetOrthographicFarClip(float farClip) { m_OrthographicFar = farClip; RecalculateProjection(); }

        ProjectionType GetProjectionType() const { return m_ProjectionType; }
        void SetProjectionType(ProjectionType type) { m_ProjectionType = type; RecalculateProjection(); }

    private:
        void RecalculateProjection();
    private:
        ProjectionType m_ProjectionType = ProjectionType::Orthographic;

        float m_OrthographicSize = 10.0f;
        float m_OrthographicNear = -1.0f, m_OrthographicFar = 1.0f;
        float m_AspectRatio = 0.0f;
    };


    struct CameraComponent final {
    public:
        DECLARE_COMPONENT_ID()

        enum class AspectRatio {
            Desktop = 0, Mobile
        };


        CameraComponent() = default;
        ~CameraComponent() = default;


        robot2D::vec2f getSize() const { return size; }
        robot2D::vec2f getPosition() const { return position; }

       // Property<float> Size;

       // PROPERTY_GET(Size) {
       //     return Size;
      //  }

      //  PROPERTY_SET(Size) {
      //      if(oldValue != newValue) {
                ///
      //      }
      //  }



        AspectRatio aspectRatio;
        float orthoSize;
        robot2D::vec2f size;
        robot2D::vec2f position;

        SceneCamera camera;
        bool isPrimary{false};
    };


    class Collider2DComponent final {
    public:
        DECLARE_COMPONENT_ID()

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
        DECLARE_COMPONENT_ID()

        Physics2DComponent() = default;
        ~Physics2DComponent() = default;

        enum class BodyType { Static = 0, Dynamic, Kinematic};
        BodyType type = BodyType::Static;

        bool fixedRotation = false;
        void* runtimeBody{nullptr};
    };

    class TextComponent: public robot2D::Transformable {
    public:
        DECLARE_COMPONENT_ID()

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

        void setFontPath(const std::string& path) { m_fontPath = path;}
        const std::string& getFontPath() const { return m_fontPath; }

        const robot2D::Texture& getTexture() const;
        std::unordered_map<int, robot2D::GlyphQuad>& getGlyphCache();
    private:
        friend class TextSystem;
        friend class SceneRender;

        std::string m_text{"Hello"};
        unsigned int m_characterSize;
        const robot2D::Font* m_font;
        robot2D::Texture* m_texture;

        bool m_needUpdate;
        std::unordered_map<int, robot2D::GlyphQuad> m_bufferCache;
        friend class TextSystem;

        bool m_scaled = false;
        std::string m_fontPath;
    };

    struct ScriptComponent {
        std::string name;
    };

    struct PrefabComponent {
        UUID prefabUUID;
    };

    struct UIComponent {
        ITreeItem::Ptr treeItem;
        void setName(std::string* text) { treeItem -> setName(text); }
    };

    struct AnimationComponent {
        void setTexture(const robot2D::Texture& texture) {
            m_texture = &texture;
        }

        const robot2D::Texture* getTexture() const {
            return m_animation -> texture;
        }

        void setTextureRect(const robot2D::IntRect& rect) {
            m_textureRect = rect;
            m_hasUpdate = true;
        }

        const robot2D::IntRect& getTextureRect() const {
            return m_textureRect;
        }

        void setAnimation(Animation* animation) { m_animation = animation; }
        Animation* getAnimation() { return m_animation; }
    private:
        friend class AnimationSystem;

        bool m_hasUpdate{false};
        Animation* m_animation;
        robot2D::IntRect m_textureRect;
        const robot2D::Texture* m_texture;
    };

    struct AnimatorComponent {
        DECLARE_COMPONENT_ID()

        void Play(const std::string& animationName);
        void Stop(const std::string& animationName);


    private:
        bool isPlaying{false};
        friend class AnimatorSystem;
        float m_currentFrameTime{0.f};
        std::uint32_t m_frameID{0};
        std::string m_animationName;
    };

}
