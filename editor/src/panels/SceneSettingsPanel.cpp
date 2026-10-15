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

#include <editor/panels/SceneSettingsPanel.hpp>
#include <robot2D/imgui/Api.hpp>
#include <editor/physics/Layers2D.hpp>
#include <imgui/imgui.h>
#include <vector>

namespace editor {
    static void DrawLayerRegistryEditor(phys2d::LayerRegistry& LR)
    {
        ImGui::TextDisabled("Total Layers: %d (Box2D supports up to 16)", phys2d::kMaxLayers);

        // --- Раздел: создание слоёв в пустых ячейках ---
        if (ImGui::CollapsingHeader("Layers")) {
            ImGui::BeginDisabled(); // имена из LayerRegistry сейчас только читаем (rename не реализован в примере)
            if (ImGui::BeginTable("layers_list", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
            {
                ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 30.f);
                ImGui::TableSetupColumn("Name");
                ImGui::TableSetupColumn("Bit");
                ImGui::TableHeadersRow();

                for (int i = 0; i < phys2d::kMaxLayers; ++i) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%d", i);
                    ImGui::TableSetColumnIndex(1);
                    const auto& name = LR.names()[i];
                    ImGui::TextUnformatted(name.empty() ? "<empty>" : name.c_str());
                    ImGui::TableSetColumnIndex(2);
                    if (!name.empty()) ImGui::Text("0x%04X", LR.bit(i));
                    else ImGui::TextDisabled("-");
                }
                ImGui::EndTable();
            }
            ImGui::EndDisabled();

            // Быстрое создание слоя в пустой ячейке
            static int newIndex = 0;
            static char newName[64] = "";
            ImGui::Separator();
            ImGui::Text("CreateNew Layer:");
            ImGui::SetNextItemWidth(80.f);
            ImGui::InputInt("Index", &newIndex);
            if (newIndex < 0)
                newIndex = 0;
            if (newIndex >= phys2d::kMaxLayers)
                newIndex = phys2d::kMaxLayers - 1;
            ImGui::SetNextItemWidth(-60.f);
            ImGui::InputText("Name", newName, IM_ARRAYSIZE(newName));
            bool canDefine = LR.names()[newIndex].empty() && std::string(newName).size() > 0;

            if (!LR.names()[newIndex].empty())
                ImGui::TextDisabled("Index %d already belongs to layer '%s'", newIndex, LR.names()[newIndex].c_str());

            if (ImGui::Button("Define Layer") && canDefine) {
                try {
                    LR.defineLayer(newIndex, std::string(newName));
                    newName[0] = '\0';
                } catch (const std::exception& e) {
                    ImGui::OpenPopup("DefineError");
                    (void)e;
                }
            }
            if (ImGui::BeginPopup("DefineError")) {
                ImGui::TextWrapped("Can't create layer. Check is unique name and index.");
                if (ImGui::Button("OK")) ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
            }
        }

        // --- Раздел: Глобальная матрица столкновений ---
        if (ImGui::CollapsingHeader("Collision matrix", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // Соберём актуальные имена
            std::vector<int> idx;
            std::vector<const char*> names;
            for (int i = 0; i < phys2d::kMaxLayers; ++i) {
                const auto& n = LR.names()[i];
                if (!n.empty()) {
                    idx.push_back(i); names.push_back(n.c_str());
                }
            }

            if (names.empty()) {
                ImGui::TextDisabled("No specific layers.");
                return;
            }

            ImGui::TextWrapped("Symmetric collision defaults for new colliders.");
            ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(6,4));
            if (ImGui::BeginTable("coll_matrix", (int)names.size() + 1,
                                  ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY,
                                  ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * (names.size() + 3))))
            {
                // Заголовок
                ImGui::TableSetupColumn("Layer", ImGuiTableColumnFlags_WidthFixed, 100.f);
                for (size_t c = 0; c < names.size(); ++c)
                    ImGui::TableSetupColumn(names[c], ImGuiTableColumnFlags_WidthFixed, 64.f);
                ImGui::TableHeadersRow();

                for (size_t r = 0; r < names.size(); ++r) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(names[r]);

                    for (size_t c = 0; c < names.size(); ++c) {
                        ImGui::TableSetColumnIndex((int)c + 1);

                        // диагональ — серым
                        if (r == c) {
                            ImGui::PushID(static_cast<int>(r));
                            ImGui::BeginDisabled();
                            bool value = false;
                            ImGui::Checkbox("##self", &value);
                            ImGui::EndDisabled();
                            ImGui::PopID();
                            continue;
                        }

                        bool v = LR.defaultCollides(LR.names()[idx[r]], LR.names()[idx[c]]);
                        ImGui::PushID((int)(r * names.size() + c));
                        if (ImGui::Checkbox("##m", &v)) {
                            // симметрично
                            LR.setDefaultCollides(LR.names()[idx[r]], LR.names()[idx[c]], v);
                        }
                        ImGui::PopID();
                    }
                }
                ImGui::EndTable();
            }
            ImGui::PopStyleVar();
        }
    }


    SceneSettingsPanel::SceneSettingsPanel(): IPanel(UniqueType(typeid(SceneSettingsPanel))) {}

    void SceneSettingsPanel::render() {
        robot2D::WindowOptions options;
        options.flagsMask = 0;
        options.name = "Scene Settings###SceneSettingsPanel";
        renderWindow(options, []() {
            DrawLayerRegistryEditor(phys2d::LayerRegistry::I());
        });
    }
}
