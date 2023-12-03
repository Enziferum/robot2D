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