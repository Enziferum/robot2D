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

#include <type_traits>
#include <robot2D/imgui/Api.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <editor/panels/InspectorPanel.hpp>
#include <editor/SelectionService.hpp>
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



    template<typename T, typename UIFunction>
    static void drawComponent(const std::string& name, SceneEntity& entity, UIFunction uiFunction) {
        if(!entity || !entity.hasComponent<T>())
            return;
        ImGui::PushID(typeid(T).name());
        auto position = ImGui::GetCursorScreenPos();
        auto width = ImGui::GetContentRegionAvail().x;
        auto buttonSize = ImGui::GetFrameHeight();
        auto flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed
            | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap
            | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        bool open = ImGui::TreeNodeEx("##Component", flags, "%s", name.c_str());
        ImGui::SameLine();
        ImGui::SetCursorScreenPos({position.x + ImMax(0.f, width - buttonSize), position.y});
        if(ImGui::Button("...", {buttonSize, buttonSize}))
            ImGui::OpenPopup("ComponentSettings");
        if(ImGui::IsItemHovered())
            ImGui::SetTooltip("Component settings");
        bool remove = false;
        if(ImGui::BeginPopup("ComponentSettings")) {
            remove = ImGui::MenuItem("Remove component", nullptr, false,
                    !std::is_same_v<T, TransformComponent>);
            if constexpr(std::is_same_v<T, TransformComponent>)
                ImGui::TextDisabled("Required component");
            ImGui::EndPopup();
        }
        if(open) {
            ImGui::Indent(8.f);
            ImGui::Spacing();
            ImGui::PushItemWidth(ImMax(40.f, ImGui::GetContentRegionAvail().x - 100.f));
            uiFunction(entity, entity.getComponent<T>());
            ImGui::PopItemWidth();
            ImGui::Unindent(8.f);
        }
        ImGui::Spacing();
        ImGui::PopID();
        if(remove) {
            entity.removeComponent<T>();
            entity.markModified();
        }
    }

    template<typename T>
    static bool addComponentMenuItem(const char* name, SceneEntity& entity) {
        if(ImGui::MenuItem(name, nullptr, false, !entity.hasComponent<T>())) {
            entity.addComponent<T>();
            entity.markModified();
            return true;
        }
        return false;
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
        IUIManager& uiManager) :
        IPanel(typeid(InspectorPanel)),
        m_messageDispatcher{messageDispatcher},
        m_messageBus{messageBus},
        m_prefabManager{prefabManager},
        m_uiManager{uiManager} {
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
        m_selectedEntity = {};
        m_inspectType = InspectType::EditorEntity;
    }

    void InspectorPanel::setSelected(SceneEntity entity) {
        if(!savePrefabChanges())
            return;
        m_selectedEntity = entity;
        m_inspectType = InspectType::EditorEntity;
    }


    void InspectorPanel::clearSelection() {
        if(savePrefabChanges())
            discardSelection();
    }

    void InspectorPanel::discardSelection() {
        m_selectedEntity = {};
        m_inspectType = InspectType::EditorEntity;
    }


    void InspectorPanel::render() {
        robot2D::WindowOptions propertiesWindowOptions{};
        propertiesWindowOptions.name = ICON_FA_INFO_CIRCLE + std::string{" Inspector"} + "###InspectorPanel";

        renderWindow(propertiesWindowOptions, [this]{
            if(m_interactor && m_inspectType == InspectType::EditorEntity) {
                auto active = m_interactor -> getSelectionService().lastActive();
                m_selectedEntity = active ? m_interactor -> getEntity(*active) : SceneEntity{};
            }
            if(m_selectedEntity && m_inspectType == InspectType::EditorEntity)
                drawComponentsBase(m_selectedEntity);
            else if(m_inspectType != InspectType::EditorEntity)
                drawAssetBase();
            else {
                ImGui::TextDisabled("No object selected");
                ImGui::TextWrapped("Select an object in Scene or Viewport.");
            }
        });
    }


    void InspectorPanel::drawAssetBase() {
        switch(m_inspectType) {
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
        auto entityID = std::to_string(static_cast<std::uint64_t>(entity.getUUID()));
        ImGui::PushID(entityID.c_str());
        if(m_inspectType == InspectType::EditorEntity) {
            if(!entity.hasComponent<TagComponent>()) {
                ImGui::PopID();
                return;
            }
            ImGui::TextDisabled("Name");
            auto tag = entity.getComponent<TagComponent>().getTag();
            ImGui::SetNextItemWidth(-1.f);
            auto flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll;
            if(robot2D::InputText("##Tag", &tag, flags)) {
                if(tag.empty())
                    tag = "Untitled Entity";
                if(entity.getName() != tag) {
                    entity.getComponent<TagComponent>().setTag(tag);
                    entity.markModified();
                }
            }
        }
        else {
            if(!entity.hasComponent<PrefabComponent>()) {
                ImGui::PopID();
                return;
            }
            ImGui::TextUnformatted("Prefab");
        }
        if(ImGui::CollapsingHeader("Details")) {
            auto uuid = m_inspectType == InspectType::EditorEntity ? entity.getUUID()
                        : entity.getComponent<PrefabComponent>().prefabUUID;
            auto value = std::to_string(static_cast<std::uint64_t>(uuid));
            ImGui::TextDisabled("UUID");
            ImGui::TextWrapped("%s", value.c_str());
            if(ImGui::SmallButton("Copy UUID"))
                ImGui::SetClipboardText(value.c_str());
        }
        ImGui::Spacing();
        drawComponents(entity);
        drawUIComponents(entity);
        ImGui::Spacing();
        if(ImGui::Button("Add Component", {ImGui::GetContentRegionAvail().x, 0.f}))
            ImGui::OpenPopup("AddComponent");
        if(ImGui::BeginPopup("AddComponent")) {
            addComponentMenuItem<CameraComponent>("Camera", entity);
            if(addComponentMenuItem<DrawableComponent>("Utility renderer", entity))
                entity.getComponent<DrawableComponent>().isUtil = true;
            addComponentMenuItem<DrawableComponent>("Sprite renderer", entity);
            addComponentMenuItem<Physics2DComponent>("Rigid body 2D", entity);
            addComponentMenuItem<Collider2DComponent>("Collider 2D", entity);
            addComponentMenuItem<ScriptComponent>("Script", entity);
            if(addComponentMenuItem<TextComponent>("Text", entity))
                entity.getComponent<TransformComponent>().setSize({1.f, 1.f});
            addComponentMenuItem<AnimationComponent>("Animation", entity);
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }
    void InspectorPanel::drawComponents(SceneEntity entity) {
        drawComponent<TransformComponent>( ICON_FA_WALKING + std::string{" Transform"}, entity,
            BIND_CLASS_FN(drawTransformComponent));
        drawComponent<CameraComponent>( ICON_FA_CAMERA + std::string{" Camera"}, entity,
            BIND_CLASS_FN(drawCameraComponent));
        drawComponent<DrawableComponent>(ICON_FA_PAINT_BRUSH + std::string{" Drawable"}, entity,
            BIND_CLASS_FN(drawDrawableComponent));
        drawComponent<ScriptComponent>(ICON_FA_FILE_CODE + std::string{" Script"}, entity,
            BIND_CLASS_FN(drawScriptComponent));
        drawComponent<Physics2DComponent>(ICON_FA_HAMMER + std::string{" Physics2D"}, entity,
            BIND_CLASS_FN(drawPhysics2DComponent));
        drawComponent<Collider2DComponent>(ICON_FA_JEDI + std::string{" Collider2D"}, entity,
            BIND_CLASS_FN(drawCollider2DComponent));
        drawComponent<TextComponent>(ICON_FA_TEXT_HEIGHT + std::string{" Text"}, entity,
            BIND_CLASS_FN(drawTextComponent));
        drawComponent<AnimationComponent>(ICON_FA_CAMERA + std::string{" Animation"}, entity,
            BIND_CLASS_FN(drawAnimationComponent));
    }


    void InspectorPanel::drawTransformComponent([[maybe_unused]] SceneEntity entity, TransformComponent& component) {
        robot2D::vec2f lastPosition = component.getPosition();
        robot2D::vec2f lastSize = component.getSize();
        float lastRotation = component.getRotate();
        auto lastOrigin = component.getOrigin();

        component.getScale();

        ui::drawVec2Control("Translation", component.getPosition());
        ui::drawVec2Control("Size", component.getSize(), 1.0f);
        ui::drawVec2Control("Origin", component.getOrigin(), 0.f, 100.f, 0.f, 1.f);
        ui::drawVec1Control("Rotation", component.getRotate(), 0.f);
        component.setRotate(component.getRotate());
        component.setPosition(component.getPosition());
        component.setSize(component.getSize());

        if(lastPosition != component.getPosition() || lastSize != component.getSize() ||
            lastRotation != component.getRotate() || lastOrigin != component.getOrigin())
            entity.markModified();

    }

    void InspectorPanel::drawCameraComponent([[maybe_unused]] SceneEntity entity, CameraComponent& component) {
        auto& camera = component.camera;
        bool lastIsPrimary = component.isPrimary;
        if(ImGui::Checkbox("Primary", &component.isPrimary))
            entity.markModified();

        if(component.isPrimary != lastIsPrimary) {
            /// TODO(a.raag) set or unset primary entity
        }

        float orthoSize = component.orthoSize;
        if(ImGui::DragFloat("Size", &orthoSize, 0.1)) {
            component.orthoSize = orthoSize;
            entity.markModified();
        }

        auto cameraRect = component.cameraRect;
        std::string posText = rbini::to_string(cameraRect.lx) + "x" + rbini::to_string(cameraRect.ly);
        std::string sizeText = rbini::to_string(cameraRect.width) + "x" + rbini::to_string(cameraRect.height);
        ImGui::Text("Position: %s", posText.c_str());
        ImGui::Text("Size: %s", sizeText.c_str());


        const char* aspectRatioTypeStrings[] = { "16:9", "9:16" };
        const char* aspectRatioProjectionTypeString = aspectRatioTypeStrings[(int)component.aspectRatio];

        imgui_Combo("AspectRation", aspectRatioProjectionTypeString) {
            for(int i = 0; i < 2; i++) {
                bool isSelected = aspectRatioProjectionTypeString == aspectRatioTypeStrings[i];
                if(ImGui::Selectable(aspectRatioTypeStrings[i], isSelected)) {
                    aspectRatioProjectionTypeString = aspectRatioTypeStrings[i];
                    component.aspectRatio = static_cast<CameraComponent::AspectRatio>(i);
                    if(!isSelected) entity.markModified();
                }

                if(isSelected)
                    ImGui::SetItemDefaultFocus();
            }

        }

        const char* scalingModeVector[] = {"PixelArt_IntegerFit_Letterbox", "PixelArt_IntegerFill_Crop",
                                           "NonPixel_Fit_Letterbox", "NonPixel_Fill_Crop", "Stretch"};
        const char* scalingModeString = scalingModeVector[(int)component.scalingMode];
        imgui_Combo("ScalingMode", scalingModeString) {
            for(int i = 0; i < 5; i++) {
                bool isSelected = scalingModeString == scalingModeVector[i];
                if(ImGui::Selectable(scalingModeVector[i], isSelected)) {
                    scalingModeString = scalingModeVector[i];
                    component.scalingMode = static_cast<CameraComponent::ScalingMode>(i);
                    if(!isSelected) entity.markModified();
                }

                if(isSelected)
                    ImGui::SetItemDefaultFocus();
            }
        }

    }

    void InspectorPanel::drawDrawableComponent(SceneEntity entity, DrawableComponent& component) {
        auto color = component.getColor().toGL();
        auto p_color = reinterpret_cast<float*>(&color);
        if(ImGui::ColorEdit4("Color", p_color))
            entity.markModified();
        if(ImGui::Checkbox("Draw BoundingBox", &component.drawBoundingBox()))
            entity.markModified();
        component.setColor(robot2D::Color::fromGL(p_color[0], p_color[1], p_color[2], p_color[3]));
        if(component.isUtil)
            return;

        int lastDepth = component.getDepth();
        ImGui::InputInt("zDepth", &component.getDepth());
        if(lastDepth != component.getDepth()) {
            component.setReorderZBuffer(true);
            entity.markModified();
        }

        ImGui::Button("Texture", ImVec2(100.0f, 0.0f));

        {
            robot2D::DragDropTarget dragDropTarget{ contentItemID };
            if(auto && payloadBuffer = dragDropTarget.unpackPayload2Buffer()) {
                auto&& path = payloadBuffer.unpack<std::string>();
                std::filesystem::path localPath = std::filesystem::path("assets") / path;
                auto texturePath = resourceFileKey(combinePath(m_interactor -> getAssociatedProjectPath(),
                        localPath.string()));

                if(component.getTexturePath() != localPath.string()) {
                    component.setTexturePath(localPath.string());
                    entity.markModified();
                }
                auto manager = ResourceManager::getManager();
                if(!manager -> hasImage(texturePath)) {
                    auto queue = TaskQueue::GetQueue();
                    // Keep a discarded template alive until its pending resource callback finishes.
                    auto owner = m_inspectType == InspectType::AssetPrefab
                            ? m_prefabManager.findPrefab(m_selectedEntity.getComponent<PrefabComponent>().prefabUUID)
                            : nullptr;
                    queue -> template addAsyncTask<ImageLoadTask>([owner, texturePath,
                        reference = localPath.string()](const ImageLoadTask& task) {
                        auto target = task.getEntity();
                        if(target.hasComponent<DrawableComponent>() &&
                        target.getComponent<DrawableComponent>().getTexturePath() == reference)
                            InspectorPanel::onLoadImage(task.getImage(), target, texturePath);
                    }, texturePath, entity);
                }
                else {
                    onLoadImage(manager -> getImage(texturePath), entity, texturePath);
                }
            }
        }

        if(entity.hasComponent<DrawableComponent>()) {
            auto& spriteComponent = entity.template getComponent<DrawableComponent>();
            if(spriteComponent.hasTexture()) {
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
            if(auto && payloadBuffer = dragDropTarget.unpackPayload2Buffer()) {
                auto&& path = payloadBuffer.unpack<std::string>();
                std::filesystem::path localPath = std::filesystem::path("assets") / path;
                auto fontPath = resourceFileKey(combinePath(m_interactor -> getAssociatedProjectPath(),
                        localPath.string()));

                if(component.getFontPath() != localPath.string()) {
                    component.setFontPath(localPath.string());
                    entity.markModified();
                }

                auto queue = TaskQueue::GetQueue();
                auto owner = m_inspectType == InspectType::AssetPrefab
                        ? m_prefabManager.findPrefab(m_selectedEntity.getComponent<PrefabComponent>().prefabUUID)
                        : nullptr;
                queue -> template addAsyncTask<FontLoadTask>([owner, fontPath,
                    reference = localPath.string()](const FontLoadTask& task) {
                    auto target = task.getEntity();
                    if(target.hasComponent<TextComponent>() &&
                    target.getComponent<TextComponent>().getFontPath() == reference)
                        InspectorPanel::onLoadFont(task.getFont(), target, fontPath);
                }, fontPath, entity);
            }
        }

        if(component.getFont()) {
            if(robot2D::InputText("##Text", &component.getText(), 0)) {
                component.setText(component.getText());
                entity.markModified();
            }
        }
    }

    inline bool DrawColliderFilterEditor(phys2d::LayerRegistry& LR,
        phys2d::FilterBits& filter,
        bool& useDefaultMask) {
        bool changed = false;

        // Список доступных слоёв
        std::vector<int> idx;
        std::vector<const char*> names;
        for(int i = 0; i < phys2d::kMaxLayers; ++i) {
            const auto& n = LR.names()[i];
            if(!n.empty()) {
                idx.push_back(i);
                names.push_back(n.c_str());
            }
        }
        if(names.empty()) {
            ImGui::TextDisabled("Нет слоёв.");
            return false;
        }

        // Текущий layer по categoryBits
        int currentLayerIndex = 0; // fallback
        for(size_t i = 0; i < idx.size(); ++i) {
            if(filter.categoryBits == (uint16_t)(1u << idx[i])) {
                currentLayerIndex = (int)i; break;
            }
        }

        // Layer dropdown
        ImGui::Text("Layer");
        ImGui::SameLine();
        if(ImGui::BeginCombo("##layer", names[currentLayerIndex])) {
            for(size_t i = 0; i < names.size(); ++i) {
                bool sel = (i == (size_t)currentLayerIndex);
                if(ImGui::Selectable(names[i], sel)) {
                    currentLayerIndex = (int)i;
                    filter.categoryBits = (uint16_t)(1u << idx[i]);
                    if(useDefaultMask) {
                        filter.maskBits = LR.defaultMaskFor(LR.names()[idx[i]]);
                        filter.name = names[i];
                    }
                    changed = true;
                }
                if(sel)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        // Use default mask toggle
        ImGui::Checkbox("Use default mask (from matrix)", &useDefaultMask);
        if(useDefaultMask) {
            // Подсказка: показываем маску как disabled
            ImGui::BeginDisabled();
        }

        // Collides With (локальные чекбоксы)
        ImGui::Text("Collides With:");
        int cols = 3;
        if(ImGui::BeginTable("mask_table", cols, ImGuiTableFlags_SizingFixedFit)) {
            int col = 0;
            for(size_t i = 0; i < idx.size(); ++i) {
                if(col == 0)  ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(col);
                bool v = (filter.maskBits & (uint16_t)(1u << idx[i])) != 0;
                ImGui::PushID((int)i);
                if(ImGui::Checkbox(names[i], &v) && !useDefaultMask) {
                    if(v)  filter.maskBits |= (uint16_t)(1u << idx[i]);
                    else filter.maskBits &= ~(uint16_t)(1u << idx[i]);
                    changed = true;
                }
                ImGui::PopID();
                col = (col + 1) % cols;
            }
            ImGui::EndTable();
        }

        if(useDefaultMask) {
            ImGui::EndDisabled();
            // синхронизируем фактическую маску
            filter.maskBits = LR.defaultMaskFor(LR.names()[idx[currentLayerIndex]]);
        }

        // Group Index
        ImGui::Separator();
        ImGui::SetNextItemWidth(100.f);
        if(ImGui::InputScalar("Group Index", ImGuiDataType_S16, &filter.groupIndex)) {
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
            for(int i = 0; i < 3; i++) {
                bool isSelected = currentBodyTypeString == bodyTypeStrings[i];
                if(ImGui::Selectable(bodyTypeStrings[i], isSelected)) {
                    currentBodyTypeString = bodyTypeStrings[i];
                    component.type = (Physics2DComponent::BodyType)i;
                    if(!isSelected) entity.markModified();
                }

                if(isSelected)
                    ImGui::SetItemDefaultFocus();
            }

        }
        bool changed = ImGui::DragFloat("GravityScale", &component.gravityScale, 0.01f, 0.0f, 1.0f);
        changed |= ImGui::DragFloat("LinearDamping", &component.linearDamping, 0.01f, 0.0f, 1.0f);
        changed |= ImGui::DragFloat("AngularDamping", &component.angularDamping, 0.01f, 0.0f, 1.0f);
        changed |= ImGui::Checkbox("Fixed Rotation", &component.fixedRotation);
        changed |= ImGui::Checkbox("IsBullet", &component.bullet);
        if(changed)
            entity.markModified();
    }

    void InspectorPanel::drawCollider2DComponent([[maybe_unused]] SceneEntity entity, Collider2DComponent& component) {
        float offset[2] = { component.offset.x, component.offset.y };
        float size[2] = { component.size.x, component.size.y };
        bool changed = ImGui::DragFloat2("Offset", offset);
        changed |= ImGui::DragFloat2("Size", size);
        changed |= ImGui::DragFloat("Density", &component.density, 0.01f, 0.0f, 1.0f);
        changed |= ImGui::DragFloat("Friction", &component.friction, 0.01f, 0.0f, 1.0f);
        changed |= ImGui::DragFloat("Restitution", &component.restitution, 0.01f, 0.0f, 1.0f);
        changed |= ImGui::DragFloat("Restitution Threshold", &component.restitutionThreshold, 0.01f, 0.0f);
        changed |= ImGui::DragFloat("SkipCollideTime(secs)", &component.skipCollideTime, 0.01f, 0.0f);
        component.offset = { offset[0], offset[1] };
        component.size = { size[0], size[1] };
        changed |= ImGui::Checkbox("IsTrigger", &component.isTrigger);
        changed |= ImGui::Checkbox("OneWay", &component.oneWay);

        phys2d::FilterBits fb = component.filter;       // categoryBits/maskBits/groupIndex
        bool useDefaultMask = component.useDefaultMask;

        if(DrawColliderFilterEditor(phys2d::LayerRegistry::I(), fb, useDefaultMask)) {
            component.filter = fb;
            component.useDefaultMask = useDefaultMask;
            component.markFilterDirty = true;           // чтобы обновить b2Fixture в рантайме
            changed = true;
        }
        if(changed)
            entity.markModified();
    }

    void InspectorPanel::drawAnimationComponent(SceneEntity, AnimationComponent& component) {
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

        if(ImGui::IsItemClicked())
            ImGui::OpenPopup("Find Script Class");

        imgui_PopupContextItem("Find Script Class") {
            auto classes = scriptInteractor -> getClassesNames();
            constexpr float filterWidth = 180.f;
            ImGuiTextFilter textFilter;
            textFilter.Draw("Classes", filterWidth);
            imgui_ListBox("##ListBox") {
                for(const auto& name: classes) {
                    if(!textFilter.PassFilter(name.c_str()))
                        continue;
                    const bool is_selected = (name == currItem);
                    ImGui::Selectable(name.c_str(), is_selected);
                    if(ImGui::IsItemClicked()) {
                        currItem = name;
                        component.name = currItem;
                        scriptInteractor -> setScriptClass(name, entity.getUUID());
                        if(!is_selected) entity.markModified();
                    }
                }
            }
        }

        bool isSceneRunning = m_interactor -> isRunning();

        /// ui -> interactor(business logic) -> scripting engine( service )

        if(isSceneRunning) {
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
            if(!hasScriptClass)
                return;

            processScriptComponent(entity, component);
        }
    }

    void InspectorPanel::processScriptComponent(SceneEntity entity, ScriptComponent& component) {
        auto scriptInteractor = m_interactor -> getScriptInteractor();
        if(!scriptInteractor)
            return;

        auto& fieldMap = scriptInteractor -> getFields(entity.getUUID());

        for(auto& [name, field] : fieldMap) {
            bool hasField = (fieldMap.find(name) != fieldMap.end());
            switch(field.getType()) {
            default:
                break;
            case FieldType::Float: {
                if(hasField) {
                    float* data = field.getValue<float>();
                    if(ImGui::DragFloat(name.c_str(), data)) {
                        field.setValue(data);
                        entity.markModified();
                    }
                }
                else {
                    float data = 0.0f;
                    if(ImGui::DragFloat(name.c_str(), &data)) {
                        field.setValue(data);
                        entity.markModified();
                    }
                }

                break;
            }
            case FieldType::Int: {
                if(hasField) {
                    int* data = field.getValue<int>();
                    if(ImGui::DragInt(name.c_str(), data)) {
                        field.setValue(data);
                        entity.markModified();
                    }
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
                    auto preEntity = resolveEntity(uuid);
                    if(preEntity)
                        resultButtonText = preEntity.getComponent<TagComponent>().getTag();
                }
                drawScriptFieldInfo(name, resultButtonText);

                {
                    robot2D::DragDropTarget dragDropTarget { treeNodeItemID };
                    if(auto uuid = dragDropTarget.unpackPayload<UUID>()) {
                        auto payloadEntity = m_uiManager.getTreeItem(*uuid);
                        if(payloadEntity && payloadEntity != entity) {
                            field.setValue(payloadEntity.getUUID());
                            entity.markModified();
                        }
                    }
                }
                break;
            }
            case FieldType::Entity: {
                std::string resultButtonText = "None";
                if(hasField) {
                    auto uuid = *field.getValue<UUID>();
                    auto preEntity = resolveEntity(uuid);
                    if(preEntity)
                        resultButtonText = preEntity.getComponent<TagComponent>().getTag();
                }
                drawScriptFieldInfo(name, resultButtonText);
                {
                    robot2D::DragDropTarget dragDropTarget{ contentPrefabItemID };
                    if(auto && payloadBuffer = dragDropTarget.unpackPayload2Buffer()) {
                        auto&& path = payloadBuffer.unpack<std::string>();
                        std::filesystem::path prefabPath = std::filesystem::path("assets") / path;
                        auto realPrefabPath = combinePath(m_interactor -> getAssociatedProjectPath(),
                                prefabPath.string());

                        Prefab::Ptr prefab = m_prefabManager.loadPrefab(m_interactor, realPrefabPath);
                        if(!prefab)
                            break;

                        SceneEntity duplicateEntity = m_interactor -> duplicateEmptyEntity(prefab -> getEntity());
                        if(duplicateEntity && duplicateEntity != entity) {
                            field.setValue(duplicateEntity.getUUID());
                            entity.markModified();
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
        if(component.hasEntity()) {
            auto preEntity = resolveEntity(component.scriptEntity);

            if(preEntity)
                resultText = preEntity.template getComponent<TagComponent>().getTag();
        }

        drawScriptFieldInfo("Object", resultText);
        {
            robot2D::DragDropTarget dragDropTarget{ treeNodeItemID };
            if(auto uuid = dragDropTarget.unpackPayload<UUID>()) {
                auto payloadEntity = m_uiManager.getTreeItem(*uuid);

                if(payloadEntity && payloadEntity.hasComponent<ScriptComponent>()) {
                    component.scriptEntity = payloadEntity.getComponent<IDComponent>().ID;
                    entity.markModified();
                }
            }
        }

        if(!component.hasEntity())
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


    void InspectorPanel::onLoadImage(const robot2D::Image& image, SceneEntity entity, const std::string& key) {
        if(!entity) {
            RB_EDITOR_WARN("Can't attach texture to Entity, because it's already destroyed");
            return;
        }

        auto* localManager = LocalResourceManager::getManager();
        auto* texture = localManager -> hasTexture(key) ? &localManager -> getTexture(key)
                : localManager -> addTexture(key);
        if(!texture)
            return;
        texture -> create(image);

        if(entity.hasComponent<DrawableComponent>())
            entity.getComponent<DrawableComponent>().setTexture(*texture);
    }

    void InspectorPanel::onLoadFont(const robot2D::Font& font, SceneEntity entity, const std::string& key) {
        if(!entity) {
            RB_EDITOR_WARN("Can't attach texture to Entity, because it's already destroyed");
            return;
        }

        auto* localManager = LocalResourceManager::getManager();
        if(localManager -> hasFont(key)) {
            if(entity.hasComponent<TextComponent>())
                entity.getComponent<TextComponent>().setFont(localManager -> getFont(key));
            return;
        }
        auto* f = localManager -> addFont(key);
        if(!f)
            return;
        f -> clone(const_cast<robot2D::Font&>(font));

        if(entity.hasComponent<TextComponent>())
            entity.getComponent<TextComponent>().setFont(*f);
    }



    SceneEntity InspectorPanel::resolveEntity(UUID uuid) const {
        if(m_inspectType == InspectType::AssetPrefab && m_selectedEntity.hasComponent<PrefabComponent>()) {
            auto prefab = m_prefabManager.findPrefab(m_selectedEntity.getComponent<PrefabComponent>().prefabUUID);
            if(prefab && prefab -> getTemplateScene()) {
                auto entity = prefab -> getTemplateScene() -> getEntity(uuid);
                if(entity)
                    return entity;
            }
        }
        return m_interactor ? m_interactor -> getEntity(uuid) : SceneEntity{};
    }

    bool InspectorPanel::savePrefabChanges() {
        if(m_inspectType != InspectType::AssetPrefab)
            return true;
        if(!m_selectedEntity.hasComponent<PrefabComponent>())
            return false;
        const auto id = m_selectedEntity.getComponent<PrefabComponent>().prefabUUID;
        auto prefab = m_prefabManager.findPrefab(id);
        if(!prefab || !prefab -> getTemplateScene())
            return false;
        if(!prefab -> getTemplateScene() -> hasChanges())
            return true;
        if(!m_prefabManager.savePrefab(m_interactor, id)) {
            RB_EDITOR_ERROR("InspectorPanel: Can't save modified prefab");
            return false;
        }
        m_interactor -> applyPrefabChanges(id, m_selectedEntity);
        return true;
    }

    void InspectorPanel::onPrefabAssetSelected(const PrefabAssetPressedMessage& message) {
        if(!m_interactor || !savePrefabChanges())
            return;
        const auto localPath = std::filesystem::path("assets") / message.localPath;
        const auto fullPath = combinePath(m_interactor -> getAssociatedProjectPath(), localPath.string());
        auto prefab = m_prefabManager.loadPrefab(m_interactor, fullPath);
        if(!prefab) {
            RB_EDITOR_ERROR("InspectorPanel: Can't load prefab by path {0}", fullPath);
            return;
        }
        m_selectedEntity = prefab -> getEntity();
        m_inspectType = InspectType::AssetPrefab;
    }

    void InspectorPanel::onPanelEntityNeedSelect(const PanelEntitySelectedMessage& message) {
        onPanelEntitySelected(message);
    }

    void InspectorPanel::onPanelEntitySelected(const PanelEntitySelectedMessage& message) {
        if(!savePrefabChanges())
            return;
        m_inspectType = InspectType::EditorEntity;
        m_selectedEntity = message.entity;
    }
}
