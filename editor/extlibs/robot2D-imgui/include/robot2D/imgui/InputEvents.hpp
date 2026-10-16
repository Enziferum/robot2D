#pragma once
#include <imgui/imgui.h>
#include <robot2D/Core/Event.hpp>

namespace robot2D {
    struct GuiInputCapture {
        bool mouse{false};
        bool keyboard{false};
        bool activeItem{false};
        bool popup{false};
    };

    /// Feed every event in reception order, regardless of capture. Main thread only.
    void feedGuiEvent(ImGuiIO& io, const Event& event);
    /// Snapshot for routing before NewFrame; not a per-event consumption result.
    GuiInputCapture captureGuiInput();
}
