/*********************************************************************
(c) Alex Raag 2024
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

#include <imgui/imgui.h>

#include <robot2D/imgui/Api.hpp>
#include <robot2D/imgui/Util.hpp>

namespace robot2D {

    void createWindow(const WindowOptions& windowOptions, std::function<void()>&& callback) {
        windowOptions.enable();
        ImGui::Begin(windowOptions.name.c_str(),
              nullptr,
              windowOptions.flagsMask);
        Defer defer{&ImGui::End};
        callback();
        // TODO(a.raag): windowOptions inside Defer
        windowOptions.disable();
    }


    void RenderFrameBuffer(const robot2D::FrameBuffer::Ptr& frameBuffer, const robot2D::vec2f& size) {
        auto imID = ImGui::convertTextureHandle(frameBuffer -> getFrameBufferRenderID());
        ImGui::Image(imID, ImVec2(size.x, size.y), {0,1}, {1, 0});
    }

    bool ImageButton(const robot2D::Texture& texture, const robot2D::vec2f& size) {
        auto imID = ImGui::convertTextureHandle(texture.getID());
        return ImGui::ImageButton(imID, ImVec2(size.x, size.y), {0, 0},
                                  {1,1}, 0);
    }

    struct InputTextCallback_UserData
    {
        std::string*            Str;
        ImGuiInputTextCallback  ChainCallback;
        void*                   ChainCallbackUserData;
    };

    static int InputTextCallback(ImGuiInputTextCallbackData* data)
    {
        auto* user_data = static_cast<InputTextCallback_UserData*>(data -> UserData);
        assert(user_data != nullptr && "InputTextCallback user data can't be nullptr");
        if (data -> EventFlag == ImGuiInputTextFlags_CallbackResize)
        {
            // Resize string callback
            // If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we need to set them back to what we want.
            std::string* str = user_data -> Str;
            IM_ASSERT(data -> Buf == str -> c_str());
            str -> resize(data -> BufTextLen);
            data -> Buf = const_cast<char*>(str -> c_str());
        }
        else if (user_data->ChainCallback)
        {
            // Forward to user callback, if any
            data -> UserData = user_data -> ChainCallbackUserData;
            return user_data->ChainCallback(data);
        }
        return 0;
    }

    bool InputText(const std::string& label, std::string* str, ImGuiInputTextFlags flags,
                   ImGuiInputTextCallback callback, void* user_data)
    {
        IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
        flags |= ImGuiInputTextFlags_CallbackResize;

        InputTextCallback_UserData cb_user_data{};
        cb_user_data.Str = str;
        cb_user_data.ChainCallback = callback;
        cb_user_data.ChainCallbackUserData = user_data;
        return ImGui::InputText(
                label.c_str(),
                const_cast<char*>(str->c_str()),
                str -> capacity() + 1,
                flags,
                InputTextCallback,
                &cb_user_data
        );
    }


    void DrawImage(robot2D::Sprite& sprite, const robot2D::vec2f& size) {
        const robot2D::Texture* sprTexture = sprite.getTexture();
        if(!sprTexture || size == robot2D::vec2f{})
            return;
        const robot2D::Texture& texture = *sprTexture;
        auto texSize = texture.getSize();
        robot2D::vec2f textureSize = texSize.as<float>();
        auto textureRect = sprite.getLocalBounds();

        auto imID = ImGui::convertTextureHandle(texture.getID());
        ImVec2 uv0(textureRect.lx / textureSize.x, textureRect.ly / textureSize.y);
        ImVec2 uv1((textureRect.lx + textureRect.width) / textureSize.x,
                   (textureRect.ly + textureRect.height) / textureSize.y);
        ImGui::Image(imID, ImVec2(size.x, size.y), uv0, uv1);
    }

    void DrawAnimatedImage(robot2D::Sprite& sprite, const robot2D::vec2f& size) {
        const robot2D::Texture* sprTexture = sprite.getTexture();
        if(!sprTexture || size == robot2D::vec2f{})
            return;
        const robot2D::Texture& texture = *sprTexture;
        robot2D::vec2f textureSize;// = { texture.getSize().x, texture.getSize().y };
        auto textureRect = sprite.getTextureRect();

        auto imID = ImGui::convertTextureHandle(texture.getID());
        ImVec2 uv0(textureRect.lx / textureSize.x, textureRect.ly / textureSize.y);
        ImVec2 uv1((textureRect.lx + textureRect.width) / textureSize.x,
                   (textureRect.ly + textureRect.height) / textureSize.y);
        ImGui::Image(imID, ImVec2(size.x, size.y), uv0, uv1);
    }


} // namespace robot2D