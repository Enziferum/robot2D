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

#include <robot2D/Core/Clock.hpp>
#include <editor/Editor.hpp>

#include <editor/ComponentPanel.hpp>
#include <editor/ScenePanel.hpp>

#include <imgui/imgui.h>
#include <editor/Wrapper.hpp>

namespace editor {

    robot2D::Texture texture;
    robot2D::Texture texture1;

    struct Quad: public robot2D::Transformable, public robot2D::Drawable {
        robot2D::Color color = robot2D::Color::White;

        void draw(robot2D::RenderTarget &target, robot2D::RenderStates states) const override {
            std::vector<robot2D::Vertex> vertices;
            states.transform *= getTransform();
            states.color = color;
            states.texture = nullptr;
            target.draw(states);
        }
    };


    Editor::Editor(robot2D::RenderWindow& window): m_window{window}, m_docking{true},
    m_activeScene{ nullptr }, m_frameBuffer{nullptr}{}

    void Editor::setup() {
        auto& panel = m_panelManager.addPanel<ComponentPanel>();
        m_panelManager.addPanel<ScenePanel>();

        robot2D::FrameBufferSpecification frameBufferSpecification;
        auto windowSize = m_window.getSize();
        frameBufferSpecification.size = {windowSize.x, windowSize.y};
        m_frameBuffer = robot2D::FrameBuffer::Create(frameBufferSpecification);
        texture.loadFromFile("awesomeface.png");
        texture1.loadFromFile("logo.png");
    }

    void Editor::handleEvents(const robot2D::Event& event) {

    }

    void Editor::handleMessages(const robot2D::Message& message) {

    }

    robot2D::Sprite sprite;

    float _dt = 0.F;

    void Editor::update(float dt) {
        m_panelManager.update(dt);
        sprite.rotate(50 * dt);
        _dt = dt;
    }


    void Editor::render() {
        m_frameBuffer -> Bind();
        robot2D::Color clearColor = robot2D::Color::fromGL(0.1, 0.1, 0.1, 1);
        m_window.clear(clearColor);
        m_window.beforeRender();
//
//
        for (int it = 0; it < 10; ++it) {
            for(int ij = 0; ij < 10; ++ij) {
                robot2D::Sprite quad;
                robot2D::vec2f startPos = {500.F, 400.F};
                quad.setPosition({40 * it + startPos.x + 10.f, ij * 40 + startPos.y + 10.f});
                quad.setTexture(texture);
                quad.rotate(50 * _dt);
                //quad.scale({20.F, 20.F});
                quad.setSize({20.F, 20.F});

                m_window.draw(quad);
            }
        }

        sprite.setTexture(texture);
        sprite.setSize({100.F, 100.F});
        sprite.setPosition({200.F, 200.F});
        m_window.draw(sprite);

        robot2D::Sprite sp;
        sp.setTexture(texture1);
        sp.setPosition({100.F, 100.F});
        sp.setSize({100.F, 100.F});
        m_window.draw(sp);

        m_window.afterRender();
        m_window.flushRender();

        // render MainScreen
        m_frameBuffer->unBind();
        imguiRender();
    }

    void Editor::imguiRender() {
        static bool opt_fullscreen = true;
        static bool opt_padding = true;
        static bool dockspace_open = false;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
        // because it would be confusing to have two docking targets within each others.
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen)
        {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }
        else
        {
            dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
        }

        // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
        // and handle the pass-thru hole, so we ask Begin() to not render a background.
        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        // Important: note that we proceed even if Begin() returns false (aka window is collapsed).
        // This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
        // all active windows docked into it will lose their parent and become undocked.
        // We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
        // any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
        if (!opt_padding)
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace Demo", &dockspace_open, window_flags);
        if (!opt_padding)
            ImGui::PopStyleVar();

        if (opt_fullscreen)
            ImGui::PopStyleVar(2);

        // Submit the DockSpace
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if(ImGui::MenuItem("Open", "Ctrl+O")) {}
                if(ImGui::MenuItem("Save", "Ctrl+S")) {}
                ImGui::Separator();

                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }


        ImGui::End();

       // m_panelManager.render();

        ImGui::Begin("Stats");
        auto stats = m_window.getStats();
        ImGui::Text("Rendering 2D Stats ...");
        ImGui::Text("Quads Count %d", stats.drawQuads);
        ImGui::Text("Draw Calls Count %d", stats.drawCalls);

        ImGui::Begin("Viewport");
        ImGui::RenderFrameBuffer(m_frameBuffer, {800, 600});
        ImGui::End();

        ImGui::End();

    }


}