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

#include <robot2D/imgui/Api.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <editor/panels/InspectorPanel.hpp>
#include <editor/scripting/ScriptingEngine.hpp>
#include <editor/panels/ScenePanelUI.hpp>
#include <editor/FileApi.hpp>
#include <editor/TaskQueue.hpp>
#include <editor/ResouceManager.hpp>
#include <editor/LocalResourceManager.hpp>
#include <editor/DragDropIDS.hpp>
#include <editor/physics/Layers2D.hpp>

#include <editor/components/ButtonComponent.hpp>
#include <editor/async/ImageLoadTask.hpp>
#include <editor/async/FontLoadTask.hpp>
#include <editor/AnimationManager.hpp>

#include "../IconsFontsAwesome5.hpp"

#include <rbini/Utils.hpp>

namespace editor {



    void DrawLayerRegistryEditor(phys2d::LayerRegistry& LR)
    {
        ImGui::TextDisabled("Total Layers: %d (Box2D supports up to 16)", phys2d::kMaxLayers);

        // --- Раздел: создание слоёв в пустых ячейках ---
        if (ImGui::CollapsingHeader("Layer (registry)")) {
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
            ImGui::SameLine();
            ImGui::SetNextItemWidth(200.f);
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
        if (ImGui::CollapsingHeader("Global collision matrix(default)", ImGuiTreeNodeFlags_DefaultOpen))
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

            ImGui::TextDisabled("Checkbox A<->B symmetric. This settings forms default mask for new colliders.");
            ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(6,4));
            if (ImGui::BeginTable("coll_matrix", (int)names.size() + 1,
                                  ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY,
                                  ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * (names.size() + 3))))
            {
                // Заголовок
                ImGui::TableSetupColumn("Layer");
                for (size_t c = 0; c < names.size(); ++c)
                    ImGui::TableSetupColumn(names[c]);
                ImGui::TableHeadersRow();

                for (size_t r = 0; r < names.size(); ++r) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(names[r]);

                    for (size_t c = 0; c < names.size(); ++c) {
                        ImGui::TableSetColumnIndex((int)c + 1);

                        // диагональ — серым
                        if (r == c) {
                            ImGui::BeginDisabled();
                            bool value = false;
                            ImGui::Checkbox("##self", &value);
                            ImGui::EndDisabled();
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


    template<typename T, typename UIFunction>
    static void drawComponent(const std::string& name, SceneEntity& entity, UIFunction uiFunction)
    {
        if(!entity || !entity.hasComponent<T>())
            return;

        static const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen
                                                 | ImGuiTreeNodeFlags_Framed
                                                 | ImGuiTreeNodeFlags_SpanAvailWidth
                                                 | ImGuiTreeNodeFlags_AllowItemOverlap
                                                 | ImGuiTreeNodeFlags_FramePadding;

        auto& component = entity.getComponent<T>();
        ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
        bool removeComponent = false;
        bool node_open = false;
        float lineHeight = 0.f;

        {
            robot2D::ScopedStyleVarVec2 styleVar{ ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 }};
            lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
            ImGui::Separator();

            auto hashCode = typeid(T).hash_code();
            node_open = ImGui::TreeNodeEx(reinterpret_cast<void*>(hashCode), treeNodeFlags, "%s", name.c_str());
            ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
        }

        imgui_Button("-", ImVec2{ lineHeight, lineHeight }) {
            ImGui::OpenPopup("ComponentSettings");
        }

        imgui_Popup("ComponentSettings") {
            if (ImGui::MenuItem("Remove component"))
                removeComponent = true;
        }

        if (node_open)
        {
            uiFunction(entity, component);
            ImGui::TreePop();
        }

        if (removeComponent)
            entity.removeComponent<T>();
    }


    void drawScriptFieldInfo(const std::string& text, std::string resultText = "None") {
        ImGui::AlignTextToFramePadding();
        imgui_Text(text.c_str());
        ImGui::SameLine();
        ImGui::Button(resultText.c_str());
    }


    InspectorPanel::InspectorPanel(MessageDispatcher& messageDispatcher,
                                   robot2D::MessageBus& messageBus,
                                   PrefabManager& prefabManager,
                                   IUIManager& uiManager):
        IPanel(typeid(InspectorPanel)),
    m_messageDispatcher{messageDispatcher},
    m_messageBus{messageBus},
    m_prefabManager{prefabManager},
    m_uiManager{uiManager}
    {
        m_messageDispatcher.onMessage<PrefabAssetPressedMessage>(
                MessageID::PrefabAssetPressed,
                BIND_CLASS_FN(onPrefabAssetSelected)
        );

        m_messageDispatcher.onMessage<PanelEntitySelectedMessage>(
                MessageID::PanelEntityNeedSelect,
                BIND_CLASS_FN(onPanelEntityNeedSelect)
        );

        m_messageDispatcher.onMessage<PanelEntitySelectedMessage>(
                MessageID::PanelEntitySelected,
                BIND_CLASS_FN(onPanelEntitySelected)
        );
    }


    void InspectorPanel::setInteractor(UIInteractor::Ptr interactor) {
        m_interactor = interactor;
    }

    void InspectorPanel::setSelected(SceneEntity entity) {
        m_selectedEntity = entity;
    }


    void InspectorPanel::clearSelection() {
        if (m_inspectType == InspectType::AssetPrefab && m_prefabHasModification) {
            if (m_selectedEntity.hasComponent<PrefabComponent>()) {
                auto& prefabComponent = m_selectedEntity.getComponent<PrefabComponent>();
                m_prefabManager.savePrefab(m_interactor, prefabComponent.prefabUUID);
                m_prefabHasModification = false;
                auto* msg =
                    m_messageBus.postMessage<PrefabAssetModificatedMessage>(MessageID::PrefabAssetModificated);
                msg -> prefabUUID = prefabComponent.prefabUUID;
                msg -> prefabEntity = m_selectedEntity;
            }
        }
        m_selectedEntity = {};
    }


    void InspectorPanel::render() {
        robot2D::WindowOptions propertiesWindowOptions{};
        propertiesWindowOptions.name = ICON_FA_INFO_CIRCLE + std::string{" Inspector"} + "###InspectorPanel";

        robot2D::createWindow(propertiesWindowOptions, [this]{
            if(m_selectedEntity && m_inspectType == InspectType::EditorEntity)
                drawComponentsBase(m_selectedEntity);
            else if(m_inspectType != InspectType::EditorEntity)
                drawAssetBase();
        });
    }


    void InspectorPanel::drawAssetBase() {
        switch (m_inspectType) {
            default:
                break;
            case InspectType::AssetPrefab: {
                drawComponentsBase(m_selectedEntity);
                break;
            }
            case InspectType::AssetScene:
            case InspectType::AssetImage:
            case InspectType::AssetFont:
                break;
        }
    }

    void InspectorPanel::drawComponentsBase(SceneEntity entity) {
        DrawLayerRegistryEditor(phys2d::LayerRegistry::I());

        if(m_inspectType == InspectType::EditorEntity) {
            if(!entity.hasComponent<TagComponent>())
                return;

            auto tag = entity.getComponent<TagComponent>().getTag();
            static ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_EnterReturnsTrue
                                                    | ImGuiInputTextFlags_AutoSelectAll;

            imgui_InputText("##Tag", &tag, inputFlags) {
                if(tag.empty())
                    tag = "Untitled Entity";
                entity.getComponent<TagComponent>().setTag(tag);
            }

            ImGui::Text("UUID: %llu", entity.getComponent<IDComponent>().ID);
        }
        else {
            if(!entity.hasComponent<PrefabComponent>())
                return;
            ImGui::Text("Prefab UUID: %llu", entity.getComponent<PrefabComponent>().prefabUUID);
        }

        ImGui::SameLine();
        ImGui::PushItemWidth(-1);

        imgui_Button("Add Component")
            ImGui::OpenPopup("AddComponent");

        imgui_Popup("AddComponent") {
            imgui_MenuItem("Camera") {
                if (!m_selectedEntity.hasComponent<CameraComponent>())
                    m_selectedEntity.addComponent<CameraComponent>();
                else
                    RB_EDITOR_WARN("This entity already has the Camera Component!");
                ImGui::CloseCurrentPopup();
            }
            imgui_MenuItem("UtilRender") {
                if(!m_selectedEntity.hasComponent<DrawableComponent>())
                    m_selectedEntity.addComponent<DrawableComponent>().isUtil = true;
                else
                    RB_EDITOR_WARN("This entity already has the Sprite Renderer Component!");
                ImGui::CloseCurrentPopup();
            }

            imgui_MenuItem("SpriteRender") {
                if (!m_selectedEntity.hasComponent<DrawableComponent>())
                    m_selectedEntity.addComponent<DrawableComponent>();
                else {
                    RB_EDITOR_WARN("This entity already has the Sprite Renderer Component!");
                }

                ImGui::CloseCurrentPopup();
            }

            imgui_MenuItem("RigidBody2D") {
                if (!m_selectedEntity.hasComponent<Physics2DComponent>())
                    m_selectedEntity.addComponent<Physics2DComponent>();
                else
                    RB_EDITOR_WARN("This entity already has the Physics2D Component!");
                ImGui::CloseCurrentPopup();
            }

            imgui_MenuItem("Collider2D") {
                if (!m_selectedEntity.hasComponent<Collider2DComponent>())
                    m_selectedEntity.addComponent<Collider2DComponent>();
                else
                    RB_EDITOR_WARN("This entity already has the Collider2D Component!");
                ImGui::CloseCurrentPopup();
            }

            imgui_MenuItem("Scripting") {
                if (!m_selectedEntity.hasComponent<ScriptComponent>())
                    m_selectedEntity.addComponent<ScriptComponent>();
                else
                    RB_EDITOR_WARN("This entity already has the Scripting Component!");
                ImGui::CloseCurrentPopup();
            }

            imgui_MenuItem("Text") {
                if (!m_selectedEntity.hasComponent<TextComponent>()) {
                    m_selectedEntity.addComponent<TextComponent>();
                    m_selectedEntity.getComponent<TransformComponent>().setSize({1.f, 1.f});
                }
                else
                    RB_EDITOR_WARN("This entity already has the Text Component!");
                ImGui::CloseCurrentPopup();
            }

            imgui_MenuItem("Animation") {
                if (!m_selectedEntity.hasComponent<AnimationComponent>()) {
                    m_selectedEntity.addComponent<AnimationComponent>();
                }
                else
                    RB_EDITOR_WARN("This entity already has the Animation Component!");
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::PopItemWidth();

        drawComponents(entity);
        drawUIComponents(entity);
    }

    void InspectorPanel::drawComponents(SceneEntity entity) {
        drawComponent<TransformComponent>( ICON_FA_WALKING + std::string{" Transform"}, entity, BIND_CLASS_FN(drawTransformComponent));
        drawComponent<CameraComponent>( ICON_FA_CAMERA + std::string{" Camera"}, entity, BIND_CLASS_FN(drawCameraComponent));
        drawComponent<DrawableComponent>(ICON_FA_PAINT_BRUSH + std::string{"  Drawable"}, entity, BIND_CLASS_FN(drawDrawableComponent));
        drawComponent<ScriptComponent>(ICON_FA_FILE_CODE + std::string{" Script"}, entity, BIND_CLASS_FN(drawScriptComponent));
        drawComponent<Physics2DComponent>(ICON_FA_HAMMER + std::string{"  Physics2D"}, entity, BIND_CLASS_FN(drawPhysics2DComponent));
        drawComponent<Collider2DComponent>(ICON_FA_JEDI + std::string{" Collider2D"}, entity, BIND_CLASS_FN(drawCollider2DComponent));
        drawComponent<TextComponent>(ICON_FA_TEXT_HEIGHT + std::string{"  Text"}, entity, BIND_CLASS_FN(drawTextComponent));
        drawComponent<AnimationComponent>(ICON_FA_CAMERA + std::string{" Animation"}, entity, BIND_CLASS_FN(drawAnimationComponent));
    }


    void InspectorPanel::drawTransformComponent([[maybe_unused]] SceneEntity entity, TransformComponent& component) {
        robot2D::vec2f lastPosition = component.getPosition();
        robot2D::vec2f lastSize = component.getSize();
        float lastRotation = component.getRotate();

        component.getScale();

        ui::drawVec2Control("Translation", component.getPosition());
        ui::drawVec2Control("Size", component.getSize(), 1.0f);
        ui::drawVec2Control("Origin", component.getOrigin(), 0.f, 100.f, 0.f, 1.f);
        ui::drawVec1Control("Rotation", component.getRotate(), 0.f);
        component.setRotate(component.getRotate());
        component.setPosition(component.getPosition());
        component.setSize(component.getSize());

        if (lastPosition != component.getPosition() || lastSize != component.getSize() ||
            lastRotation != component.getRotate())
            m_prefabHasModification = true;

    }
    
    void InspectorPanel::drawCameraComponent([[maybe_unused]] SceneEntity entity, CameraComponent& component) {
        auto& camera = component.camera;
        bool lastIsPrimary = component.isPrimary;
        ImGui::Checkbox("Primary", &component.isPrimary);

        if(component.isPrimary != lastIsPrimary) {
            /// TODO(a.raag) set or unset primary entity
        }

        float orthoSize = component.orthoSize;
        if (ImGui::DragFloat("Size", &orthoSize, 0.1))
            component.orthoSize = orthoSize;

        auto cameraRect = component.cameraRect;
        std::string posText = rbini::to_string(cameraRect.lx) + "x" + rbini::to_string(cameraRect.ly);
        std::string sizeText = rbini::to_string(cameraRect.width) + "x" + rbini::to_string(cameraRect.height);
        ImGui::Text("Position: %s", posText.c_str());
        ImGui::Text("Size: %s", sizeText.c_str());


        const char* aspectRatioTypeStrings[] = { "16:9", "9:16" };
        const char* aspectRatioProjectionTypeString = aspectRatioTypeStrings[(int)component.aspectRatio];

        imgui_Combo("AspectRation", aspectRatioProjectionTypeString) {
            for (int i = 0; i < 2; i++)
            {
                bool isSelected = aspectRatioProjectionTypeString == aspectRatioTypeStrings[i];
                if (ImGui::Selectable(aspectRatioTypeStrings[i], isSelected))
                {
                    aspectRatioProjectionTypeString = aspectRatioTypeStrings[i];
                    component.aspectRatio = static_cast<CameraComponent::AspectRatio>(i);
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }

        }

        const char* scalingModeVector[] = {"PixelArt_IntegerFit_Letterbox", "PixelArt_IntegerFill_Crop",
                                           "NonPixel_Fit_Letterbox", "NonPixel_Fill_Crop", "Stretch"};
        const char* scalingModeString = scalingModeVector[(int)component.scalingMode];
        imgui_Combo("ScalingMode", scalingModeString) {
            for (int i = 0; i < 5; i++)
            {
                bool isSelected = scalingModeString == scalingModeVector[i];
                if (ImGui::Selectable(scalingModeVector[i], isSelected))
                {
                    scalingModeString = scalingModeVector[i];
                    component.scalingMode = static_cast<CameraComponent::ScalingMode>(i);
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
        }

    }
    
    void InspectorPanel::drawDrawableComponent(SceneEntity entity, DrawableComponent& component) {
        auto color = component.getColor().toGL();
        auto p_color = reinterpret_cast<float*>(&color);
        ImGui::ColorEdit4("Color", p_color);
        ImGui::Checkbox("Draw BoundingBox", &component.drawBoundingBox());
        if (component.isUtil)
            return;

        component.setColor(robot2D::Color::fromGL(p_color[0], p_color[1], p_color[2], p_color[3]));
        int lastDepth = component.getDepth();
        ImGui::InputInt("zDepth", &component.getDepth());
        if (lastDepth != component.getDepth())
            component.setReorderZBuffer(true);

        ImGui::Button("Texture", ImVec2(100.0f, 0.0f));

        {
            robot2D::DragDropTarget dragDropTarget{ contentItemID };
            if(auto&& payloadBuffer = dragDropTarget.unpackPayload2Buffer()) {
                auto&& path = payloadBuffer.unpack<std::string>();
                std::filesystem::path localPath = std::filesystem::path("assets") / path;
                auto texturePath = combinePath(m_interactor -> getAssociatedProjectPath(), localPath.string());

                component.setTexturePath(localPath.string());
                auto manager = ResourceManager::getManager();
                if (!manager -> hasImage(localPath.filename().string())) {
                    auto queue = TaskQueue::GetQueue();
                    queue -> template addAsyncTask<ImageLoadTask>([](const ImageLoadTask& task) {
                        InspectorPanel::onLoadImage(task.getImage(), task.getEntity());
                    }, texturePath, entity);
                }
                else {
                    auto* localManager = LocalResourceManager::getManager();
                    auto idComponent = entity.getComponent<IDComponent>();
                    auto* texture = localManager -> addTexture(std::to_string(idComponent.ID));
                    if (texture) {
                        auto image = manager -> getImage(localPath.filename().string());
                        texture -> create(image);
                        component.setTexture(*texture);
                    }
                }
            }
        }

        if (entity.hasComponent<DrawableComponent>()) {
            auto& spriteComponent = entity.template getComponent<DrawableComponent>();
            if (spriteComponent.hasTexture()) {
                auto size = spriteComponent.getTexture().getSize();
                ImGui::Text("Width = %i", size.x);
                ImGui::SameLine();
                ImGui::Text("Height = %i", size.y);
                ImGui::Text("ImageColorFormat = %s", "RGBA");
            }
        }

    }

    void InspectorPanel::drawTextComponent(SceneEntity entity, TextComponent& component) {
        ImGui::Button("Font", ImVec2(100.0f, 0.0f));

        {
            robot2D::DragDropTarget dragDropTarget{ contentItemID };
            if(auto&& payloadBuffer = dragDropTarget.unpackPayload2Buffer()) {
                auto&& path = payloadBuffer.unpack<std::string>();
                std::filesystem::path localPath = std::filesystem::path("assets") / path;
                auto fontPath = combinePath(m_interactor -> getAssociatedProjectPath(), localPath.string());

                auto queue = TaskQueue::GetQueue();
                queue -> template addAsyncTask<FontLoadTask>([](const FontLoadTask& task) {
                    InspectorPanel::onLoadFont(task.getFont(), task.getEntity());
                }, fontPath, entity);
            }
        }

        if (component.getFont()) {
            robot2D::InputText("##Text", &component.getText(), 0);
            component.setText(component.getText());
        }
    }

    inline bool DrawColliderFilterEditor(phys2d::LayerRegistry& LR,
                                         phys2d::FilterBits& filter,
                                         bool& useDefaultMask)
    {
        bool changed = false;

        // Список доступных слоёв
        std::vector<int> idx;
        std::vector<const char*> names;
        for (int i = 0; i < phys2d::kMaxLayers; ++i) {
            const auto& n = LR.names()[i];
            if (!n.empty()) { idx.push_back(i); names.push_back(n.c_str()); }
        }
        if (names.empty()) {
            ImGui::TextDisabled("Нет слоёв.");
            return false;
        }

        // Текущий layer по categoryBits
        int currentLayerIndex = 0; // fallback
        for (size_t i = 0; i < idx.size(); ++i) {
            if (filter.categoryBits == (uint16_t)(1u << idx[i])) { currentLayerIndex = (int)i; break; }
        }

        // Layer dropdown
        ImGui::Text("Layer");
        ImGui::SameLine();
        if (ImGui::BeginCombo("##layer", names[currentLayerIndex])) {
            for (size_t i = 0; i < names.size(); ++i) {
                bool sel = (i == (size_t)currentLayerIndex);
                if (ImGui::Selectable(names[i], sel)) {
                    currentLayerIndex = (int)i;
                    filter.categoryBits = (uint16_t)(1u << idx[i]);
                    if (useDefaultMask) {
                        filter.maskBits = LR.defaultMaskFor(LR.names()[idx[i]]);
                    }
                    changed = true;
                }
                if (sel) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        // Use default mask toggle
        ImGui::Checkbox("Use default mask (from matrix)", &useDefaultMask);
        if (useDefaultMask) {
            // Подсказка: показываем маску как disabled
            ImGui::BeginDisabled();
        }

        // Collides With (локальные чекбоксы)
        ImGui::Text("Collides With:");
        int cols = 3;
        if (ImGui::BeginTable("mask_table", cols, ImGuiTableFlags_SizingFixedFit)) {
            int col = 0;
            for (size_t i = 0; i < idx.size(); ++i) {
                if (col == 0) ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(col);
                bool v = (filter.maskBits & (uint16_t)(1u << idx[i])) != 0;
                ImGui::PushID((int)i);
                if (ImGui::Checkbox(names[i], &v) && !useDefaultMask) {
                    if (v) filter.maskBits |=  (uint16_t)(1u << idx[i]);
                    else   filter.maskBits &= ~(uint16_t)(1u << idx[i]);
                    changed = true;
                }
                ImGui::PopID();
                col = (col + 1) % cols;
            }
            ImGui::EndTable();
        }

        if (useDefaultMask) {
            ImGui::EndDisabled();
            // синхронизируем фактическую маску
            filter.maskBits = LR.defaultMaskFor(LR.names()[idx[currentLayerIndex]]);
        }

        // Group Index
        ImGui::Separator();
        ImGui::SetNextItemWidth(100.f);
        if (ImGui::InputScalar("Group Index", ImGuiDataType_S16, &filter.groupIndex)) {
            changed = true;
        }
        ImGui::SameLine();
        ImGui::TextDisabled("0 – by masks, >0 – always inside group, <0 – newer inside group");

        return changed;
    }



    void InspectorPanel::drawPhysics2DComponent([[maybe_unused]] SceneEntity entity, Physics2DComponent& component) {
        const char* bodyTypeStrings[] = { "Static", "Dynamic", "Kinematic" };
        const char* currentBodyTypeString = bodyTypeStrings[(int)component.type];
        imgui_Combo("Body Type", currentBodyTypeString) {
            for (int i = 0; i < 3; i++)
            {
                bool isSelected = currentBodyTypeString == bodyTypeStrings[i];
                if (ImGui::Selectable(bodyTypeStrings[i], isSelected))
                {
                    currentBodyTypeString = bodyTypeStrings[i];
                    component.type = (Physics2DComponent::BodyType)i;
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }

        }
        ImGui::DragFloat("GravityScale", &component.gravityScale, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("LinearDamping", &component.linearDamping, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("AngularDamping", &component.angularDamping, 0.01f, 0.0f, 1.0f);
        ImGui::Checkbox("Fixed Rotation", &component.fixedRotation);
        ImGui::Checkbox("IsBullet", &component.bullet);
    }

    void InspectorPanel::drawCollider2DComponent([[maybe_unused]] SceneEntity entity, Collider2DComponent& component) {
        float offset[2] = { component.offset.x, component.offset.y };
        float size[2] = { component.size.x, component.size.y };
        ImGui::DragFloat2("Offset", offset);
        ImGui::DragFloat2("Size", size);
        ImGui::DragFloat("Density", &component.density, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Friction", &component.friction, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Restitution", &component.restitution, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Restitution Threshold", &component.restitutionThreshold, 0.01f, 0.0f);
        ImGui::DragFloat("SkipCollideTime(secs)", &component.skipCollideTime, 0.01f, 0.0f);
        component.offset = { offset[0], offset[1] };
        component.size = { size[0], size[1] };
        ImGui::Checkbox("IsTrigger", &component.isTrigger);
        ImGui::Checkbox("OneWay", &component.oneWay);

        phys2d::FilterBits fb = component.filter;       // categoryBits/maskBits/groupIndex
        bool useDefaultMask   = component.useDefaultMask;

        if (DrawColliderFilterEditor(phys2d::LayerRegistry::I(), fb, useDefaultMask)) {
            component.filter         = fb;
            component.useDefaultMask = useDefaultMask;
            component.markFilterDirty = true;           // чтобы обновить b2Fixture в рантайме
        }
    }

    void InspectorPanel::drawAnimationComponent(SceneEntity, AnimationComponent& component) {
        auto* animationManager = AnimationManager::getManager();
    }



    void InspectorPanel::drawScriptComponent(SceneEntity entity, ScriptComponent& component) {
        std::string currItem = component.name; // Here we store our selection data as an index.
        auto scriptInteractor = m_interactor -> getScriptInteractor();
        if(!scriptInteractor)
            return;
        bool hasScriptClass = currItem.empty() ? false : scriptInteractor -> hasEntityClass(component.name);

        {
            if(!hasScriptClass)
                robot2D::ScopedStyleColor{ImGuiCol_Text,
                                          robot2D::Color::fromGL(0.9f, 0.2f, 0.3f, 1.0f)};
            ImGui::Selectable(currItem.empty() ? "FindClass" : currItem.c_str());
        }

        if (ImGui::IsItemClicked())
            ImGui::OpenPopup("Find Script Class");

        imgui_PopupContextItem("Find Script Class") {
            auto classes = scriptInteractor -> getClassesNames();
            constexpr float filterWidth = 180.f;
            ImGuiTextFilter textFilter;
            textFilter.Draw("Classes", filterWidth);
            imgui_ListBox("##ListBox") {
                for (const auto& name: classes) {
                    if (!textFilter.PassFilter(name.c_str()))
                        continue;
                    const bool is_selected = (name == currItem);
                    ImGui::Selectable(name.c_str(), is_selected);
                    if (ImGui::IsItemClicked()) {
                        currItem = name;
                        component.name = currItem;
                        scriptInteractor -> setScriptClass(name, entity.getUUID());
                    }
                }
            }
        }

        bool isSceneRunning = m_interactor -> isRunning();

        /// ui -> interactor(business logic) -> scripting engine( service )

        if (isSceneRunning) {
/*            auto scriptInstance =
                ScriptEngine::getEntityScriptInstance(entity.getComponent<IDComponent>().ID);
            if(!scriptInstance)
                return;
            const auto& fields = scriptInstance -> getClassWrapper() -> getFields();
            for (const auto& [name, field] : fields)
            {
                switch(field.Type) {
                    default:
                        break;
                    case ScriptFieldType::Float: {
                        float data = scriptInstance -> getFieldValue<float>(name);
                        if (ImGui::DragFloat(name.c_str(), &data))
                            scriptInstance -> setFieldValue(name, data);
                    }
                }
            }*/
        }
        else {
            if (!hasScriptClass)
                return;

            processScriptComponent(entity, component);
        }
    }

    void InspectorPanel::processScriptComponent(SceneEntity entity, ScriptComponent& component) {
        auto scriptInteractor = m_interactor -> getScriptInteractor();
        if(!scriptInteractor)
            return;

        auto& fieldMap = scriptInteractor -> getFields(entity.getUUID());

        for (auto& [name, field] : fieldMap) {
            bool hasField = (fieldMap.find(name) != fieldMap.end());
            switch(field.getType()) {
                default:
                    break;
                case FieldType::Float: {
                    if(hasField) {
                        float* data = field.getValue<float>();
                        if (ImGui::DragFloat(name.c_str(), data))
                            field.setValue(data);
                    }
                    else {
                        float data = 0.0f;
                        if (ImGui::DragFloat(name.c_str(), &data))
                            field.setValue(data);
                    }

                    break;
                }
                case FieldType::Int: {
                    if(hasField) {
                        int* data = field.getValue<int>();
                        if (ImGui::DragInt(name.c_str(), data))
                            field.setValue(&data);
                    }
                    else {
                        /// TODO(a.raag): add logic
                    }

                    break;
                }
                case FieldType::Transform: {
                    std::string resultButtonText = "None";
                    if(hasField) {
                        auto uuid = *field.getValue<UUID>();
                        auto preEntity = m_interactor -> getEntity(uuid);
                        if (preEntity)
                            resultButtonText = preEntity.getComponent<TagComponent>().getTag();
                    }
                    drawScriptFieldInfo(name, resultButtonText);

                    {
                        robot2D::DragDropTarget dragDropTarget { treeNodeItemID };
                        if(auto uuid = dragDropTarget.unpackPayload<UUID>()) {
                            auto payloadEntity = m_uiManager.getTreeItem(*uuid);
                            if (payloadEntity && payloadEntity != entity) {
                                field.setValue(payloadEntity.getUUID());
                            }
                        }
                    }
                    break;
                }
                case FieldType::Entity: {
                    std::string resultButtonText = "None";
                    if(hasField) {
                        auto uuid = *field.getValue<UUID>();
                        auto preEntity = m_interactor -> getEntity(uuid);
                        if (preEntity)
                            resultButtonText = preEntity.getComponent<TagComponent>().getTag();
                    }
                    drawScriptFieldInfo(name, resultButtonText);
                    {
                        robot2D::DragDropTarget dragDropTarget{ contentPrefabItemID };
                        if(auto&& payloadBuffer = dragDropTarget.unpackPayload2Buffer()) {
                            auto&& path = payloadBuffer.unpack<std::string>();
                            std::filesystem::path prefabPath = std::filesystem::path("assets") / path;
                            auto realPrefabPath = combinePath(m_interactor -> getAssociatedProjectPath(),
                                                              prefabPath.string());

                            Prefab::Ptr prefab = m_prefabManager.loadPrefab(m_interactor, realPrefabPath);
                            if(!prefab)
                                break;

                            SceneEntity duplicateEntity = m_interactor -> duplicateEmptyEntity(prefab -> getEntity());
                            if (duplicateEntity && duplicateEntity != entity) {
                                field.setValue(duplicateEntity.getUUID());
                            }
                        }
                    }
                    break;
                }
            }
        }
    }

    void InspectorPanel::drawUIComponents(SceneEntity entity) {
        drawComponent<ButtonComponent>("Button", entity, BIND_CLASS_FN(drawUIButtonComponent));
    }

    void InspectorPanel::drawUIButtonComponent([[maybe_unused]] SceneEntity entity, ButtonComponent& component) {
        ImGui::Text("OnClick");
        std::string resultText = "No Object";
        if (component.hasEntity()) {
            auto preEntity = m_interactor -> getEntity(component.scriptEntity);

            if (preEntity)
                resultText = preEntity.template getComponent<TagComponent>().getTag();
        }

        drawScriptFieldInfo("Object", resultText);
        {
            robot2D::DragDropTarget dragDropTarget{ treeNodeItemID };
            if(auto uuid = dragDropTarget.unpackPayload<UUID>()) {
                auto payloadEntity = m_uiManager.getTreeItem(*uuid);

                if (payloadEntity && payloadEntity.hasComponent<ScriptComponent>())
                    component.scriptEntity = payloadEntity.getComponent<IDComponent>().ID;
            }
        }

        if (!component.hasEntity())
            return;

/*        auto klasses = ScriptEngine::getClasses();
        auto scriptEntity = m_interactor -> getEntity(component.scriptEntity);
        MonoClassWrapper::Ptr klass = klasses[scriptEntity.template getComponent<ScriptComponent>().name];

        std::string preview = component.clickMethodName.empty() ? "No Function" : component.clickMethodName;
        imgui_Combo("##RegMethods", preview.c_str()) {
            for (auto& [name, _] : klass->getRegisterMethods()) {
                const bool is_selected = (component.clickMethodName == name);
                if (ImGui::Selectable(name.c_str(), is_selected)) {
                    if (!component.onClickCallback) {
                        component.onClickCallback = [](UUID uuid, const std::string& methodName) {
                            auto instance = ScriptEngine::getEntityScriptInstance(uuid);
                            if (instance)
                                instance->getClassWrapper()->callMethod(methodName);
                        };
                    }
                    component.clickMethodName = name;
                }

                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
        }*/

    }


    void InspectorPanel::onLoadImage(const robot2D::Image& image, SceneEntity entity) {
        if(!entity) {
            RB_EDITOR_WARN("Can't attach texture to Entity, because it's already destroyed");
            return;
        }

        auto* localManager = LocalResourceManager::getManager();
        auto idComponent = entity.getComponent<IDComponent>();
        auto* texture = localManager -> addTexture(std::to_string(idComponent.ID));
        if(!texture)
            return;
        texture -> create(image);

        if(entity.hasComponent<DrawableComponent>())
            entity.getComponent<DrawableComponent>().setTexture(*texture);
    }

    void InspectorPanel::onLoadFont(const robot2D::Font& font, SceneEntity entity) {
        if(!entity) {
            RB_EDITOR_WARN("Can't attach texture to Entity, because it's already destroyed");
            return;
        }

        auto* localManager = LocalResourceManager::getManager();
        auto idComponent = entity.getComponent<IDComponent>();
        auto* f = localManager -> addFont(std::to_string(idComponent.ID));
        if(!f)
            return;
        f -> clone(const_cast<robot2D::Font&>(font));

        if(entity.hasComponent<TextComponent>())
            entity.getComponent<TextComponent>().setFont(*f);
    }



    void InspectorPanel::onPrefabAssetSelected(const PrefabAssetPressedMessage& message) {
        m_inspectType = InspectType::AssetPrefab;

        std::filesystem::path localPath = std::filesystem::path("assets") / message.localPath;
        auto fullPath = combinePath(m_interactor->getAssociatedProjectPath(), localPath.string());

        auto prefab = m_prefabManager.loadPrefab(m_interactor, fullPath);
        if (!prefab) {
            RB_EDITOR_ERROR("InspectorPanel: Can't load prefab by path {0}", fullPath);
        }
        else {
           m_selectedEntity = prefab -> getEntity();
        }
    }

    void InspectorPanel::onPanelEntityNeedSelect(const PanelEntitySelectedMessage& message) {
        if (m_inspectType == InspectType::AssetPrefab && m_prefabHasModification) {
            if (m_selectedEntity.hasComponent<PrefabComponent>()) {
                auto& prefabComponent = m_selectedEntity.getComponent<PrefabComponent>();
                m_prefabManager.savePrefab(m_interactor, prefabComponent.prefabUUID);
                m_prefabHasModification = false;
                auto* msg =
                    m_messageBus.postMessage<PrefabAssetModificatedMessage>(MessageID::PrefabAssetModificated);
                msg -> prefabUUID = prefabComponent.prefabUUID;
                msg -> prefabEntity = m_selectedEntity;
            }
        }

        m_inspectType = InspectType::EditorEntity;
        m_selectedEntity = message.entity;
    }

    void InspectorPanel::onPanelEntitySelected(const PanelEntitySelectedMessage& message) {
        if (m_inspectType == InspectType::AssetPrefab && m_prefabHasModification) {
            if (m_selectedEntity.hasComponent<PrefabComponent>()) {
                auto& prefabComponent = m_selectedEntity.getComponent<PrefabComponent>();
                m_prefabManager.savePrefab(m_interactor, prefabComponent.prefabUUID);
                m_prefabHasModification = false;
                auto* msg =
                    m_messageBus.postMessage<PrefabAssetModificatedMessage>(MessageID::PrefabAssetModificated);
                msg -> prefabUUID = prefabComponent.prefabUUID;
                msg -> prefabEntity = m_selectedEntity;
            }
        }
        m_inspectType = InspectType::EditorEntity;
        m_selectedEntity = message.entity;
    }
}