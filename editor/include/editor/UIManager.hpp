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

#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <cassert>

#include <editor/SceneEntity.hpp>
#include <robot2D/imgui/Gui.hpp>
#include <robot2D/Core/MessageBus.hpp>
#include "panels/IPanel.hpp"

#include "Uuid.hpp"

namespace editor {

    class UIInteractor;

    class IUIManager {
    public:
        virtual ~IUIManager() = 0;
        virtual void blockEvents(bool flag) = 0;
        virtual SceneEntity getTreeItem(UUID uuid)  = 0;
    };

    class UIManager final: public IUIManager {
    public:
        UIManager(robot2D::Gui& gui);
        UIManager(const UIManager&) = delete;
        UIManager& operator=(const UIManager&) = delete;
        UIManager(UIManager&&) = delete;
        UIManager& operator=(UIManager&&) = delete;
        ~UIManager() override = default;



        template<typename T, typename ...Args>
        T& addPanel(Args&& ...args);

        template<typename T>
        T& getPanel();

        void update(float dt);
        void render();
        void setRuntimeControls(UIInteractor* interactor, robot2D::MessageBus& messageBus);

        struct PanelEntry {
            std::string key;
            std::string name;
            std::string windowName;
            IPanel* panel;
        };
        bool registerPanel(IPanel& panel, const std::string& key, const std::string& name,
                           const std::string& windowName = {});
        const std::vector<PanelEntry>& getPanels() const { return m_registeredPanels; }
        bool setPanelVisible(const std::string& key, bool visible);
        void showAllPanels();
        void restoreDefaultLayout();
        void renderDiagnostics() const;

        void blockEvents(bool flag) override;
        SceneEntity getTreeItem(editor::UUID uuid) override;
    private:
        void renderBaseCanvas();
        void canvasWindowFunction();
        void renderRuntimeToolbar();

        void createLayout();
        bool isStatusBarVisible() const;
    private:
        robot2D::Gui& m_gui;
        UIInteractor* m_runtimeInteractor { nullptr };
        robot2D::MessageBus* m_runtimeMessageBus { nullptr };
        std::vector<IPanel::Ptr> m_panels;
        std::vector<PanelEntry> m_registeredPanels;
        bool m_resetLayout { false };
        unsigned int m_dockspaceID { 0 };
    };

    template<typename T, typename ...Args>
    T& UIManager::addPanel(Args&& ...args) {
        static_assert(std::is_base_of_v<IPanel, T> && "T must be IPanel inherit");
        m_panels.emplace_back(std::make_shared<T>(std::forward<Args>(args)...));
        return *(dynamic_cast<T*>(m_panels.back().get()));
    }

    template<typename T>
    T& UIManager::getPanel() {
        UniqueType uniqueType(typeid(T));
        auto found = std::find_if(m_panels.begin(), m_panels.end(), [uniqueType](const IPanel::Ptr& ptr) {
            return ptr -> getID() == uniqueType;
        });

        assert(found != m_panels.end() && "Panel must be added before using");

        return *(static_cast<T*>(found -> get()));
    }



} // namespace editor
