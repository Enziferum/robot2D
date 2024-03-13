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

#include <robot2D/imgui/Api.hpp>
#include <robot2D/Util/Logger.hpp>

#include <sandbox/EditorUIScene.hpp>
#include <sandbox/Systems.hpp>
#include <sandbox/Components.hpp>

#include "TreeHierarchy.hpp"

namespace sandbox {


    constexpr const char* gridVertexShader = R"(
        #version 330 core
        layout (location = 0) in vec2 Position;
        layout (location = 1) in vec2 TexCoord;

        out vec2 fragCoord;
        uniform mat4 projectionMatrix;

        void main() {
            fragCoord = TexCoord;
            gl_Position = vec4(Position, 0, 1.0);
            gl_Position = sign(gl_Position);

            fragCoord = (vec2(gl_Position.x, gl_Position.y)
                         + vec2(1.0)) / vec2(2.0);
        }

    )";

    constexpr const char* gridFragmentShader = R"(
        #version 450 core

        layout (location = 0) out vec4 fragColor;
        in vec2 fragCoord;

        const float cellSize = 128.0;

        // colors
        const vec4 col0 = vec4(0.5, 0.5, 0.5, 1.0);
        const vec4 col1 = vec4(0.7, 0.7, 0.7, 1.0);

        bool checkboard(vec2 textureCoords, vec2 scale) {
            float s = textureCoords.x;
            float t = textureCoords.y;

            float sum = floor(s * scale.x) + floor(t * scale.y);
            bool isEven = mod(sum, 2.0) == 0.0;
            return isEven;
        }

        void main() {
            vec2 scale = vec2(gl_FragCoord.x / cellSize, gl_FragCoord.y / cellSize);
            bool isEven = checkboard(fragCoord, vec2(cellSize, cellSize * 0.6));
            fragColor = isEven ? col0 : col1;
        }
    )";


    robot2D::TreeHierarchy treeHierarchy{"MainScene"};

    struct TagComponent {
        std::string name;
    };


    robot2D::ecs::Entity addEntity(robot2D::ecs::Scene& scene,
                                   robot2D::vec2f position,
                                   robot2D::vec2f size,
                                   std::string name) {
        auto entity = scene.createEntity();
        auto& transform = entity.addComponent<TransformComponent>();
        transform.setPosition(position);
        transform.setSize(size);

        auto& drawable = entity.addComponent<SpriteComponent>();
        drawable.setColor(robot2D::Color::Green);

        entity.addComponent<TagComponent>().name = std::move(name);
        return entity;
    }


    EditorUIScene::EditorUIScene(robot2D::RenderWindow& window):
    Scene(window),
    m_messageBus{},
    m_scene{m_messageBus}
    {

    }

    void EditorUIScene::setup() {
        m_scene.addSystem<RenderSystem>(m_messageBus);
        m_attachedEntities.clear();

        m_view.reset({0, 0, 1920, 1080});
        //m_view.setViewport({0, 0, 0.5, 1});
       // m_view.setIsClipping(true);

       m_texture.loadFromFile("res/textures/Doodler.png");

        treeHierarchy.addOnSelectCallback([this](robot2D::ITreeItem::Ptr item){
            RB_INFO("Select TreeItem {0}", item -> getID());
        });

        treeHierarchy.addOnCallback([this](robot2D::ITreeItem::Ptr item) {
            bool entityDeleted = false;
            if (ImGui::BeginPopupContextItem())
            {
                if (ImGui::MenuItem("Delete Entity"))
                    entityDeleted = true;

                ImGui::EndPopup();
            }
            if(entityDeleted) {
                if(item -> isChild()) {
                    item -> removeSelf();
                }
                else {
                    if(item -> hasChildrens()) {
                        for(auto child: item -> getChildrens()) {
                            auto entity = child -> getUserData<robot2D::ecs::Entity>();
                            removeEntity(std::move(*entity));
                            item -> deleteChild(child);
                        }
                        auto entity = item -> getUserData<robot2D::ecs::Entity>();
                        removeEntity(std::move(*entity));
                        treeHierarchy.deleteItem(item);
                    }
                    else {
                        auto entity = item -> getUserData<robot2D::ecs::Entity>();
                        removeEntity(std::move(*entity));
                        treeHierarchy.deleteItem(item);
                    }
                }
            }
        });

        treeHierarchy.addOnReorderCallback([](robot2D::ITreeItem::Ptr source, robot2D::ITreeItem::Ptr target){
            RB_INFO("Reorder TreeItem source ID: {0}", source -> getID());
            RB_INFO("Reorder TreeItem target ID: {0}", target -> getID());
            treeHierarchy.setBefore(source, target);
        });

        treeHierarchy.addOnMakeChildCallback([this](robot2D::ITreeItem::Ptr source, robot2D::ITreeItem::Ptr intoTarget) {
            auto entity = intoTarget -> getUserData<robot2D::ecs::Entity>();
            auto& transform = entity -> getComponent<TransformComponent>();
            transform.addChild(*source -> getUserData<robot2D::ecs::Entity>());
            auto pos = transform.getPosition();
            pos.y += 100;
            transform.setPosition(pos);

            treeHierarchy.deleteItem(source);
            intoTarget -> addChild(source);
        });


        constexpr int startEntityValue = 1;

        for(int i = 0; i < startEntityValue; ++i) {

            std::string tagName = "Entity:" + std::to_string(i);
            auto entity = addEntity(m_scene, { 100 + 200 * i, 100 }, { 54, 25 }, tagName);
            if(i == 0) {
                entity.getComponent<SpriteComponent>().setColor(robot2D::Color::Yellow);
                entity.getComponent<SpriteComponent>().setTexture(m_texture);
            }

            m_attachedEntities.push_back(entity);
            if(i == 3) {
                entity.getComponent<SpriteComponent>().setColor(robot2D::Color::Red);
                auto item = treeHierarchy.getItem<robot2D::ecs::Entity>(1);
                item -> getUserData<robot2D::ecs::Entity>()->getComponent<TransformComponent>().addChild(entity);
                auto childItem = item -> addChild();
                childItem -> setName(entity.getComponent<TagComponent>().name);
                childItem -> setUserData(m_attachedEntities[i]);
            }
            else {
                auto item = treeHierarchy.addItem<robot2D::ecs::Entity>();
                item -> setName(entity.getComponent<TagComponent>().name);
                item -> setUserData(m_attachedEntities[i]);
            }

        }

    }

    void EditorUIScene::handleEvents(const robot2D::Event& event) {
        if(event.type == robot2D::Event::MousePressed && event.mouse.btn
            == robot2D::mouse2int(robot2D::Mouse::MouseLeft))
            RB_INFO("Mouse Pos {0}:{1}", event.mouse.x, event.mouse.y);
    }

    void EditorUIScene::update(float dt) {
        m_scene.update(dt);
    }

    void EditorUIScene::render() {
        //m_window.clear(robot2D::Color::White);
        m_window.beforeRender();
        m_window.setView(m_view);

        m_window.draw(m_scene);
        //m_shader.use();
        m_window.afterRender();



       // m_shader.unUse();
    }

    void EditorUIScene::imGuiRender() {
        robot2D::WindowOptions windowOptions {};
        windowOptions.name = "EditorUIWindow";
        windowOptions.flagsMask =  ImGuiWindowFlags_NoScrollbar;

        robot2D::createWindow(windowOptions, [this]() { this -> windowFunction(); });
    }

    void EditorUIScene::windowFunction() {
        treeHierarchy.render();
    }

    void EditorUIScene::removeEntity(robot2D::ecs::Entity&& entity) {
        auto found = std::find_if(m_attachedEntities.begin(), m_attachedEntities.end(),
                                                [&entity](robot2D::ecs::Entity ent) {
            return ent.getIndex() == entity.getIndex();
        });
        m_scene.removeEntity(entity);
        m_attachedEntities.erase(found, m_attachedEntities.end());
    }


}


