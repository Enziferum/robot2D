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

#include <robot2D/Graphics/View.hpp>
#include <robot2D/Core/Event.hpp>

namespace editor {

    struct SceneCameraConfiguration {
        float zoomOffset = 1.F;
        float zoomMultiplier = 0.1F;
    };

    class SceneCamera {
    public:
        SceneCamera();
        ~SceneCamera() = default;

        void onEvent(const robot2D::Event& event);
        void resize(const robot2D::FloatRect& viewPort);

        float& getCameraSpeed() { return m_cameraSpeed; }
        float getZoom() const;

        robot2D::View& getView();
        const robot2D::View& getView() const;
    private:
        robot2D::View m_view;
        robot2D::FloatRect m_sizeRect;
        SceneCameraConfiguration m_configuration;
        float m_zoom;
        float m_cameraSpeed = 10.F;
    };
}
