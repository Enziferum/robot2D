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

#include <algorithm>

#include <robot2D/Graphics/RenderTarget.hpp>
#include <robot2D/Ecs/EntityManager.hpp>

#include <editor/RendererSystem.hpp>
#include <editor/TextSystem.hpp>
#include <editor/Scene.hpp>

//#include "glm/gtc/type_ptr.hpp"
#include "editor/panels/TreeHierarchy.hpp"

namespace editor {

    namespace  {
        struct BoundingBox: robot2D::Drawable {
            struct Wall: robot2D::Drawable {
                void draw(robot2D::RenderTarget& target,
                          robot2D::RenderStates states) const {
                    robot2D::Transform transform;
                    transform.translate(position);
                    transform.scale(size);
                    states.transform *= transform;
                    states.color = color;
                    target.draw(states);
                }

                robot2D::Color color;
                robot2D::vec2f position;
                robot2D::vec2f size;
                float angle = 0.f;
            };

            BoundingBox() {
                for(auto& wall: m_moveWalls)
                    wall.color = m_borderColor;
            }

            void setPosition(const robot2D::vec2f& position) {
                m_aabb.lx = position.x;
                m_aabb.ly = position.y;
            }

            void setSize(const robot2D::vec2f& size) {
                m_aabb.width = size.x;
                m_aabb.height = size.y;
            }

            void setAngle(float angle) {
                m_angle = static_cast<float>(fmod(angle, 360));
            }

            void setBox(const robot2D::FloatRect& rect) {
                m_aabb = rect;
            }

            void draw(robot2D::RenderTarget& target, robot2D::RenderStates states) const override {

                m_moveWalls[0].position = robot2D::vec2f(m_aabb.lx - m_borderWidth, m_aabb.ly - m_borderWidth);
                m_moveWalls[0].size = robot2D::vec2f(m_aabb.width + m_borderWidth * 2, m_borderWidth);
                m_moveWalls[0].angle = m_angle;

                m_moveWalls[1].position = robot2D::vec2f(m_aabb.lx, m_aabb.ly);
                m_moveWalls[1].size = robot2D::vec2f(m_borderWidth, m_aabb.height);
                m_moveWalls[1].angle = m_angle;

                m_moveWalls[2].position = robot2D::vec2f(m_aabb.lx, m_aabb.ly + m_aabb.height);
                m_moveWalls[2].size = robot2D::vec2f(m_aabb.width, m_borderWidth);
                m_moveWalls[2].angle = m_angle;

                m_moveWalls[3].position = robot2D::vec2f(m_aabb.lx + m_aabb.width, m_aabb.ly);
                m_moveWalls[3].size = robot2D::vec2f(m_borderWidth, m_aabb.height);
                m_moveWalls[3].angle = m_angle;

                for(const auto& wall: m_moveWalls) {
                    target.draw(wall);
                }


            }
        private:
            float m_borderWidth = 1.f;
            robot2D::Color m_borderColor = robot2D::Color::Cyan;
            mutable std::array<Wall, 4> m_moveWalls;
            robot2D::FloatRect m_aabb;
            float m_angle;
        };
    }


    RenderSystem::RenderSystem(robot2D::MessageBus& messageBus):
            robot2D::ecs::System(messageBus,typeid(RenderSystem)),
            m_needUpdateZBuffer{false} {
        addRequirement<TransformComponent>();
        addRequirement<DrawableComponent>();
    }

    void RenderSystem::update(float dt) {
        (void)dt;

        for(auto item: m_insertItems) {
            auto source = std::get<1>(item);
            m_entities.insert(std::get<0>(item), source);

            if(std::get<2>(item) == ReorderDeleteType::Last) {
                auto found = util::find_last_if(m_entities.begin(), m_entities.end(),
                                                [&source](robot2D::ecs::Entity item) {
                    return item.getIndex() == source.getIndex();
                });
                m_entities.erase(found);
            }
            else if(std::get<2>(item) == ReorderDeleteType::First) {
                auto found = util::find_first_if(m_entities.begin(), m_entities.end(),
                                                 [&source](robot2D::ecs::Entity item) {
                    return item.getIndex() == source.getIndex();
                });
                m_entities.erase(found);
            }
        }
        m_insertItems.clear();


        for(auto& entity: m_entities) {
            auto& drawable = entity.getComponent<DrawableComponent>();
            if(drawable.m_needUpdateZbuffer) {
                m_needUpdateZBuffer = true;
                drawable.m_needUpdateZbuffer = false;
            }
            if(entity.hasComponent<CameraComponent>()) {
                auto camera = entity.getComponent<CameraComponent>();
                if (camera.isPrimary) {
                    auto size = camera.getSize();
                    auto pos = camera.getPosition();

                    m_cameraView.reset({pos.x, pos.y, size.x, size.y});
                }
            }
        }

        if(m_needUpdateZBuffer) {
            std::stable_sort(m_entities.begin(), m_entities.end(),
                      [](const robot2D::ecs::Entity& left, const robot2D::ecs::Entity& right) {
                return left.getComponent<DrawableComponent>().getDepth() <
                        right.getComponent<DrawableComponent>().getDepth();
            });

            m_needUpdateZBuffer = false;
        }
    }


    void RenderSystem::draw(robot2D::RenderTarget& target, robot2D::RenderStates states) const {
        for(auto& ent: m_entities) {
            auto& transform = ent.getComponent<TransformComponent>();
            auto& drawable = ent.getComponent<DrawableComponent>();
            auto t = transform.getTransform();

            if (ent.hasComponent<CameraComponent>() && m_runtimeFlag) {
                auto camera = ent.getComponent<CameraComponent>();
                if (camera.isPrimary) {
                     target.setView(m_cameraView);
                }
            }

            robot2D::RenderStates renderStates;
            renderStates.transform *= t;
            if(drawable.hasTexture())
                renderStates.texture = &drawable.getTexture();
            renderStates.color = drawable.getColor();
            renderStates.layerID = drawable.getLayerIndex();

            if(getScene() -> hasSystem<TextSystem>() && ent.hasComponent<TextComponent>()) {
                if(!ent.getComponent<TextComponent>().getFont())
                    continue;

                auto textSystem = getScene() -> getSystem<TextSystem>();
                auto vertexArray =  textSystem -> getVertexArray();
                renderStates.shader = const_cast<robot2D::ShaderHandler*>(&textSystem -> getShader());
                renderStates.shader -> use();
                auto view = target.getView(drawable.getLayerIndex()).getTransform();
                renderStates.shader -> setMatrix("projection", view.get_matrix());
                renderStates.shader -> unUse();
                renderStates.renderInfo.indexCount = textSystem -> getIndexCount();
                target.draw(vertexArray, renderStates);
            }
            else {
                auto& vertices = drawable.getVertices();
                vertices[0].position = renderStates.transform * robot2D::vec2f {0.F, 0.F};
                vertices[1].position = renderStates.transform * robot2D::vec2f {1.F, 0.F};
                vertices[2].position = renderStates.transform * robot2D::vec2f {1.F, 1.F};
                vertices[3].position = renderStates.transform * robot2D::vec2f {0.F, 1.F};

                renderStates.entityID = ent.getIndex();

                robot2D::VertexData vertexData{};
                for(const auto& vertex: vertices)
                    vertexData.emplace_back(vertex);
                target.draw(vertexData, renderStates);
            }

        }
        for(const auto& ent: m_entities) {
            const auto& transform = ent.getComponent<TransformComponent>();
            const auto& drawable = ent.getComponent<DrawableComponent>();
            if(!drawable.drawBoundingBox())
                continue;
            const auto& rect = transform.getGlobalBounds();
            BoundingBox bb;
            bb.setBox(rect);
            target.draw(bb);
        }
    }

    void RenderSystem::setScene(Scene* scene) {
        m_activeScene = scene;
    }

    void RenderSystem::setBefore(robot2D::ecs::Entity source, robot2D::ecs::Entity target) {
        auto sourceIter = std::find_if(m_entities.begin(), m_entities.end(),
                                       [&source](robot2D::ecs::Entity item) {
            return item.getIndex() == source.getIndex();
        });

        auto targetIter = std::find_if(m_entities.begin(), m_entities.end(),
                                       [&target](robot2D::ecs::Entity item) {
            return item.getIndex() == target.getIndex();
        });

        auto sourceOldDistance = std::distance(m_entities.begin(), sourceIter);
        auto targetOldDistance = std::distance(m_entities.begin(), targetIter);

        if(sourceOldDistance > targetOldDistance) {
            /// insert before remove last
            m_insertItems.push_back(std::make_tuple(targetIter, source, ReorderDeleteType::Last));
        }
        else if (sourceOldDistance < targetOldDistance) {
            m_insertItems.push_back(std::make_tuple(targetIter, source, ReorderDeleteType::First));
        }
    }

    robot2D::ecs::System::Ptr RenderSystem::cloneSelf(robot2D::ecs::Scene* scene, const std::vector<robot2D::ecs::Entity>& newEntities) {
        auto cloneSystem = std::make_shared<RenderSystem>(m_messageBus);
        if(!cloneBase(cloneSystem, scene, newEntities))
            return nullptr;
        cloneSystem -> m_needUpdateZBuffer = m_needUpdateZBuffer;
        cloneSystem -> m_runtimeFlag;
        cloneSystem -> m_cameraView = m_cameraView;
        return cloneSystem;
    }

}