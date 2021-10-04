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
#include "sandbox/Render2DScene.hpp"
#include <robot2D/Ecs/Component.hpp>
#include <robot2D/Ecs/System.hpp>

class SpriteComponent final {
public:
    SpriteComponent();
    ~SpriteComponent() = default;

    void setTexture(const robot2D::Texture& texture);
    robot2D::Texture& getTexture();
    const robot2D::Texture& getTexture() const;


    void setColor(const robot2D::Color& color);
    const robot2D::Color& getColor() const;
private:
    const robot2D::Texture* m_texture;
    robot2D::Color m_color;
};


SpriteComponent::SpriteComponent(): m_color(robot2D::Color::White) {

}

void SpriteComponent::setTexture(const robot2D::Texture &texture) {
    m_texture = &texture;
}

robot2D::Texture& SpriteComponent::getTexture() {
    return const_cast<robot2D::Texture &>(*m_texture);
}

const robot2D::Texture& SpriteComponent::getTexture() const {
    return *m_texture;
}

void SpriteComponent::setColor(const robot2D::Color& color) {
    m_color = color;
}

const robot2D::Color& SpriteComponent::getColor() const {
    return m_color;
}



class TransformComponent final: public robot2D::Transformable {
public:
    TransformComponent();
    ~TransformComponent() override = default;

};

TransformComponent::TransformComponent() {}



class RenderSystem: public robot2D::ecs::System, public robot2D::Drawable {
public:
    RenderSystem(robot2D::MessageBus&);
    ~RenderSystem() override = default;

    void draw(robot2D::RenderTarget &target, robot2D::RenderStates) const override;
private:
};

RenderSystem::RenderSystem(robot2D::MessageBus& messageBus):
        System(messageBus, typeid(RenderSystem)) {
    addRequirement<SpriteComponent>();
    addRequirement<TransformComponent>();
}


void RenderSystem::draw(robot2D::RenderTarget& target, robot2D::RenderStates states) const {
    for(auto& it: m_entities) {
        auto& transform = it.getComponent<TransformComponent>();

        auto& sprite = it.getComponent<SpriteComponent>();
        states.transform = transform.getTransform();
        states.texture = &sprite.getTexture();
        states.color = sprite.getColor();
        target.draw(states);
    }
}



class DemoMoveSystem: public robot2D::ecs::System {
public:
    DemoMoveSystem(robot2D::MessageBus&);
    ~DemoMoveSystem() override = default;

    void update(float dt) override;
private:
};


DemoMoveSystem::DemoMoveSystem(robot2D::MessageBus& messageBus):
        robot2D::ecs::System(messageBus, typeid(DemoMoveSystem)) {
    addRequirement<TransformComponent>();
}

void DemoMoveSystem::update(float dt) {
    for(auto& it: m_entities) {
        auto& transform = it.getComponent<TransformComponent>();
        transform.move(robot2D::vec2f(1.f * dt, 0.f));
    }
}


struct Quad: public robot2D::Transformable, public robot2D::Drawable {
    void draw(robot2D::RenderTarget &target, robot2D::RenderStates states) const override {
        std::vector<robot2D::Vertex> vertices;
        states.transform *= getTransform();
        states.color = robot2D::Color::Blue;
        states.texture = nullptr;
        target.draw(states);
    }
};



Render2DScene::Render2DScene(robot2D::RenderWindow& window) : Scene(window),
                                                              m_scene(messageBus) {

}

void Render2DScene::setup() {
    ///// setup Ecs /////
    m_scene.addSystem<RenderSystem>(messageBus);
    m_scene.addSystem<DemoMoveSystem>(messageBus);

    ///// setup Ecs /////

    m_textures.loadFromFile("sprite", "awesomeface.png", true);

    for(auto it = 0; it < 5; ++it) {
        robot2D::ecs::Entity entity = m_scene.createEntity();

        auto& transform = entity.addComponent<TransformComponent>();

        transform.setPosition({100.F, 100.F + 100.F * it} );
        transform.scale({100.F, 100.F});

        auto& sprite = entity.addComponent<SpriteComponent>();
        sprite.setTexture(m_textures.get("sprite"));
    }

}

void Render2DScene::handleEvents(const robot2D::Event& event) {
    (void)event;
}

void Render2DScene::update(float dt) {
    m_scene.update(dt);
}

void Render2DScene::imGuiRender() {

}

void Render2DScene::render() {
    m_window.beforeRender();

//    robot2D::Sprite sprite;
//    sprite.setTexture(m_textures.get("sprite"));
//    sprite.setPosition({300.F, 300.F});
//    sprite.setSize({150.F, 150.F});
//
//    sprite.setTextureRect({48, 82, 83, 91});
//    m_window.draw(sprite);

    m_window.draw(m_scene);

    m_window.afterRender();
    m_window.flushRender();
}
