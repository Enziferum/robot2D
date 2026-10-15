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

#include <imgui/imgui_internal.h>
#include <robot2D/imgui/Api.hpp>
#include <editor/panels/ScenePanelUI.hpp>

namespace editor::ui {

    namespace {
        void drawVectorControl(const std::string& label, float* values, int count,
                               float resetValue, float columnWidth, float min, float max) {
            static const char* axes[] = {"X", "Y", "Z"};
            static const ImVec4 colors[] = {{0.45f, 0.20f, 0.24f, 1.f},
                                           {0.20f, 0.38f, 0.28f, 1.f},
                                           {0.20f, 0.30f, 0.50f, 1.f}};
            ImGui::PushID(label.c_str());
            auto available = ImGui::GetContentRegionAvail().x;
            if(ImGui::BeginTable("##Vector", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoSavedSettings)) {
                ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, ImMin(columnWidth, available * 0.34f));
                ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted(label.c_str());
                ImGui::TableSetColumnIndex(1);
                auto fieldArea = ImGui::GetContentRegionAvail().x;
                bool stacked = fieldArea < count * 78.f;
                auto gap = 4.f;
                auto fieldWidth = stacked ? fieldArea : (fieldArea - gap * (count - 1)) / count;
                auto buttonSize = ImGui::GetFrameHeight();
                for(int axis = 0; axis < count; ++axis) {
                    if(axis && !stacked)
                        ImGui::SameLine(0.f, gap);
                    ImGui::PushID(axis);
                    auto color = colors[axis];
                    ImGui::PushStyleColor(ImGuiCol_Button, color);
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {color.x + 0.1f, color.y + 0.1f, color.z + 0.1f, 1.f});
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
                    if(ImGui::Button(axes[axis], {buttonSize, buttonSize}))
                        values[axis] = resetValue;
                    ImGui::PopStyleColor(3);
                    ImGui::SameLine(0.f, 2.f);
                    ImGui::SetNextItemWidth(ImMax(1.f, fieldWidth - buttonSize - 2.f));
                    ImGui::DragFloat("##Value", &values[axis], 0.1f, min, max, "%.2f");
                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
            ImGui::PopID();
        }
    }

    void drawVec1Control(const std::string& label, float& value, float resetValue, float columnWidth) {
        drawVectorControl(label, &value, 1, resetValue, columnWidth, 0.f, 0.f);
    }

    void drawVec2Control(const std::string& label, robot2D::vec2f& values,
                         float resetValue, float columnWidth, float min, float max) {
        float data[] = {values.x, values.y};
        drawVectorControl(label, data, 2, resetValue, columnWidth, min, max);
        values = {data[0], data[1]};
    }

    void drawVec3Control(const std::string& label, robot2D::vec3f& values,
                         float resetValue, float columnWidth) {
        float data[] = {values.x, values.y, values.z};
        drawVectorControl(label, data, 3, resetValue, columnWidth, 0.f, 0.f);
        values = {data[0], data[1], data[2]};
    }
}
