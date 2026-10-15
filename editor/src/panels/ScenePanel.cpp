/*********************************************************************
(c) Alex Raag 2026
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

#include <filesystem>

#include <robot2D/imgui/Api.hpp>
#include <editor/panels/ScenePanel.hpp>
#include <editor/Components.hpp>
#include <editor/Messages.hpp>
#include <editor/Macro.hpp>
#include <editor/FileApi.hpp>
#include <editor/DragDropIDS.hpp>

#include "../IconsFontsAwesome5.hpp"

namespace editor {

    ScenePanel::ScenePanel(robot2D::MessageBus& messageBus,
                           MessageDispatcher& messageDispatcher,
                           PrefabManager& prefabManager):
    IPanel(UniqueType(typeid(ScenePanel))),
    m_messageBus{messageBus},
    m_prefabManager{prefabManager},
    m_treeHierarchy("MainScene")
    {
        m_treeHierarchy.setSelectionCallback(BIND_CLASS_FN(onSelectionChanged));
        m_treeHierarchy.setReorderCallback(BIND_CLASS_FN(onReorderEntities));
        m_treeHierarchy.setMakeChildCallback(BIND_CLASS_FN(onMakeChildEntities));
    }

    void ScenePanel::render() {
        if(!m_interactor)
            return;

        robot2D::WindowOptions windowOptions{};
        windowOptions.flagsMask = 0;
        windowOptions.name = ICON_FA_EYE + std::string{" Scene"} + "###ScenePanel";
        renderWindow(windowOptions, BIND_CLASS_FN(windowFunction));
    }

    void ScenePanel::windowFunction() {
        ImGui::BeginDisabled(m_interactor -> isRunning());
        imgui_PopupContextWindow("ScenePanelEntityCreation",
                                 m_configuration.rightMouseButton) {
            imgui_MenuItem("Create Entity") {
                m_interactor -> addEmptyEntity();
                selectEntity(m_interactor -> getEntities().back());
            }
            imgui_Menu("Create UI Elemement") {
                imgui_MenuItem("Button") {
                    selectEntity(m_interactor -> addButton());
                }
            }
        }
        ImGui::EndDisabled();

        m_treeHierarchy.render();

        robot2D::DragDropTarget dragDropTarget{contentPrefabItemID};
        if(auto&& payloadBuffer = dragDropTarget.unpackPayload2Buffer()) {
            if(m_interactor -> isRunning())
                return;
            auto&& path = payloadBuffer.unpack<std::string>();
            std::filesystem::path prefabPath = std::filesystem::path("assets") / path;
            auto realPrefabPath = combinePath(m_interactor -> getAssociatedProjectPath(),
                                              prefabPath.string());
            if(auto prefab = m_prefabManager.loadPrefab(m_interactor, realPrefabPath))
                selectEntity(m_interactor -> duplicateEmptyEntity(prefab -> getEntity()));
        }
    }

    void ScenePanel::setInteractor(UIInteractor::Ptr interactor) {
        m_interactor = interactor;
        m_treeHierarchy.clear();
        if(!m_interactor) {
            m_treeHierarchy.setEntitySource({});
            return;
        }
        m_treeHierarchy.setEntitySource([this]() -> const std::list<SceneEntity>& {
            return m_interactor -> getEntities();
        });
        m_treeHierarchy.setSelectionService(m_interactor -> getSelectionService());
    }

    SceneEntity ScenePanel::getTreeItem(UUID uuid) {
        return m_interactor ? m_interactor -> getEntity(uuid) : SceneEntity{};
    }

    void ScenePanel::selectEntity(SceneEntity entity) {
        if(!entity)
            return;
        m_interactor -> getSelectionService().selectEntity(entity.getUUID());
        onSelectionChanged();
    }

    void ScenePanel::clearSelection() {
        if(m_interactor)
            m_treeHierarchy.clearSelection();
    }

    void ScenePanel::onSelectionChanged() {
        if(!m_interactor)
            return;
        auto active = m_interactor -> getSelectionService().lastActive();
        auto* msg = m_messageBus.postMessage<PanelEntitySelectedMessage>(MessageID::PanelEntitySelected);
        msg -> entity = active ? m_interactor -> getEntity(*active) : SceneEntity{};
    }

    void ScenePanel::onReorderEntities(UUID source, UUID target) {
        if(!m_interactor || m_interactor -> isRunning())
            return;
        auto entity = m_interactor -> getEntity(source);
        auto sibling = m_interactor -> getEntity(target);
        if(!entity || !sibling || sibling.isChild())
            return;
        if(entity.isChild() && !m_interactor -> reparentEntity(entity))
            return;
        if(m_interactor -> setBefore(entity, sibling) && entity.hasComponent<DrawableComponent>())
            entity.getComponent<DrawableComponent>().setReorderZBuffer(true);
    }

    void ScenePanel::onMakeChildEntities(UUID source, UUID target) {
        if(!m_interactor || m_interactor -> isRunning())
            return;
        auto entity = m_interactor -> getEntity(source);
        auto parent = m_interactor -> getEntity(target);
        if(entity && parent && m_interactor -> reparentEntity(entity, parent)
           && entity.hasComponent<DrawableComponent>())
            entity.getComponent<DrawableComponent>().setReorderZBuffer(true);
    }
}
