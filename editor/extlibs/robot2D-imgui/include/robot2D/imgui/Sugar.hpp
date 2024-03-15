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
#include "imgui/imgui.h"

/// Inspired by imgui_sugar: https://github.com/mnesarco/imgui_sugar

namespace robot2D::Sugar {
    using ImGuiScoppedEndCallback = void(*)();


    template<bool AlwaysCallEnd>
    struct ImGuiScopedCall {
        ImGuiScopedCall(const bool state, const ImGuiScoppedEndCallback end) noexcept: m_state{state}, m_end{end} {}
        ImGuiScopedCall(const ImGuiScopedCall& other) = delete;
        ImGuiScopedCall& operator=(const ImGuiScopedCall& other) = delete;
        ImGuiScopedCall(ImGuiScopedCall&& other) = delete;
        ImGuiScopedCall& operator=(ImGuiScopedCall&& other) = delete;
        ~ImGuiScopedCall() noexcept;

        operator bool() const noexcept { return m_state; }

    private:
        const bool m_state;
        const ImGuiScoppedEndCallback m_end;
    };

    template<>
    inline ImGuiScopedCall<true>::~ImGuiScopedCall() noexcept {
        m_end();
    }

    template<>
    inline ImGuiScopedCall<false>::~ImGuiScopedCall() noexcept {
        if(m_state)
            m_end();
    }

    inline void PopStyleColor() { ImGui::PopStyleColor(1); };
    inline void PopStyleVar()   { ImGui::PopStyleVar(1); };

    inline auto BeginToolTip() -> bool {
        if(ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            return true;
        }
        return false;
    }
} // namespace robot2D::Sugar

// Portable Expression Statement, calls void function and returns true
#define IMGUI_SUGAR_ES(FN, ...) ([&]() -> bool { FN(__VA_ARGS__); return true; }())
#define IMGUI_SUGAR_ES_0(FN) ([&]() -> bool { FN(); return true; }())

// Concatenating symbols with __LINE__ requires two levels of indirection
#define IMGUI_SUGAR_CONCAT0(A, B) A ## B
#define IMGUI_SUGAR_CONCAT1(A, B) IMGUI_SUGAR_CONCAT0(A, B)

#define IMGUI_SUGAR_SCOPED_BOOL(BEGIN, END, ALWAYS, ...) \
    if (const robot2D::Sugar::ImGuiScopedCall<ALWAYS> _ui_scope_guard = {BEGIN(__VA_ARGS__), &END})

#define IMGUI_SUGAR_SCOPED_BOOL_0(BEGIN, END, ALWAYS) \
    if (const robot2D::Sugar::ImGuiScopedCall<ALWAYS> _ui_scope_guard = {BEGIN(), &END})

#define IMGUI_SUGAR_SCOPED_BOOL_PARENT_0(BEGIN, END, ALWAYS) \
    if (const robot2D::Sugar::ImGuiScopedCall<ALWAYS> _ui_scope_guard_parent = {BEGIN(), &END})

#define IMGUI_SUGAR_SCOPED_VOID_N(BEGIN, END, ...) \
    if (const robot2D::Sugar::ImGuiScopedCall<true> _ui_scope_guard = {IMGUI_SUGAR_ES(BEGIN, __VA_ARGS__), &END})

#define IMGUI_SUGAR_SCOPED_VOID_0(BEGIN, END) \
    if (const robot2D::Sugar::ImGuiScopedCall<true> _ui_scope_guard = {IMGUI_SUGAR_ES_0(BEGIN), &END})

#define IMGUI_SUGAR_PARENT_SCOPED_VOID_N(BEGIN, END, ...) \
    const robot2D::Sugar::ImGuiScopedCall<true> IMGUI_SUGAR_CONCAT1(_ui_scope_, __LINE__) = {IMGUI_SUGAR_ES(BEGIN, __VA_ARGS__), &END}

#define imgui_Window(...)             IMGUI_SUGAR_SCOPED_BOOL(ImGui::Begin,                   ImGui::End,               true,  __VA_ARGS__)
#define imgui_Child(...)              IMGUI_SUGAR_SCOPED_BOOL(ImGui::BeginChild,              ImGui::EndChild,          true,  __VA_ARGS__)
#define imgui_ChildFrame(...)         IMGUI_SUGAR_SCOPED_BOOL(ImGui::BeginChildFrame,         ImGui::EndChildFrame,     true,  __VA_ARGS__)
#define imgui_Combo(...)              IMGUI_SUGAR_SCOPED_BOOL(ImGui::BeginCombo,              ImGui::EndCombo,          false, __VA_ARGS__)
#define imgui_ListBox(...)            IMGUI_SUGAR_SCOPED_BOOL(ImGui::BeginListBox,            ImGui::EndListBox,        false, __VA_ARGS__)
#define imgui_Menu(...)               IMGUI_SUGAR_SCOPED_BOOL(ImGui::BeginMenu,               ImGui::EndMenu,           false, __VA_ARGS__)
#define imgui_Popup(...)              IMGUI_SUGAR_SCOPED_BOOL(ImGui::BeginPopup,              ImGui::EndPopup,          false, __VA_ARGS__)
#define imgui_PopupModal(...)         IMGUI_SUGAR_SCOPED_BOOL(ImGui::BeginPopupModal,         ImGui::EndPopup,          false, __VA_ARGS__)
#define imgui_PopupContextItem(...)   IMGUI_SUGAR_SCOPED_BOOL(ImGui::BeginPopupContextItem,   ImGui::EndPopup,          false, __VA_ARGS__)
#define imgui_PopupContextWindow(...) IMGUI_SUGAR_SCOPED_BOOL(ImGui::BeginPopupContextWindow, ImGui::EndPopup,          false, __VA_ARGS__)
#define imgui_PopupContextVoid(...)   IMGUI_SUGAR_SCOPED_BOOL(ImGui::BeginPopupContextVoid,   ImGui::EndPopup,          false, __VA_ARGS__)
#define imgui_Table(...)              IMGUI_SUGAR_SCOPED_BOOL(ImGui::BeginTable,              ImGui::EndTable,          false, __VA_ARGS__)
#define imgui_TabBar(...)             IMGUI_SUGAR_SCOPED_BOOL(ImGui::BeginTabBar,             ImGui::EndTabBar,         false, __VA_ARGS__)
#define imgui_TabItem(...)            IMGUI_SUGAR_SCOPED_BOOL(ImGui::BeginTabItem,            ImGui::EndTabItem,        false, __VA_ARGS__)
#define imgui_DragDropSource(...)     IMGUI_SUGAR_SCOPED_BOOL(ImGui::BeginDragDropSource,     ImGui::EndDragDropSource, false, __VA_ARGS__)
#define imgui_TreeNode(...)           IMGUI_SUGAR_SCOPED_BOOL(ImGui::TreeNode,                ImGui::TreePop,           false, __VA_ARGS__)
#define imgui_TreeNodeV(...)          IMGUI_SUGAR_SCOPED_BOOL(ImGui::TreeNodeV,               ImGui::TreePop,           false, __VA_ARGS__)
#define imgui_TreeNodeEx(...)         IMGUI_SUGAR_SCOPED_BOOL(ImGui::TreeNodeEx,              ImGui::TreePop,           false, __VA_ARGS__)
#define imgui_TreeNodeExV(...)        IMGUI_SUGAR_SCOPED_BOOL(ImGui::TreeNodeExV,             ImGui::TreePop,           false, __VA_ARGS__)

#define imgui_TooltipOnHover          IMGUI_SUGAR_SCOPED_BOOL_0(ImGuiSugar::BeginTooltip,     ImGui::EndTooltip,        false)
#define imgui_DragDropTarget          IMGUI_SUGAR_SCOPED_BOOL_0(ImGui::BeginDragDropTarget,   ImGui::EndDragDropTarget, false)
#define imgui_MainMenuBar             IMGUI_SUGAR_SCOPED_BOOL_PARENT_0(ImGui::BeginMainMenuBar,      ImGui::EndMainMenuBar,    false)
#define imgui_MenuBar                 IMGUI_SUGAR_SCOPED_BOOL_PARENT_0(ImGui::BeginMenuBar,          ImGui::EndMenuBar,        false)

#define imgui_Group                   IMGUI_SUGAR_SCOPED_VOID_0(ImGui::BeginGroup,            ImGui::EndGroup)
#define imgui_Tooltip                 IMGUI_SUGAR_SCOPED_VOID_0(ImGui::BeginTooltip,          ImGui::EndTooltip)

#define imgui_Font(...)               IMGUI_SUGAR_SCOPED_VOID_N(ImGui::PushFont,               ImGui::PopFont,               __VA_ARGS__)
#define imgui_AllowKeyboardFocus(...) IMGUI_SUGAR_SCOPED_VOID_N(ImGui::PushAllowKeyboardFocus, ImGui::PopAllowKeyboardFocus, __VA_ARGS__)
#define imgui_ButtonRepeat(...)       IMGUI_SUGAR_SCOPED_VOID_N(ImGui::PushButtonRepeat,       ImGui::PopButtonRepeat,       __VA_ARGS__)
#define imgui_ItemWidth(...)          IMGUI_SUGAR_SCOPED_VOID_N(ImGui::PushItemWidth,          ImGui::PopItemWidth,          __VA_ARGS__)
#define imgui_TextWrapPos(...)        IMGUI_SUGAR_SCOPED_VOID_N(ImGui::PushTextWrapPos,        ImGui::PopTextWrapPos,        __VA_ARGS__)
#define imgui_ID(...)                 IMGUI_SUGAR_SCOPED_VOID_N(ImGui::PushID,                 ImGui::PopID,                 __VA_ARGS__)
#define imgui_ClipRect(...)           IMGUI_SUGAR_SCOPED_VOID_N(ImGui::PushClipRect,           ImGui::PopClipRect,           __VA_ARGS__)
#define imgui_TextureID(...)          IMGUI_SUGAR_SCOPED_VOID_N(ImGui::PushTextureID,          ImGui::PopTextureID,          __VA_ARGS__)

// Non self scoped guards (managed by parent scope)

#define imgui_set_Font(...)                IMGUI_SUGAR_PARENT_SCOPED_VOID_N(ImGui::PushFont,               ImGui::PopFont,               __VA_ARGS__)
#define imgui_set_AllowKeyboardFocus(...)  IMGUI_SUGAR_PARENT_SCOPED_VOID_N(ImGui::PushAllowKeyboardFocus, ImGui::PopAllowKeyboardFocus, __VA_ARGS__)
#define imgui_set_ButtonRepeat(...)        IMGUI_SUGAR_PARENT_SCOPED_VOID_N(ImGui::PushButtonRepeat,       ImGui::PopButtonRepeat,       __VA_ARGS__)
#define imgui_set_ItemWidth(...)           IMGUI_SUGAR_PARENT_SCOPED_VOID_N(ImGui::PushItemWidth,          ImGui::PopItemWidth,          __VA_ARGS__)
#define imgui_set_TextWrapPos(...)         IMGUI_SUGAR_PARENT_SCOPED_VOID_N(ImGui::PushTextWrapPos,        ImGui::PopTextWrapPos,        __VA_ARGS__)
#define imgui_set_ID(...)                  IMGUI_SUGAR_PARENT_SCOPED_VOID_N(ImGui::PushID,                 ImGui::PopID,                 __VA_ARGS__)
#define imgui_set_ClipRect(...)            IMGUI_SUGAR_PARENT_SCOPED_VOID_N(ImGui::PushClipRect,           ImGui::PopClipRect,           __VA_ARGS__)
#define imgui_set_TextureID(...)           IMGUI_SUGAR_PARENT_SCOPED_VOID_N(ImGui::PushTextureID,          ImGui::PopTextureID,          __VA_ARGS__)

// Special case (overloaded functions StyleColor and StyleVar)

#define imgui_set_StyleColor(...)          IMGUI_SUGAR_PARENT_SCOPED_VOID_N(ImGui::PushStyleColor,  ImGuiSugar::PopStyleColor,           __VA_ARGS__)
#define imgui_set_StyleVar(...)            IMGUI_SUGAR_PARENT_SCOPED_VOID_N(ImGui::PushStyleVar,    ImGuiSugar::PopStyleVar,             __VA_ARGS__)

#define imgui_StyleColor(...)         IMGUI_SUGAR_SCOPED_VOID_N(ImGui::PushStyleColor,         ImGuiSugar::PopStyleColor,           __VA_ARGS__)
#define imgui_StyleVar(...)           IMGUI_SUGAR_SCOPED_VOID_N(ImGui::PushStyleVar,           ImGuiSugar::PopStyleVar,             __VA_ARGS__)

// Non RAII

#define imgui_CollapsingHeader(...) if (ImGui::CollapsingHeader(__VA_ARGS__))
#define imgui_MenuItem(...) if (ImGui::MenuItem(__VA_ARGS__))
#define imgui_Button(...) if (ImGui::Button(__VA_ARGS__))
#define imgui_InputText(...) if(robot2D::InputText(__VA_ARGS__))
#define imgui_Text(text) ImGui::Text("%s", text)