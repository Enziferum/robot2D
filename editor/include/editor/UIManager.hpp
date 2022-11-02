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

#pragma once
#include <vector>
#include <algorithm>
#include <cassert>

#include <robot2D/Ecs/Entity.hpp>
#include "robot2D/Extra/Gui.hpp"
#include "IPanel.hpp"


namespace editor {

    class IUIManager {
    public:
        virtual ~IUIManager() = 0;
        virtual void blockEvents(bool flag) = 0;
        virtual robot2D::ecs::Entity getSelectedEntity()= 0;
    };

    class UIManager final: public IUIManager {
    public:
        UIManager(ImGui::Gui& gui);
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

        void blockEvents(bool flag) override;
        robot2D::ecs::Entity getSelectedEntity() override;
    private:
        void dockingCanvas();
    private:
        ImGui::Gui& m_gui;
        std::vector<IPanel::Ptr> m_panels;
    };

    template<typename T, typename ...Args>
    T& UIManager::addPanel(Args&& ...args) {
        static_assert(std::is_base_of<IPanel, T>::value && "Adding T, must be IPanel child");
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



}
