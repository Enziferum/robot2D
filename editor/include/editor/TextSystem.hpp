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