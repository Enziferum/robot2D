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
    const robot2D::vec2f position = robot2D::vec2f {100.F, 100.F};
    const robot2D::vec2f size = robot2D::vec2f {200.F, 200.F};
    constexpr char* texturePath = "awesomeface.png";
    constexpr char* texturePath1 = "old_logo.png";
    constexpr char* texturePath2 = "cityskyline.png";
    constexpr char* texturePath3 = "paddle.png";
    constexpr unsigned startEntitiesCount = 1;
}

Render2DScene::Render2DScene(robot2D::RenderWindow& window) : Scene(window),
                                                              m_scene(messageBus) {

}

void Render2DScene::setup() {
    ///// setup Ecs /////
    m_scene.addSystem<RenderSystem>(messageBus);
   // m_scene.addSystem<DemoMoveSystem>(messageBus);

    ///// setup Ecs /////

    m_textures.loadFromFile(ResourceID::Face, texturePath);
    m_textures.loadFromFile(ResourceID::Logo, texturePath1);
    m_textures.loadFromFile(ResourceID::City, texturePath2);
    m_textures.loadFromFile(ResourceID::Paddle, texturePath3);

    for(auto it = 0; it < startEntitiesCount; ++it)
        createEntity({position.x, position.y + size.x * it});
}

void Render2DScene::handleEvents(const robot2D::Event& event) {
    if(event.type == robot2D::Event::Resized) {
        RB_INFO("New Size = {0} and {1}", event.size.widht, event.size.heigth);
        m_window.resize({event.size.widht, event.size.heigth});
        m_window.setView(robot2D::View(robot2D::FloatRect{0, 0, event.size.widht, event.size.heigth}));

        //createEntity({position.x, position.y + size.x});
    }
}

void Render2DScene::update(float dt) {
    m_scene.update(dt);
}

void Render2DScene::imGuiRender() {

}

struct Quad: robot2D::Drawable, robot2D::Transformable {
    robot2D::Texture* texture = nullptr;
    robot2D::Color color = robot2D::Color::White;
    void draw(robot2D::RenderTarget& target, robot2D::RenderStates states) const override {
        states.transform *= getTransform();
        states.texture = texture;
        states.color = color;
        target.draw(states);
    }
};

void Render2DScene::render() {
    m_window.beforeRender();

    Quad quad;
    quad.texture = &m_textures.get(ResourceID::City);
    quad.setScale({800.F, 600.F});
    quad.setPosition({400.F, 300.F});

    Quad quad0;
    quad0.texture = &m_textures.get(ResourceID::Paddle);
    quad0.setScale({100.F, 100.F});
    quad0.setPosition({350.F, 350.F});


    Quad quad1;
    quad1.texture = &m_textures.get(ResourceID::Logo);
    quad1.setScale({200.F, 200.F});
    quad1.setPosition({200.F, 200.F});

    m_window.draw(quad);
    m_window.draw(quad0);
    m_window.draw(m_scene);
    m_window.draw(quad1);

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
