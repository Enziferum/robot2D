/*********************************************************************
(c) Alex Raag 2023
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

#include <editor/components/ButtonComponent.hpp>
#include <editor/async/ImageLoadTask.hpp>
#include <editor/async/FontLoadTask.hpp>
#include <editor/AnimationManager.hpp>

namespace editor {

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
                m_prefabManager.savePrefab(prefabComponent.prefabUUID);
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
        propertiesWindowOptions.name = "Inspector";

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
                    m_selectedEntity.getComponent<TransformComponent>().setScale({1.f, 1.f});
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
        drawComponent<TransformComponent>("Transform", entity, BIND_CLASS_FN(drawTransformComponent));
        drawComponent<CameraComponent>("Camera", entity, BIND_CLASS_FN(drawCameraComponent));
        drawComponent<DrawableComponent>("Drawable", entity, BIND_CLASS_FN(drawDrawableComponent));
        drawComponent<ScriptComponent>("Script", entity, BIND_CLASS_FN(drawScriptComponent));
        drawComponent<Physics2DComponent>("physics2D", entity, BIND_CLASS_FN(drawPhysics2DComponent));
        drawComponent<Collider2DComponent>("Collider2D", entity, BIND_CLASS_FN(drawCollider2DComponent));
        drawComponent<TextComponent>("Text", entity, BIND_CLASS_FN(drawTextComponent));
        drawComponent<AnimationComponent>("Animation", entity, BIND_CLASS_FN(drawAnimationComponent));
    }


    void InspectorPanel::drawTransformComponent([[maybe_unused]] SceneEntity entity, TransformComponent& component) {
        robot2D::vec2f lastPosition = component.getPosition();
        robot2D::vec2f lastScale = component.getScale();
        float lastRotation = component.getRotate();

        ui::drawVec2Control("Translation", component.getPosition());
        ui::drawVec2Control("Scale", component.getScale(), 1.0f);
        ui::drawVec1Control("Rotation", component.getRotate(), 0.f);
        component.setRotate(component.getRotate());

        if (lastPosition != component.getPosition() || lastScale != component.getScale() ||
            lastRotation != component.getRotate())
            m_prefabHasModification = true;

    }
    
    void InspectorPanel::drawCameraComponent([[maybe_unused]] SceneEntity entity, CameraComponent& component) {
        auto& camera = component.camera;
        ImGui::Checkbox("Primary", &component.isPrimary);
        float orthoSize = component.orthoSize;
        if (ImGui::DragFloat("Size", &orthoSize, 0.1))
            component.orthoSize = orthoSize;

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
    }
    
    void InspectorPanel::drawDrawableComponent(SceneEntity entity, DrawableComponent& component) {
        auto color = component.getColor().toGL();
        auto p_color = reinterpret_cast<float*>(&color);
        ImGui::ColorEdit4("Color", p_color);

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
        ImGui::Checkbox("Fixed Rotation", &component.fixedRotation);
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
        component.offset = { offset[0], offset[1] };
        component.size = { size[0], size[1] };
    }

    void InspectorPanel::drawAnimationComponent(SceneEntity, AnimationComponent& component) {
        auto* animationManager = AnimationManager::getManager();
    }

    void InspectorPanel::drawScriptComponent(SceneEntity entity, ScriptComponent& component) {
        std::string currItem = component.name; // Here we store our selection data as an index.
        bool hasScriptClass = ScriptEngine::hasEntityClass(component.name);

        {
            if(!hasScriptClass)
                robot2D::ScopedStyleColor{ImGuiCol_Text, robot2D::Color::fromGL(0.9f, 0.2f, 0.3f, 1.0f)};
            ImGui::Selectable(currItem.empty() ? "FindClass" : currItem.c_str());
        }

        if (ImGui::IsItemClicked())
            ImGui::OpenPopup("Find Script Class");

        imgui_PopupContextItem("Find Script Class") {
            auto classes = ScriptEngine::getClasses();
            constexpr float filterWidth = 180.f;
            ImGuiTextFilter textFilter;
            textFilter.Draw("Classes", filterWidth);
            imgui_ListBox("##ListBox") {
                for (const auto& [name, Class] : classes) {
                    if (!textFilter.PassFilter(name.c_str()))
                        continue;
                    const bool is_selected = (name == currItem);
                    ImGui::Selectable(name.c_str(), is_selected);
                    if (ImGui::IsItemClicked()) {
                        currItem = name;
                        component.name = currItem;
                    }
                }
            }
        }

        bool isSceneRunning = m_interactor -> isRunning();

        if (isSceneRunning) {
            auto scriptInstance =
                ScriptEngine::getEntityScriptInstance(entity.getComponent<IDComponent>().ID);
            if(!scriptInstance)
                return;
            const auto& fields = scriptInstance->getClassWrapper()->getFields();
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
            }
        }
        else {
            if (!hasScriptClass)
                return;

            processScriptComponent(entity, component);
        }
    }

    void InspectorPanel::processScriptComponent(SceneEntity entity, ScriptComponent& component) {
        auto entityClass = ScriptEngine::getEntityClass(component.name);
        const auto& fields = entityClass -> getFields();

        /// TODO(a.raag): moving to sceneGraph
        auto& entityFields = ScriptEngine::getScriptFieldMap(SceneEntity{std::move(entity)});

        for (const auto& [name, field] : fields) {
            bool hasField = (entityFields.find(name) != entityFields.end());
            ScriptFieldInstance& scriptField = entityFields.at(name);

            switch(field.Type) {
                default:
                    break;
                case ScriptFieldType::Float: {
                    if(hasField) {
                        float data = scriptField.getValue<float>();
                        if (ImGui::DragFloat(name.c_str(), &data))
                            scriptField.setValue(data);
                    }
                    else {
                        float data = 0.0f;
                        if (ImGui::DragFloat(name.c_str(), &data))
                        {
                            ScriptFieldInstance& fieldInstance = entityFields[name];
                            fieldInstance.Field = field;
                            fieldInstance.setValue(data);
                        }
                    }

                    break;
                }
                case ScriptFieldType::Int: {
                    if(hasField) {
                        int data = scriptField.getValue<int>();
                        if (ImGui::DragInt(name.c_str(), &data))
                            scriptField.setValue(data);
                    }
                    else {
                        /// TODO(a.raag): add logic
                    }

                    break;
                }
                case ScriptFieldType::Transform: {
                    std::string resultButtonText = "None";
                    if(hasField) {
                        auto uuid = scriptField.getValue<UUID>();
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
                                if(hasField)
                                    scriptField.setValue(payloadEntity.getComponent<IDComponent>().ID);
                                else {
                                    ScriptFieldInstance& fieldInstance = entityFields[name];
                                    fieldInstance.Field = field;
                                    fieldInstance.setValue(payloadEntity.getComponent<IDComponent>().ID);
                                }
                            }
                        }
                    }
                    break;
                }
                case ScriptFieldType::Entity: {
                    std::string resultButtonText = "None";
                    if(hasField) {
                        auto uuid = scriptField.getValue<UUID>();
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
                            auto realPrefabPath = combinePath(m_interactor->getAssociatedProjectPath(),
                                                              prefabPath.string());

                            /// TODO(a.raag): findPrefab remove when test loadPrefab Correctness
                            Prefab::Ptr prefab { nullptr };
                            if(auto findPrefab = m_prefabManager.findPrefab(realPrefabPath))
                                prefab = findPrefab;
                            else if(auto loadPrefab = m_prefabManager.loadPrefab(m_interactor, realPrefabPath))
                                prefab = loadPrefab;

                            if(!prefab)
                                break;

                            SceneEntity duplicateEntity =
                                    m_interactor -> duplicateEmptyEntity(prefab -> entity);
                            if (duplicateEntity && duplicateEntity != entity) {
                                if(hasField)
                                    scriptField.setValue(duplicateEntity.getComponent<IDComponent>().ID);
                                else {
                                    ScriptFieldInstance& fieldInstance = entityFields[name];
                                    fieldInstance.Field = field;
                                    scriptField.setValue(duplicateEntity.getComponent<IDComponent>().ID);
                                }
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

        auto klasses = ScriptEngine::getClasses();
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
        }

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
            m_selectedEntity = prefab -> entity;
        }
    }

    void InspectorPanel::onPanelEntityNeedSelect(const PanelEntitySelectedMessage& message) {
        if (m_inspectType == InspectType::AssetPrefab && m_prefabHasModification) {
            if (m_selectedEntity.hasComponent<PrefabComponent>()) {
                auto& prefabComponent = m_selectedEntity.getComponent<PrefabComponent>();
                m_prefabManager.savePrefab(prefabComponent.prefabUUID);
                m_prefabHasModification = false;
                auto* msg =
                    m_messageBus.postMessage<PrefabAssetModificatedMessage>(MessageID::PrefabAssetModificated);
                msg->prefabUUID = prefabComponent.prefabUUID;
                msg->prefabEntity = m_selectedEntity;
            }
        }

        m_inspectType = InspectType::EditorEntity;
        m_selectedEntity = message.entity;
    }

    void InspectorPanel::onPanelEntitySelected(const PanelEntitySelectedMessage& message) {
        if (m_inspectType == InspectType::AssetPrefab && m_prefabHasModification) {
            if (m_selectedEntity.hasComponent<PrefabComponent>()) {
                auto& prefabComponent = m_selectedEntity.getComponent<PrefabComponent>();
                m_prefabManager.savePrefab(prefabComponent.prefabUUID);
                m_prefabHasModification = false;
                auto* msg =
                    m_messageBus.postMessage<PrefabAssetModificatedMessage>(MessageID::PrefabAssetModificated);
                msg->prefabUUID = prefabComponent.prefabUUID;
                msg->prefabEntity = m_selectedEntity;
            }
        }
        m_inspectType = InspectType::EditorEntity;
        m_selectedEntity = message.entity;
    }


}