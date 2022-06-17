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

namespace editor {
    class TransformComponent: public robot2D::Transformable {
    public:
        TransformComponent(){}
        ~TransformComponent() override {}

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

    class NativeScriptComponent final {
    public:
        NativeScriptComponent(){}
    };
}
