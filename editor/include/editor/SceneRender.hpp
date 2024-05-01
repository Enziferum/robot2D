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

#pragma once
#include <vector>

#include <robot2D/Graphics/RenderTarget.hpp>
#include <robot2D/Graphics/Shader.hpp>
#include <robot2D/Ecs/Entity.hpp>
#include <robot2D/Graphics/QuadBatchRender.hpp>
#include <robot2D/Graphics/Vertex.hpp>

namespace editor {
    class SceneRender {
    public:
        SceneRender() = default;
        ~SceneRender() = default;

        void setup();
        void setRuntimeCamera(bool flag) { m_runtimeRender = flag; }
        void update(std::vector<robot2D::ecs::Entity>& entities);
        void render(robot2D::RenderTarget& renderTarget,
                    std::vector<robot2D::ecs::Entity>& entities);
    private:
        bool m_needUpdateZBuffer{false};
        bool m_needUpdateText{false};
        robot2D::ShaderHandler m_textShader;
        robot2D::QuadBatchRender<robot2D::Vertex> m_quadBatchRender;

        robot2D::View* m_mainCamera{nullptr};
        bool m_runtimeRender{false};
    };
}