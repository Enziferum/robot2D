#pragma once
#include "imgui/imgui.h"

namespace robot2D::Sugar {
    using ImGuiScoppedEndCallback = void(*)();
    template<typename Always>
    struct ImGuiScopedCall {
        //ImGuiScopedCall(const bool state, ImGuiScope);
        ~ImGuiScopedCall();


    };




}

#define on_Window(BEGIN, END)