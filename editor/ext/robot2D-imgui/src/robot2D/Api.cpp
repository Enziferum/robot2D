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

    void DrawImage(robot2D::Sprite& sprite, const robot2D::vec2f& size) {
        const robot2D::Texture* sprTexture = sprite.getTexture();
        if(!sprTexture || size == robot2D::vec2f{})
            return;
        const robot2D::Texture& texture = *sprTexture;
        robot2D::vec2f textureSize = {texture.getSize().x, texture.getSize().y};
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
        robot2D::vec2f textureSize = {texture.getSize().x, texture.getSize().y};
        auto textureRect = sprite.getTextureRect();

        auto imID = ImGui::convertTextureHandle(texture.getID());
        ImVec2 uv0(textureRect.lx / textureSize.x, textureRect.ly / textureSize.y);
        ImVec2 uv1((textureRect.lx + textureRect.width) / textureSize.x,
                   (textureRect.ly + textureRect.height) / textureSize.y);
        ImGui::Image(imID, ImVec2(size.x, size.y), uv0, uv1);
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

} // namespace robot2D