#include <sandbox/EcsScene.hpp>

namespace sandbox {









    class TransformComponent {
    public:


    };

















    EcsScene::EcsScene(robot2D::RenderWindow& window):
        Scene(window),
        m_scene(m_messageBus) {}

    void EcsScene::setup() {

        auto entity = m_scene.createEntity();
        entity.addComponent<TransformComponent>();





        auto& entityManager = m_scene.getEntityManager();

    }



    void EcsScene::handleEvents(const robot2D::Event& event) {

    }

    void EcsScene::update(float dt) {
        m_scene.update(dt);
    }

    void EcsScene::render() {
        m_window.draw(m_scene);
    }



}

