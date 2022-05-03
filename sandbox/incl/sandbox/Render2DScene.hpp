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

#include <robot2D/Util/ResourceHandler.hpp>
#include <robot2D/Ecs/Scene.hpp>
#include "Scene.hpp"

enum class ResourceID {
    Face, Logo, City,
    Paddle
};

class Render2DScene: public Scene {
public:
    explicit Render2DScene(robot2D::RenderWindow&);
    ~Render2DScene() override = default;

    virtual void setup() override;
    virtual void handleEvents(const robot2D::Event &event) override;
    virtual void update(float dt) override;
    virtual void imGuiRender() override;
    virtual void render() override;

private:
    robot2D::ecs::Entity createEntity(const robot2D::vec2f& position, unsigned int layerIndex = 1);
private:
    robot2D::ecs::Scene m_scene;
    robot2D::MessageBus messageBus;
    robot2D::ResourceHandler<robot2D::Texture, ResourceID> m_textures;
};