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

namespace editor {
    class TransformComponent: public robot2D::Transformable {
    public:
        ~TransformComponent() override = default;
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


    class SpriteComponent final {
    public:
        SpriteComponent();
        ~SpriteComponent() = default;

        void setTexture(const robot2D::Texture& texture);
        robot2D::Texture& getTexture();
        const robot2D::Texture& getTexture() const;
        const robot2D::Texture* getTexturePointer() const {return m_texture;}

        void setColor(const robot2D::Color& color);
        const robot2D::Color& getColor() const;
    private:
        const robot2D::Texture* m_texture;
        robot2D::Color m_color;
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

        /// Todo Add 3D Space Camera ///
    private:
        robot2D::View m_view;
        robot2D::FloatRect m_actualViewport;
    };


    class Collider2DComponent final {
    public:
        Collider2DComponent(){}
    };

    class Physics2DComponent final {
    public:
        Physics2DComponent(){}
    };

    struct ScriptComponent {
        std::string name;
    };

}
