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

#include <cstring>
#include <filesystem>

#include <robot2D/imgui/Api.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>


#include <editor/panels/ScenePanel.hpp>
#include <editor/Components.hpp>
#include <editor/Messages.hpp>

#include <editor/scripting/ScriptingEngine.hpp>
#include <editor/scripting/ScriptEngineData.hpp>

#include <editor/panels/ScenePanelUI.hpp>
#include <editor/Macro.hpp>

#include <editor/TaskQueue.hpp>
#include <editor/async/ImageLoadTask.hpp>
#include <editor/async/FontLoadTask.hpp>
#include <editor/FileApi.hpp>
#include <editor/ResouceManager.hpp>

namespace editor {

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



    ScenePanel::ScenePanel(MessageDispatcher& messageDispatcher):
    IPanel(UniqueType(typeid(ScenePanel))),
    m_messageDispatcher{messageDispatcher},
    m_scene(nullptr),
    m_selectedEntity{},
    m_configuration{},
    m_treeHierarchy("MainScene")
    {
        m_messageDispatcher.onMessage<EntitySelection>(
                EntitySelected,
                BIND_CLASS_FN(onEntitySelection)
        );


        setupTreeHierarchy();
    }

    void ScenePanel::render() {
        if(m_scene == nullptr)
            return;

        robot2D::WindowOptions windowOptions{};
        windowOptions.name = "ScenePanel";
        robot2D::createWindow(windowOptions, BIND_CLASS_FN(windowFunction));


        ImGui::Begin("Properties");
        if (m_selectedEntity)
        {
            drawComponentsBase(m_selectedEntity);
        }

        ImGui::End();
    }

    void ScenePanel::drawEntity(robot2D::ecs::Entity entity) {
        auto& tag = entity.getComponent<TagComponent>().getTag();

        /// SceneHieherocy
        /// 1. Make Replacement onto scene
        /// 2. Always Add MainCamera As Start
        /// 3. Don't SubItems while it doesn't need
        /// 4. Make ScenePanelUI for controls and etc



        /*
        ImGuiTreeNodeFlags flags = ((m_selectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0)
                    | ImGuiTreeNodeFlags_OpenOnArrow;
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
         */
    }

    void ScenePanel::drawComponentsBase(robot2D::ecs::Entity entity) {
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

        ImGui::Text("UUID: %llu", entity.getComponent<IDComponent>().ID);

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
                if (!m_selectedEntity.hasComponent<DrawableComponent>())
                    m_selectedEntity.addComponent<DrawableComponent>();
                else
                    RB_EDITOR_WARN("This entity already has the Sprite Renderer Component!");
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

            ImGui::EndPopup();
        }

        ImGui::PopItemWidth();

        drawComponents(entity);
    }

    void ScenePanel::drawComponents(robot2D::ecs::Entity entity) {
        drawComponent<TransformComponent>("Transform", entity, [](auto& component)
        {
            ui::drawVec2Control("Translation", component.getPosition());
            ui::drawVec2Control("Scale", component.getScale(), 1.0f);
            //DrawVec2Control("Rotation", component.getRotation(), 0.f);
        });

        drawComponent<CameraComponent>("Camera", entity, [](auto& component)
        {
            ImGui::Checkbox("Primary", &component.isPrimary);
        });

        drawComponent<DrawableComponent>("Drawable", entity, [this, &entity](auto& component)
        {
            // TODO(a.raag): fix color getting
            auto color = component.getColor().toGL();
            float colors[4];
            colors[0] = color.red; colors[1] = color.green;
            colors[2] = color.blue; colors[3] = color.alpha;
            ImGui::ColorEdit4("Color", colors);

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
                    auto texturePath = combinePath(m_scene -> getAssociatedProjectPath(), localPath.string());

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

        drawComponent<ScriptComponent>("Scripting", entity, [entity, scene = m_scene](auto& component) {
            bool hasScriptClass = ScriptEngine::hasEntityClass(component.name);

            static char buffer[64];
            strcpy(buffer, component.name.c_str());

            ImGui::Text("EntityID = %i", entity.getIndex());

            if(!hasScriptClass)
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.2f, 0.3f, 1.0f));

            if(ImGui::InputText("Class", buffer, sizeof(buffer)))
                component.name = buffer;

            bool isSceneRunning = scene -> isRunning();

            if(isSceneRunning) {
                auto scriptInstance = ScriptEngine::getEntityScriptInstance(entity.getIndex());
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
                        }
                    }
                }
            }


            if(!hasScriptClass)
                ImGui::PopStyleColor();
        });

        drawComponent<Physics2DComponent>("physics2D", entity, [](auto& component) {
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

        drawComponent<Collider2DComponent>("Collider2D", entity, [](auto& component) {
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


        drawComponent<TextComponent>("Text", entity, [this, entity, scene = m_scene](auto& component) {
            ImGui::Text(component.getText().c_str());
            ImGui::Button("Font", ImVec2(100.0f, 0.0f));
            if (ImGui::BeginDragDropTarget())
            {
                // all string types make as Parameters somewhere

                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                {
                    const wchar_t* path = (const wchar_t*)payload->Data;
                    std::filesystem::path localPath = std::filesystem::path("assets") / path;
                    auto texturePath = combinePath(scene -> getAssociatedProjectPath(), localPath.string());

                    auto queue = TaskQueue::GetQueue();
//                    queue -> template addAsyncTask<FontLoadTask>([this](const FontLoadTask& task) {
//                        this -> onLoadFont(task.getFont(), task.getEntity());
//                    }, texturePath, entity);
                }
                ImGui::EndDragDropTarget();
            }

        });
    }

    void ScenePanel::onEntitySelection(const EntitySelection& entitySelection) {
        for(const auto& entity: m_scene -> getEntities()) {
            if(entity.getIndex() == entitySelection.entityID) {
                m_selectedEntity = entity;
                break;
            }
        }
    }

    void ScenePanel::windowFunction() {
        m_treeHierarchy.render();

        if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
            m_selectedEntity = {};

        // Right-click on blank space
        if (ImGui::BeginPopupContextWindow(0, m_configuration.rightMouseButton, false))
        {
            if (ImGui::MenuItem("Create Empty Entity")) {
                m_scene -> addEmptyEntity();
                auto item = m_treeHierarchy.addItem<robot2D::ecs::Entity>();
                m_selectedEntity = m_scene -> getEntities().back();
                item -> setName(m_scene -> getEntities().back().getComponent<TagComponent>().getTag());
                item -> setUserData(m_selectedEntity);
            }

            ImGui::EndPopup();
        }

    }

    void ScenePanel::setupTreeHierarchy() {

        m_treeHierarchy.addOnSelectCallback([this](ITreeItem::Ptr selectedItem) {
            auto entity = selectedItem -> getUserData<robot2D::ecs::Entity>();
            if(entity) {
                m_selectedEntity = *entity;
            }
            else {
                RB_EDITOR_ERROR("Selected item in Hierarchy don't have userData");
            }
        });

        m_treeHierarchy.addOnCallback([this](ITreeItem::Ptr item) {

            bool deleteSelected = false;
            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::MenuItem("Delete Entity"))
                    deleteSelected = true;

                ImGui::EndPopup();
            }

            if(deleteSelected) {
                if(item -> isChild()) {
                    auto entity = item -> getUserData<robot2D::ecs::Entity>();
                    if(entity) {
                        auto& transform = entity -> getComponent<TransformComponent>();
                        if(transform.isChild())
                            transform.removeSelf();
                    }
                    item -> removeSelf();
                }
                else {
                    if(item -> hasChildrens()) {
                        for(auto child: item -> getChildrens()) {
                            auto entity = child -> getUserData<robot2D::ecs::Entity>();
                            m_scene -> removeEntity(std::move(*entity));
                            item -> deleteChild(child);
                        }
                        auto entity = item -> getUserData<robot2D::ecs::Entity>();
                        m_scene -> removeEntity(std::move(*entity));
                        m_treeHierarchy.deleteItem(item);
                    }
                    else {
                        auto entity = item -> getUserData<robot2D::ecs::Entity>();
                        if(!entity) {
                            RB_EDITOR_ERROR("Cant in Hierarchy don't have userData");
                            return;
                        }
                        m_scene -> removeEntity(*entity);
                        m_treeHierarchy.deleteItem(item);
                    }
                }
                m_selectedEntity = {};
            }
        });

        m_treeHierarchy.addOnReorderCallback([this](ITreeItem::Ptr source, ITreeItem::Ptr target){
            auto sourceEntity = source -> getUserData<robot2D::ecs::Entity>();
            if(!sourceEntity) {
                RB_EDITOR_ERROR("Can't item in Hierarchy don't have userData");
                return;
            }

            if(sourceEntity -> hasComponent<DrawableComponent>()) {
                auto& drawable = sourceEntity -> getComponent<DrawableComponent>();
                drawable.setReorderZBuffer(true);
            }

            m_treeHierarchy.setBefore(source, target);
        });

        m_treeHierarchy.addOnMakeChildCallback([this](ITreeItem::Ptr source, ITreeItem::Ptr intoTarget) {
           auto entity = intoTarget -> getUserData<robot2D::ecs::Entity>();
           if(!entity) {
               RB_EDITOR_ERROR("Can't item in Hierarchy don't have userData");
               return;
           }
           auto& transform = entity -> getComponent<TransformComponent>();
           auto sourceEntity = source -> getUserData<robot2D::ecs::Entity>();
           if(!sourceEntity) {
               RB_EDITOR_ERROR("Can't item in Hierarchy don't have userData");
               return;
           }

           if(sourceEntity -> hasComponent<DrawableComponent>()) {
               auto& sprite = sourceEntity -> getComponent<DrawableComponent>();
               sprite.setReorderZBuffer(true);
           }

           transform.addChild(*sourceEntity);
           m_scene -> removeEntityChild(*sourceEntity);
           m_treeHierarchy.deleteItem(source);
           intoTarget -> addChild(source);
        });

    }

    void ScenePanel::onLoadImage(const robot2D::Image& image, robot2D::ecs::Entity entity) {
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

    void ScenePanel::setActiveScene(Scene::Ptr ptr) {
        m_scene = ptr;
        m_selectedEntity = {};
        for(auto& entity: m_scene -> getEntities()) {
            auto item = m_treeHierarchy.addItem<robot2D::ecs::Entity>();
            item -> setName(entity.getComponent<TagComponent>().getTag());
            item -> setUserData(entity);

            auto& transform = entity.getComponent<TransformComponent>();
            if(transform.hasChildren()) {
                for(auto& child: transform.getChildren()) {
                    auto childItem = item -> addChild();
                    childItem -> setName(child.getComponent<TagComponent>().getTag());
                    childItem -> setUserData(child);
                }
            }
        }

    }

    void ScenePanel::onLoadFont(const robot2D::Image& image, robot2D::ecs::Entity entity) {
        if(entity.destroyed()) {
            RB_EDITOR_WARN("Can't attach texture to Entity, because it's already destroyed");
            return;
        }

        auto t = m_fonts.add(entity.getIndex());
        if(t)
            t -> create(image);

        if(entity.hasComponent<TextComponent>()) {
           // entity.getComponent<TextComponent>().setFont();
        }
    }

}