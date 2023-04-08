#pragma once

#include <robot2D/Graphics/Vertex.hpp>
#include <robot2D/Graphics/Shader.hpp>
#include <robot2D/Graphics/View.hpp>
#include <robot2D/Graphics/Drawable.hpp>

#include "Font.hpp"
#include "QuadBatchRender.hpp"

namespace robot2D {

    class Text: public robot2D::Drawable,
                public robot2D::Transformable {
    public:
        Text();
        ~Text();

        void setFont(const Font& font);
        void setText(const std::string& text);
        const std::string& getText() const;

        void setPosition(const robot2D::vec2f& position);
        void setPosition(float posX, float posY);
        const robot2D::vec2f& getPosition() const {
            return m_pos;
        }

        void setScale(const float& scale);
        const float& getScale() const;

        void setColor(const robot2D::Color& color);
        const robot2D::Color& getColor() const;

        void draw(RenderTarget& target, RenderStates states) const override;

        void move(const vec2f& offset);
        void scale(const vec2f& factor);
        void rotate(float angle);
    private:
        void setupGL();

        void beforeRender() const;
        void updateGeometry() const;
        void afterRender(bool hardUpdate) const;
    private:
        const Font* m_font;
        std::string m_text;

        ShaderHandler m_textShader;

        mutable bool m_initialized = false;
        mutable bool m_needupdate = false;
        mutable bool m_scaled = false;
        mutable robot2D::Color m_color;

        mutable std::unordered_map<int, GlyphQuad> m_bufferCache;
        mutable QuadBatchRender<Vertex> quadBatchRender;
    };

}

