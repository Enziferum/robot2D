#pragma once

#include <imgui/imgui.h>
#include <robot2D/Core/MessageBus.hpp>
#include <editor/Messages.hpp>
#include <editor/UIInteractor.hpp>
#include "../IconsFontsAwesome5.hpp"

namespace editor {

    inline void renderRuntimeControls(UIInteractor* interactor, robot2D::MessageBus& messageBus, float width = 0.f) {
        const bool running = interactor && interactor -> isRunning();
        ImGui::BeginDisabled(interactor == nullptr);
        if(running)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.45f, 0.16f, 0.19f, 1.f});
        if(ImGui::Button(running ? ICON_FA_STOP " Stop" : ICON_FA_PLAY " Play", {width, 0.f})) {
            auto* message = messageBus.postMessage<ToolbarMessage>(MessageID::ToolbarPressed);
            message -> type = running ? ToolbarMessage::Type::Stop : ToolbarMessage::Type::Start;
        }
        if(running)
            ImGui::PopStyleColor();
        if(ImGui::IsItemHovered())
            ImGui::SetTooltip(running ? "Stop scene and return to editing" : "Run scene");
        ImGui::EndDisabled();
    }

}
