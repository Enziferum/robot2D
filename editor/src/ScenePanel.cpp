/*********************************************************************
(c) Alex Raag 2021
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
#include <imgui/imgui_internal.h>
#include <editor/ScenePanel.hpp>
#include <editor/Components.hpp>
#include <cstring>
#include <filesystem>

namespace editor {
    constexpr int rightMouseButton = 1;

    static void DrawVec2Control(const std::string& label, robot2D::vec2f& values,
                                float resetValue = 0.0f, float columnWidth = 100.0f)
    {
        ImGuiIO& io = ImGui::GetIO();
        auto boldFont = io.Fonts->Fonts[0];

        ImGui::PushID(label.c_str());

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::Text(label.c_str());
        ImGui::NextColumn();

        ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

        float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
        ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("X", buttonSize))
            values.x = resetValue;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushFont(boldFont);
        if (ImGui::Button("Y", buttonSize))
            values.y = resetValue;
        ImGui::PopFont();
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        ImGui::PopStyleVar();
        ImGui::Columns(1);
        ImGui::PopID();
    }


    template<typename T, typename UIFunction>
    static void drawComponent(const std::string& name, robot2D::ecs::Entity& entity, UIFunction uiFunction)
    {
        const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen
                                                 | ImGuiTreeNodeFlags_Framed
                                                 | ImGuiTreeNodeFlags_SpanAvailWidth
                                                 | ImGuiTreeNodeFlags_AllowItemOverlap
                                                 | ImGuiTreeNodeFlags_FramePadding;
        if (entity.hasComponent<T>())
        {
            auto& component = entity.getComponent<T>();
            ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
            float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
            ImGui::Separator();
            bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
            ImGui::PopStyleVar();
            ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
            if (ImGui::Button("-", ImVec2{ lineHeight, lineHeight }))
            {
                ImGui::OpenPopup("ComponentSettings");
            }

            bool removeComponent = false;
            if (ImGui::BeginPopup("ComponentSettings"))
            {
                if (ImGui::MenuItem("Remove component"))
                    removeComponent = true;

                ImGui::EndPopup();
            }

            if (open)
            {
                uiFunction(component);
                ImGui::TreePop();
            }

            if (removeComponent)
                entity.removeComponent<T>();
        }
    }

    ScenePanel::ScenePanel(): IPanel(UniqueType(typeid(ScenePanel))),
    m_scene(nullptr), m_selectedEntity{} {
    }

    void ScenePanel::render() {
        if(m_scene == nullptr)
            return;

        ImGui::Begin("ScenePanel");
        ImGuiIO& io = ImGui::GetIO();
        auto boldFont = io.Fonts->Fonts[0];
        ImGui::PushFont(boldFont);
        ImGui::Text(m_scene -> getPath().c_str());
        ImGui::PopFont();

        if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
            m_selectedEntity = {};

        for(auto& entity: m_scene->getEntities()) {
            drawEntity(entity);
        }

        // Right-click on blank space
        if (ImGui::BeginPopupContextWindow(0, rightMouseButton, false))
        {
            if (ImGui::MenuItem("Create Empty Entity")) {
                m_scene -> addEmptyEntity();
                m_selectedEntity = m_scene->getEntities().back();
            }

            ImGui::EndPopup();
        }

        ImGui::End();

        ImGui::Begin("Properties");
        if (m_selectedEntity)
        {
            drawComponents(m_selectedEntity);
        }

        ImGui::End();
    }

    void ScenePanel::drawEntity(robot2D::ecs::Entity entity) {
        auto& tag = entity.getComponent<TagComponent>().getTag();

        ImGuiTreeNodeFlags flags = ((m_selectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
        flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
        bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity.getIndex(), flags, tag.c_str());
        if (ImGui::IsItemClicked())
        {
            m_selectedEntity = entity;
        }

        bool entityDeleted = false;
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Delete Entity"))
                entityDeleted = true;

            ImGui::EndPopup();
        }

        if (opened)
        {
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
            bool opened = ImGui::TreeNodeEx((void*)9817239, flags, tag.c_str());
            if (opened)
                ImGui::TreePop();
            ImGui::TreePop();
        }

        if (entityDeleted)
        {
            m_scene -> removeEntity(entity);
            if (m_selectedEntity == entity)
                m_selectedEntity = {};
        }
    }

    void ScenePanel::drawComponents(robot2D::ecs::Entity& entity) {
        if(!entity.hasComponent<TagComponent>())
            return;
        auto& tag = entity.getComponent<TagComponent>().getTag();
        char buffer[256];
        memset(buffer, 0, sizeof(buffer));
        std::strncpy(buffer, tag.c_str(), sizeof(buffer));
        if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
        {
            tag = std::string(buffer);
            if(tag.empty())
                tag = "Untitled Entity";
        }

        ImGui::SameLine();
        ImGui::PushItemWidth(-1);

        if (ImGui::Button("Add Component"))
            ImGui::OpenPopup("AddComponent");


        if (ImGui::BeginPopup("AddComponent"))
        {
            if (ImGui::MenuItem("Camera"))
            {
                if (!m_selectedEntity.hasComponent<CameraComponent>())
                    m_selectedEntity.addComponent<CameraComponent>();
                else
                    RB_EDITOR_WARN("This entity already has the Camera Component!");
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::MenuItem("Sprite Renderer"))
            {
                if (!m_selectedEntity.hasComponent<SpriteComponent>())
                    m_selectedEntity.addComponent<SpriteComponent>();
                else
                    RB_EDITOR_WARN("This entity already has the Sprite Renderer Component!");
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        ImGui::PopItemWidth();

        drawComponent<TransformComponent>("Transform", entity, [](auto& component)
        {
            DrawVec2Control("Translation", component.getPosition());
            DrawVec2Control("Scale", component.getScale(), 1.0f);
            robot2D::vec2f rotation = {component.getRotate(), 0.F};
            DrawVec2Control("Rotation", rotation, 0.F);
            component.setRotate(rotation.x);
        });

        drawComponent<SpriteComponent>("Sprite Renderer", entity, [](auto& component)
        {
            // fix
            auto color = component.getColor().toGL();
            float colors[4];
            colors[0] = color.red; colors[1] = color.green;
            colors[2] = color.blue; colors[3] = color.alpha;
            ImGui::ColorEdit4("Color", colors);

            component.setColor(robot2D::Color::fromGL(colors[0], colors[1], colors[2], colors[3]));

            ImGui::Button("Texture", ImVec2(100.0f, 0.0f));
            if (ImGui::BeginDragDropTarget())
            {
                // all string types make as Parametrs some Where

                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                {
                    const wchar_t* path = (const wchar_t*)payload->Data;
                    std::filesystem::path texturePath = std::filesystem::path("assets") / path;


                    RB_EDITOR_INFO("PATH {0}", texturePath.string().c_str());
//                    if(!texture.loadFromFile(texturePath.string()))
//                        return;
//
//
//                    component.setTexture(texture);
                }
                ImGui::EndDragDropTarget();
            }
        });
    }

}