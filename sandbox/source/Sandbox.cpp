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

#include <sandbox/Sandbox.hpp>
/// Scenes ///
#include <sandbox/Render2DScene.hpp>
#include <sandbox/LayerScene.hpp>
/// Scenes ///

Sandbox::Sandbox():
robot2D::Application(),
m_scene(nullptr)
{
}

void Sandbox::setup() {
    //// Put own scenes here /////
    Render2DScene::Ptr render2DScene = std::make_unique<Render2DScene>(*m_window);
    render2DScene -> setup();

    LayerScene::Ptr  layerScene = std::make_unique<LayerScene>(*m_window);
    layerScene -> setup();
    //// Put own scenes here /////

    m_scene = std::move(render2DScene);
}

void Sandbox::handleEvents(const robot2D::Event& event) {
    if(event.type == robot2D::Event::KeyPressed &&
       event.key.code == robot2D::Key::ESCAPE)
        m_running = false;
    m_scene -> handleEvents(event);
}

void Sandbox::update(float dt) {
    m_scene -> update(dt);
}

void Sandbox::render() {
    m_window -> clear();
    m_scene -> render();
    m_window -> display();
}
