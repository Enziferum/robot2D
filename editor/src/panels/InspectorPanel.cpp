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

#include <editor/async/ImageLoadTask.hpp>
#include <editor/async/FontLoadTask.hpp>

namespace editor {

    template<typename T, typename UIFunction>
    static void drawComponent(const std::string& name, robot2D::ecs::Entity& entity, UIFunction uiFunction)
    {
        if(entity.destroyed())
            return;
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
        m_messageDispatcher.onMessage<PrefabAssetPressedMessage>(MessageID::PrefabAssetPressed,
                                                                 BIND_CLASS_FN(onPrefabAssetSelected));

        m_messageDispatcher.onMessage<PanelEntitySelectedMessage>(
                MessageID::PanelEntityNeedSelect,
                [this](const auto& message) {
                    if(m_inspectType == InspectType::AssetPrefab && m_prefabHasModification) {
                        if(m_selectedEntity.hasComponent<PrefabComponent>()) {
                            auto& prefabComponent = m_selectedEntity.getComponent<PrefabComponent>();
                            m_prefabManager.savePrefab(prefabComponent.prefabUUID);
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
        );

        m_messageDispatcher.onMessage<PanelEntitySelectedMessage>(
                MessageID::PanelEntitySelected,
                [this](const auto& message) {
                    if(m_inspectType == InspectType::AssetPrefab && m_prefabHasModification) {
                        if(m_selectedEntity.hasComponent<PrefabComponent>()) {
                            auto& prefabComponent = m_selectedEntity.getComponent<PrefabComponent>();
                            m_prefabManager.savePrefab(prefabComponent.prefabUUID);
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
        );
    }


    void InspectorPanel::setInteractor(UIInteractor::Ptr interactor) {
        m_interactor = interactor;
    }

    void InspectorPanel::render() {
        robot2D::WindowOptions propertiesWindowOptions{};
        propertiesWindowOptions.name = "Inspector";

        robot2D::createWindow(propertiesWindowOptions, [this]{
            if(m_selectedEntity && !m_selectedEntity.destroyed() && m_inspectType == InspectType::EditorEntity)
                drawComponentsBase(m_selectedEntity);
            else if(m_inspectType != InspectType::EditorEntity)
                drawAssetBase();
        });
    }

    void InspectorPanel::drawComponentsBase(robot2D::ecs::Entity entity, bool isEntity) {
        if(isEntity) {
            if(!entity.hasComponent<TagComponent>() || entity.destroyed())
                return;

            auto tag = entity.getComponent<TagComponent>().getTag();

            /// TODO(a.raag): robot2D-imgui Api's Text using
            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            std::strncpy(buffer, tag.c_str(), sizeof(buffer));

            if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
            {
                std::string sbuffer = std::string(buffer);
                if(sbuffer.empty())
                    sbuffer = "Untitled Entity";
                entity.getComponent<TagComponent>().setTag(sbuffer);
            }

            ImGui::Text("UUID: %llu", entity.getComponent<IDComponent>().ID);
        }
        else {
            if(!entity.hasComponent<PrefabComponent>() || entity.destroyed())
                return;
            ImGui::Text("Prefab UUID: %llu", entity.getComponent<PrefabComponent>().prefabUUID);
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

            if(ImGui::MenuItem("UtilRender")) {
                if(!m_selectedEntity.hasComponent<DrawableComponent>())
                    m_selectedEntity.addComponent<DrawableComponent>().isUtil = true;
                else
                    RB_EDITOR_WARN("This entity already has the Sprite Renderer Component!");
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::MenuItem("SpriteRender"))
            {
                if (!m_selectedEntity.hasComponent<DrawableComponent>())
                    m_selectedEntity.addComponent<DrawableComponent>();
                else {
                    RB_EDITOR_WARN("This entity already has the Sprite Renderer Component!");
                }

                ImGui::CloseCurrentPopup();
            }

            if (ImGui::MenuItem("RigidBody2D"))
            {
                if (!m_selectedEntity.hasComponent<Physics2DComponent>())
                    m_selectedEntity.addComponent<Physics2DComponent>();
                else
                    RB_EDITOR_WARN("This entity already has the Physics2D Component!");
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::MenuItem("Collider2D"))
            {
                if (!m_selectedEntity.hasComponent<Collider2DComponent>())
                    m_selectedEntity.addComponent<Collider2DComponent>();
                else
                    RB_EDITOR_WARN("This entity already has the Collider2D Component!");
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::MenuItem("Scripting"))
            {
                if (!m_selectedEntity.hasComponent<ScriptComponent>())
                    m_selectedEntity.addComponent<ScriptComponent>();
                else
                    RB_EDITOR_WARN("This entity already has the Scripting Component!");
                ImGui::CloseCurrentPopup();
            }

            if (ImGui::MenuItem("Text"))
            {
                if (!m_selectedEntity.hasComponent<TextComponent>()) {
                    m_selectedEntity.addComponent<TextComponent>();
                    m_selectedEntity.getComponent<TransformComponent>().setScale({1.f, 1.f});
                }
                else
                    RB_EDITOR_WARN("This entity already has the Scripting Component!");
                ImGui::CloseCurrentPopup();
            }


            ImGui::EndPopup();
        }

        ImGui::PopItemWidth();

        drawComponents(entity, isEntity);
    }

    void InspectorPanel::drawComponents(robot2D::ecs::Entity entity, bool isEntity) {
        drawComponent<TransformComponent>("Transform", entity, [this, isEntity](auto& component)
        {
            robot2D::vec2f lastPosition = component.getPosition();
            robot2D::vec2f lastScale = component.getScale();
            float lastRotation = component.getRotate();

            ui::drawVec2Control("Translation", component.getPosition());
            ui::drawVec2Control("Scale", component.getScale(), 1.0f);
            ui::drawVec1Control("Rotation", component.getRotate(), 0.f);
            component.setRotate(component.getRotate());

            if(lastPosition != component.getPosition() || lastScale != component.getScale() ||
                                                                    lastRotation != component.getRotate())
                m_prefabHasModification = true;

        });

        drawComponent<CameraComponent>("Camera", entity, [isEntity](auto& component)
        {
            auto& camera = component.camera;
            ImGui::Checkbox("Primary", &component.isPrimary);
            float orthoSize = component.orthoSize;
            if (ImGui::DragFloat("Size", &orthoSize, 0.1))
                component.orthoSize = orthoSize;

            const char* aspectRatioTypeStrings[] = { "16:9", "9:16" };
            const char* aspectRatioProjectionTypeString = aspectRatioTypeStrings[(int)component.aspectRatio];
            if (ImGui::BeginCombo("AspectRatio", aspectRatioProjectionTypeString))
            {
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

                ImGui::EndCombo();
            }

        });

        drawComponent<DrawableComponent>("Drawable", entity, [this, &entity, isEntity](auto& component)
        {
            // TODO(a.raag): fix color getting
            auto color = component.getColor().toGL();
            float colors[4];
            colors[0] = color.red; colors[1] = color.green;
            colors[2] = color.blue; colors[3] = color.alpha;
            ImGui::ColorEdit4("Color", colors);


            if(component.isUtil)
                return;

            component.setColor(robot2D::Color::fromGL(colors[0], colors[1], colors[2], colors[3]));
            int lastDepth = component.getDepth();
            ImGui::InputInt("zDepth", &component.getDepth());
            if(lastDepth != component.getDepth())
                component.setReorderZBuffer(true);

            ImGui::Button("Texture", ImVec2(100.0f, 0.0f));
            if (ImGui::BeginDragDropTarget())
            {
                // all string types make as Parameters somewhere

                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                {
                    const wchar_t* path = (const wchar_t*)payload->Data;
                    std::filesystem::path localPath = std::filesystem::path("assets") / path;
                    auto texturePath = combinePath(m_interactor -> getAssociatedProjectPath(), localPath.string());

                    component.setTexturePath(localPath.string());
                    auto manager = ResourceManager::getManager();
                    if(!manager -> hasImage(localPath.filename().string())) {
                        auto queue = TaskQueue::GetQueue();
                        queue -> template addAsyncTask<ImageLoadTask>([this](const ImageLoadTask& task) {
                            this -> onLoadImage(task.getImage(), task.getEntity());
                        }, texturePath, entity);
                    }
                    else {
                        auto texture = m_textures.template add(entity.getIndex());
                        if(texture) {
                            auto image = manager -> getImage(localPath.filename().string());
                            texture -> create(image);
                            component.setTexture(*texture);
                        }
                    }
                }
                ImGui::EndDragDropTarget();
            }

            if(entity.hasComponent<DrawableComponent>()) {
                auto& spriteComponent = entity.template getComponent<DrawableComponent>();
                if(spriteComponent.hasTexture()) {
                    auto size = spriteComponent.getTexture().getSize();
                    auto colorFormat = spriteComponent.getTexture().getColorFormat();

                    ImGui::Text("Width = %i", size.x);
                    ImGui::SameLine();
                    ImGui::Text("Height = %i", size.y);
                    ImGui::Text("ImageColorFormat = %s", "RGBA");
                }
            }
        });

        drawComponent<ScriptComponent>("Script", entity, [this, entity,
                interactor= m_interactor, isEntity](auto& component) {
            std::string currItem = component.name; // Here we store our selection data as an index.
            bool hasScriptClass = ScriptEngine::hasEntityClass(component.name);

            if(!hasScriptClass)
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.2f, 0.3f, 1.0f));
            ImGui::Selectable(currItem.empty() ? "FindClass" : currItem.c_str());
            if(!hasScriptClass)
                ImGui::PopStyleColor();

            if(ImGui::IsItemClicked())
                ImGui::OpenPopup("Find Script Class");

            if(ImGui::BeginPopupContextItem("Find Script Class"))
            {
                auto classes = ScriptEngine::getClasses();

                ImGuiTextFilter textFilter;
                textFilter.Draw("Classes", 180);
                if (ImGui::BeginListBox("##Listbox"))
                {
                    for(const auto& [name, Class]: classes) {
                        if(!textFilter.PassFilter(name.c_str())) continue;
                        const bool is_selected = (name == currItem);
                        ImGui::Selectable(name.c_str(), is_selected);
                        if(ImGui::IsItemClicked()) {
                            currItem = name;
                            component.name = currItem;
                        }
                    }
                    ImGui::EndListBox();
                }

                ImGui::EndPopup();
            }

            bool isSceneRunning = interactor -> isRunning();

            if(isSceneRunning) {
                auto scriptInstance =
                        ScriptEngine::getEntityScriptInstance(entity.getComponent<IDComponent>().ID);
                if (scriptInstance)
                {
                    const auto& fields = scriptInstance -> getClassWrapper() -> getFields();
                    for (const auto& [name, field] : fields)
                    {
                        if (field.Type == ScriptFieldType::Float)
                        {
                            float data = scriptInstance -> getFieldValue<float>(name);
                            if (ImGui::DragFloat(name.c_str(), &data))
                            {
                                scriptInstance -> setFieldValue(name, data);
                            }
                        }
                    }
                }
            }
            else {
                if(hasScriptClass) {
                    auto entityClass = ScriptEngine::getEntityClass(component.name);
                    const auto& fields = entityClass -> getFields();
                    auto& entityFields = ScriptEngine::getScriptFieldMap(entity);

                    for(const auto& [name, field]: fields) {
                        // Field has been set in editor
                        if (entityFields.find(name) != entityFields.end())
                        {
                            ScriptFieldInstance& scriptField = entityFields.at(name);

                            // Display control to set it maybe
                            if (field.Type == ScriptFieldType::Float)
                            {
                                float data = scriptField.getValue<float>();
                                if (ImGui::DragFloat(name.c_str(), &data))
                                    scriptField.setValue(data);
                            }
                            if(field.Type == ScriptFieldType::Int) {
                                int data = scriptField.getValue<int>();
                                if(ImGui::DragInt(name.c_str(), &data))
                                    scriptField.setValue(data);
                            }
                            if(field.Type == ScriptFieldType::Transform) {
                                ImGui::AlignTextToFramePadding();
                                ImGui::Text(name.c_str());
                                ImGui::SameLine();
                                auto uuid = scriptField.getValue<UUID>();
                                auto preEntity = interactor -> getByUUID(uuid);
                                std::string resultText = "None";
                                if(preEntity)
                                    resultText = preEntity.getComponent<TagComponent>().getTag();
                                ImGui::Button(resultText.c_str());

                                if(ImGui::BeginDragDropTarget()) {
                                    auto* payload = ImGui::AcceptDragDropPayload("TreeNodeItem");
                                    if(payload) {
                                        if(payload -> IsDataType("TreeNodeItem")) {
                                            UUID id = *static_cast<UUID*>(payload -> Data);
                                            auto payloadEntity = m_uiManager.getTreeItem(id);

                                            if(payloadEntity) {
                                                scriptField.setValue(payloadEntity.getComponent<IDComponent>().ID);
                                            }
                                        }
                                    }
                                    ImGui::EndDragDropTarget();
                                }
                            }

                        }
                        else
                        {
                            // Display control to set it maybe
                            if (field.Type == ScriptFieldType::Float)
                            {
                                float data = 0.0f;
                                if (ImGui::DragFloat(name.c_str(), &data))
                                {
                                    ScriptFieldInstance& fieldInstance = entityFields[name];
                                    fieldInstance.Field = field;
                                    fieldInstance.setValue(data);
                                }
                            }
                            if(field.Type == ScriptFieldType::Transform) {
                                ImGui::AlignTextToFramePadding();
                                ImGui::Text(name.c_str());
                                ImGui::SameLine();
                                std::string resultText = "None";
                                ImGui::Button(resultText.c_str());

                                if(ImGui::BeginDragDropTarget()) {
                                    auto* payload = ImGui::AcceptDragDropPayload("TreeNodeItem");
                                    if(payload) {
                                        if(payload -> IsDataType("TreeNodeItem")) {
                                            UUID id = *static_cast<UUID*>(payload -> Data);
                                            auto payloadEntity = m_uiManager.getTreeItem(id);

                                            if(payloadEntity) {
                                                if(payloadEntity != entity) {
                                                    ScriptFieldInstance& fieldInstance = entityFields[name];
                                                    fieldInstance.Field = field;
                                                    fieldInstance.setValue(payloadEntity.getComponent<IDComponent>().ID);
                                                }
                                            }
                                        }
                                    }
                                    ImGui::EndDragDropTarget();
                                }
                            }

                        }
                    }
                }
            }

        });

        drawComponent<Physics2DComponent>("physics2D", entity, [isEntity](auto& component) {
            const char* bodyTypeStrings[] = { "Static", "Dynamic", "Kinematic"};
            const char* currentBodyTypeString = bodyTypeStrings[(int)component.type];
            if (ImGui::BeginCombo("Body Type", currentBodyTypeString))
            {
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

                ImGui::EndCombo();
            }

            ImGui::Checkbox("Fixed Rotation", &component.fixedRotation);
        });

        drawComponent<Collider2DComponent>("Collider2D", entity, [isEntity](auto& component) {
            float offset[2] = { component.offset.x, component.offset.y };
            float size[2] = { component.size.x, component.size.y };
            ImGui::DragFloat2("Offset", offset);
            ImGui::DragFloat2("Size", size);
            ImGui::DragFloat("Density", &component.density, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Friction", &component.friction, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Restitution", &component.restitution, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Restitution Threshold", &component.restitutionThreshold, 0.01f, 0.0f);
            component.offset = {offset[0], offset[1] };
            component.size = { size[0], size[1] };
        });

        drawComponent<TextComponent>("Text", entity, [this, entity, isEntity](auto& component) {
            ImGui::Button("Font", ImVec2(100.0f, 0.0f));
            if (ImGui::BeginDragDropTarget())
            {
                // all string types make as Parameters somewhere

                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                {
                    const wchar_t* path = (const wchar_t*)payload->Data;
                    std::filesystem::path localPath = std::filesystem::path("assets") / path;
                    auto texturePath = combinePath(m_interactor -> getAssociatedProjectPath(), localPath.string());

                    auto queue = TaskQueue::GetQueue();
                    queue -> template addAsyncTask<FontLoadTask>([this](const FontLoadTask& task) {
                        this -> onLoadFont(task.getFont(), task.getEntity());
                    }, texturePath, entity);
                }
                ImGui::EndDragDropTarget();
            }

            if(component.getFont()) {
                robot2D::InputText("##Text", &component.getText(), 0);
                component.setText(component.getText());
            }
        });
    }

    void InspectorPanel::onLoadImage(const robot2D::Image& image, robot2D::ecs::Entity entity) {
        if(entity.destroyed()) {
            RB_EDITOR_WARN("Can't attach texture to Entity, because it's already destroyed");
            return;
        }

        auto t = m_textures.add(entity.getIndex());
        if(t)
            t -> create(image);

        if(entity.hasComponent<DrawableComponent>()) {
            entity.getComponent<DrawableComponent>().setTexture(m_textures[entity.getIndex()]);
        }
    }

    void InspectorPanel::onLoadFont(const robot2D::Font& font, robot2D::ecs::Entity entity) {
        if(entity.destroyed()) {
            RB_EDITOR_WARN("Can't attach texture to Entity, because it's already destroyed");
            return;
        }

        auto f = m_fonts.add(entity.getIndex());
        if(!f)
            return;
        f -> clone(const_cast<robot2D::Font &>(font));

        if(entity.hasComponent<TextComponent>()) {
            entity.getComponent<TextComponent>().setFont(m_fonts[entity.getIndex()]);
        }
    }

    void InspectorPanel::onPrefabAssetSelected(const PrefabAssetPressedMessage& message) {
        m_inspectType = InspectType::AssetPrefab;

        std::filesystem::path localPath = std::filesystem::path("assets") / message.localPath;
        auto fullPath = combinePath(m_interactor -> getAssociatedProjectPath(), localPath.string());

        auto prefab = m_prefabManager.loadPrefab(m_interactor, fullPath);
        if(!prefab) {
            RB_EDITOR_ERROR("InspectorPanel: Can't load prefab by path {0}", fullPath);
        }
        else {
            m_selectedEntity = prefab -> entity;
        }
    }

    void InspectorPanel::clearSelection() {
        if(m_inspectType == InspectType::AssetPrefab && m_prefabHasModification) {
            if(m_selectedEntity.hasComponent<PrefabComponent>()) {
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

    void InspectorPanel::drawAssetBase() {
        switch(m_inspectType) {
            default:
                break;
            case InspectType::AssetPrefab: {
                drawComponentsBase(m_selectedEntity, false);
                break;
            }
            case InspectType::AssetScene: {
                break;
            }
            case InspectType::AssetImage: {
                break;
            }
            case InspectType::AssetFont: {
                break;
            }
        }
    }

    void InspectorPanel::setSelected(robot2D::ecs::Entity entity) {
        m_selectedEntity = entity;
    }


}