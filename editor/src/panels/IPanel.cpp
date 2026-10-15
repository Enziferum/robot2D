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

#include <editor/panels/IPanel.hpp>

#include <editor/EditorConfig.hpp>
#include <robot2D/imgui/Api.hpp>

namespace editor {
    IPanel::IPanel(UniqueType uniqueType): m_id(uniqueType) {}

    IPanel::~IPanel() = default;

    void IPanel::update(float dt) {}

    PanelState IPanel::getState() const {
        if(!m_panelKey.empty()) {
            const auto& visibility = EditorConfig::getConfig().panelVisibility;
            auto found = visibility.find(m_panelKey);
            if(found != visibility.end())
                return found -> second ? PanelState::Visible : PanelState::Closed;
        }
        return m_state;
    }
    
    void IPanel::setState(const PanelState& panelState) {
        if(getState() == panelState)
            return;
        m_state = panelState;
        if(!m_panelKey.empty()) {
            EditorConfig::getConfig().panelVisibility[m_panelKey] = panelState == PanelState::Visible;
            m_focusRequested = panelState == PanelState::Visible;
            if(ImGui::GetCurrentContext())
                ImGui::GetIO().WantSaveIniSettings = true;
        }
    }

    void IPanel::requestFocus() {
        m_focusRequested = true;
    }

    void IPanel::renderWindow(const robot2D::WindowOptions& options, std::function<void()>&& content) {
        if(getState() == PanelState::Closed)
            return;
        options.enable();
        robot2D::Defer<std::function<void()>> disableStyles{[&options]() { options.disable(); }};
        if(m_focusRequested) {
            ImGui::SetNextWindowFocus();
            m_focusRequested = false;
        }
        bool open = true;
        bool visible = ImGui::Begin(options.name.c_str(), m_panelKey.empty() ? nullptr : &open, options.flagsMask);
        robot2D::Defer endWindow{&ImGui::End};
        if(!open)
            setState(PanelState::Closed);
        if(visible && open)
            content();
    }

}
