#include <imgui/imgui.h>
#include <editor/Gui/Api.hpp>
#include <editor/Gui/Util.hpp>

namespace ImGui {

    void Image(robot2D::Sprite& sprite, const robot2D::vec2f &size) {
        const robot2D::Texture* sprTexture = sprite.getTexture();
        if(!sprTexture || size == robot2D::vec2f{})
            return;
        const robot2D::Texture& texture = *sprTexture;
        robot2D::vec2f textureSize = {texture.getSize().x, texture.getSize().y};
        auto textureRect = sprite.getLocalBounds();

        auto imID = convertTextureHandle(texture.getID());
        ImVec2 uv0(textureRect.lx / textureSize.x, textureRect.ly / textureSize.y);
        ImVec2 uv1((textureRect.lx + textureRect.width) / textureSize.x,
                   (textureRect.ly + textureRect.height) / textureSize.y);
        ImGui::Image(imID, ImVec2(size.x, size.y), uv0, uv1);
    }

    void RenderFrameBuffer(const robot2D::FrameBuffer::Ptr& frameBuffer, const robot2D::vec2f& size) {
        auto imID = convertTextureHandle(frameBuffer -> getFrameBufferRenderID());
        ImGui::Image(imID, ImVec2(size.x, size.y), {0,1}, {1, 0});
    }

}