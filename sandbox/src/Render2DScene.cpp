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
    const robot2D::vec2f
    position = robot2D::vec2f {100.F, 100.F};
    const robot2D::vec2f size = robot2D::vec2f {100.F, 100.F};
    constexpr const char* texturePath = "res/textures/old_logo.png";
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

    m_textures.loadFromFile(ResourceID::Logo, texturePath);

    for(unsigned int it = 0; it < startEntitiesCount; ++it)
        createEntity({position.x, position.y + size.x * static_cast<float>(it)});
}

void Render2DScene::handleEvents(const robot2D::Event& event) {
    if(event.type == robot2D::Event::Resized) {
        RB_INFO("New Size = {0} and {1}", event.size.widht, event.size.heigth);
        m_window.resize({static_cast<int>(event.size.widht),
                         static_cast<int>(event.size.heigth)});
        m_window.setView(robot2D::View(robot2D::FloatRect{0, 0,
                                                          static_cast<float>(event.size.widht),
                                                          static_cast<float>(event.size.heigth)
        }));
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

robot2D::vec2f getScale(robot2D::vec2f targetSize, robot2D::vec2u originSize) {
    assert(originSize != robot2D::vec2u{});
    if(targetSize.x == originSize.x && targetSize.y == originSize.y)
        return {1.f, 1.f};
    return {targetSize.x / originSize.x, targetSize.y / originSize.y};
}

void Render2DScene::render() {
    m_window.beforeRender();
    m_window.draw(m_scene);
    m_window.afterRender();
}

void Render2DScene::createEntity(const robot2D::vec2f& position) {
    robot2D::ecs::Entity entity = m_scene.createEntity();

    auto& transform = entity.addComponent<TransformComponent>();

    transform.setPosition({position.x, position.y} );


    auto& sprite = entity.addComponent<SpriteComponent>();
    sprite.setTexture(m_textures.get(ResourceID::Logo));
    auto tx_sz = sprite.getTexture().getSize();
    transform.scale({100.f / tx_sz.y, 100.f / tx_sz.y });
}
