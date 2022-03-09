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

#include <editor/SceneCamera.hpp>
#include <robot2D/Util/Logger.hpp>

namespace editor {
    SceneCamera::SceneCamera():
    m_view(),
    m_zoom(0.F),
    m_configuration{} {}

    void SceneCamera::onEvent(const robot2D::Event& event) {
        if(event.type == robot2D::Event::MouseWheel) {
            m_zoom = m_configuration.zoomOffset + event.wheel.scroll_y * m_configuration.zoomMultiplier;
            m_view.zoom(m_zoom);
        }

        if(event.type == robot2D::Event::KeyPressed) {
            if(event.key.code == robot2D::Key::W)
                m_view.move(0, -m_cameraSpeed);
            if(event.key.code == robot2D::Key::S)
                m_view.move(0, m_cameraSpeed);
            if(event.key.code == robot2D::Key::A)
                m_view.move(-m_cameraSpeed, 0);
            if(event.key.code == robot2D::Key::D)
                m_view.move(m_cameraSpeed, 0);
        }
    }

    void SceneCamera::resize(const robot2D::FloatRect& viewPort) {
        m_sizeRect = viewPort;
        m_view.reset(viewPort);
    }

    robot2D::View& SceneCamera::getView() {
        return m_view;
    }

    const robot2D::View &SceneCamera::getView() const {
        return m_view;
    }

    const float SceneCamera::getZoom() const {
        auto actualSize = m_view.getSize();
        robot2D::vec2f defaultSize = {m_sizeRect.width, m_sizeRect.height};
        robot2D::vec2f zoom = {defaultSize.x / actualSize.x, defaultSize.y / actualSize.y};
        return zoom.x;
    }
}