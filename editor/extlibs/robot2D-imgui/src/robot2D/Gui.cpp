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

#include <robot2D/imgui/Gui.hpp>
#include "GuiImpl.hpp"

namespace robot2D {

    Gui::Gui()
    {
        m_impl = std::make_unique<priv::GuiImpl>();
        if(!m_impl) {
            // TODO(a.raag): fatal error
        }
    }

    Gui::~Gui() = default;

    void Gui::setup(robot2D::Window& window,
                    const std::string& customFontPath, std::vector<std::string>&& customIconsPaths) {
        m_impl -> setup(window, customFontPath, std::move(customIconsPaths));
    }

    void Gui::handleEvents(const robot2D::Event& event) {
        m_impl -> handleEvents(event);
    }

    void Gui::update(float dt) {
        m_impl -> update(dt);
    }

    void Gui::render() {
        m_impl -> render();
    }

} // namespace robot2D