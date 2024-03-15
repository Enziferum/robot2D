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

#pragma once

#include <functional>
#include <string>

#include <imgui/imgui.h>
#include <robot2D/Graphics/Sprite.hpp>
#include <robot2D/Graphics/FrameBuffer.hpp>

#include "WindowOptions.hpp"
#include "ScopedStyleColor.hpp"
#include "Spinner.hpp"
#include "Sugar.hpp"

namespace robot2D {

    void createWindow(const WindowOptions& windowOptions, std::function<void()>&& callback);

    bool ImageButton(const robot2D::Texture& texture, const robot2D::vec2f& size);

    void RenderFrameBuffer(const robot2D::FrameBuffer::Ptr& frameBuffer, const robot2D::vec2f& size);

    bool InputText(
            const std::string& label,
            std::string* str,
            ImGuiInputTextFlags flags,
            ImGuiInputTextCallback callback = nullptr,
            void* user_data = nullptr
    );

    /////////////////////////////////////////////////////////// Deprecated / Unused ///////////////////////////////////////////////////////////

    [[deprecated("Unstable method need rework")]]
    void DrawImage(robot2D::Sprite& sprite, const robot2D::vec2f& size);

    [[deprecated("Unstable method need rework")]]
    void DrawAnimatedImage(robot2D::Sprite& sprite, const robot2D::vec2f& size);

    template<typename F,
            typename = std::enable_if_t<std::is_invocable_v<F>>>
    struct Defer {
        Defer(F&& func):
                m_func{std::move(func)}{}

        ~Defer() {
            try{
                if(m_func)
                    m_func();
            }
            catch (...) {}
        }

        F m_func;
    };


    struct ColorButton {
        enum class Type {
            Default, Hovered, Active
        } type;

        robot2D::Color color;
    };


    template<typename T>
    [[deprecated("Unstable method need rework")]]
    void SimpleDragDrop(const std::string& payloadID, std::function<void(T*)>&& callback) {
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(payloadID.c_str())) {
                T* data = (T*)(payload -> Data);
                if(data)
                    callback(data);
            }
            ImGui::EndDragDropTarget();
        }
    }

    template<
            typename Func,
            typename ...Args,
            typename = std::enable_if_t<std::is_invocable_v<Func, Args...>>
    >
    [[deprecated("Unstable method need rework")]]
    void colorButton(std::string title,
                     std::initializer_list<ColorButton> colors,
                     Func&& func, Args&& ... args) {
        ImGui::PushID(title.c_str());
        for(const auto& color: colors) {
            switch(color.type) {
                case ColorButton::Type::Default:
                    ImGui::PushStyleColor(
                            ImGuiCol_Button,
                            (ImVec4)ImColor{
                                    color.color.red,
                                    color.color.green,
                                    color.color.blue,
                                    color.color.alpha
                            }
                    );
                    break;
                case ColorButton::Type::Hovered:
                    ImGui::PushStyleColor(
                            ImGuiCol_ButtonHovered,
                            (ImVec4)ImColor{
                                    color.color.red,
                                    color.color.green,
                                    color.color.blue,
                                    color.color.alpha
                            }
                    );
                    break;
                case ColorButton::Type::Active:
                    ImGui::PushStyleColor(
                            ImGuiCol_ButtonActive,
                            (ImVec4)ImColor{
                                    color.color.red,
                                    color.color.green,
                                    color.color.blue,
                                    color.color.alpha
                            }
                    );
                    break;
            }
        }
        if(ImGui::Button(title.c_str()))
            func(std::forward<Args>(args)...);
        ImGui::PopStyleColor(static_cast<int>(colors.size()));
        ImGui::PopID();
    }

/////////////////////////////////////////////////////////// Deprecated / Unused ///////////////////////////////////////////////////////////

} // namespace robot2D