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

#include <robot2D/Graphics/Graphics.hpp>
#include <sandbox/Render2DScene.hpp>
#include <sandbox/Systems.hpp>
#include <sandbox/Components.hpp>

namespace {
    const robot2D::vec2f position = robot2D::vec2f {50.F, 50.F};
    const robot2D::vec2f size = robot2D::vec2f {100.F, 100.F};
    constexpr char* texturePath = "awesomeface.png";
    constexpr unsigned startEntitiesCount = 5;
}

Render2DScene::Render2DScene(robot2D::RenderWindow& window) : Scene(window),
                                                              m_scene(messageBus) {

}

void Render2DScene::setup() {
    ///// setup Ecs /////
    m_scene.addSystem<RenderSystem>(messageBus);
    m_scene.addSystem<DemoMoveSystem>(messageBus);

    ///// setup Ecs /////

    m_textures.loadFromFile(ResourceID::Face, texturePath);

    for(auto it = 0; it < startEntitiesCount; ++it)
        createEntity({position.x, position.y + size.x * it});
}

void Render2DScene::handleEvents(const robot2D::Event& event) {
    if(event.type == robot2D::Event::Resized) {
        RB_INFO("New Size = {0} and {1}", event.size.widht, event.size.heigth);
        m_window.resize({event.size.widht, event.size.heigth});
        m_window.setView(robot2D::View(robot2D::FloatRect{0, 0, event.size.widht, event.size.heigth}));

        createEntity({position.x, position.y + size.x});
    }
}

void Render2DScene::update(float dt) {
    m_scene.update(dt);
}

void Render2DScene::imGuiRender() {

}

void Render2DScene::render() {
    m_window.beforeRender();
    m_window.draw(m_scene);
    m_window.afterRender();
    m_window.flushRender();
}

void Render2DScene::createEntity(const robot2D::vec2f& position) {
    robot2D::ecs::Entity entity = m_scene.createEntity();

    auto& transform = entity.addComponent<TransformComponent>();

    transform.setPosition({position.x, position.y} );
    transform.scale({size});

    auto& sprite = entity.addComponent<SpriteComponent>();
    sprite.setTexture(m_textures.get(ResourceID::Face));
}
