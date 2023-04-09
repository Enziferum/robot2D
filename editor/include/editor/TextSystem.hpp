/*********************************************************************
(c) Alex Raag 2023
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

#include <robot2D/Ecs/System.hpp>
#include <robot2D/Graphics/Shader.hpp>
#include <robot2D/Graphics/Vertex.hpp>
#include <robot2D/Graphics/QuadBatchRender.hpp>

#include "Components.hpp"

namespace editor {

    class TextSystem: public robot2D::ecs::System {
    public:
        TextSystem(robot2D::MessageBus& messageBus);
        ~TextSystem() override = default;

        void update(float dt);

        robot2D::VertexArray::Ptr getVertexArray() const { return m_quadBatchRender.getVertexArray(); }
        int getIndexCount() const { return m_quadBatchRender.getIndexCount(); }
        const robot2D::ShaderHandler& getShader() const { return m_textShader; }
    private:
        void setupGL();
    private:
        robot2D::ShaderHandler m_textShader;
        robot2D::QuadBatchRender<robot2D::Vertex> m_quadBatchRender;

        bool m_initialized;
        bool m_needUpdate;
    };

}