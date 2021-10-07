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
#include <stdexcept>
#include <imgui/imgui.h>
#include <tfd/tinyfiledialogs.h>

#include <editor/Editor.hpp>
// Panels
#include <editor/ComponentPanel.hpp>
#include <editor/ScenePanel.hpp>
#include <editor/AssetsPanel.hpp>

#include <editor/Wrapper.hpp>
#include <editor/SceneSerializer.hpp>
#include <editor/Components.hpp>



namespace editor {
    // develop only flag
    constexpr bool useGUI = true;

    Editor::Editor(robot2D::RenderWindow& window, robot2D::MessageBus& messageBus): m_window{window},
    m_messageBus{messageBus},
    m_activeScene{ nullptr }, m_frameBuffer{nullptr}, m_ViewportSize{} {}

    void Editor::setup() {
        const std::string texturesPath = "res/textures/";
        std::unordered_map<TextureID, std::string> texturePaths = {
                {TextureID::Logo, "logo.png"}
        };

        for(auto& it: texturePaths) {
            auto fullPath = texturesPath + it.second;
            if(!m_textures.loadFromFile(it.first, fullPath)) {
                std::runtime_error("Can't load Texture");
            }
        }

        m_activeScene = std::make_shared<Scene>(m_messageBus);

        auto& panel = m_panelManager.addPanel<ComponentPanel>();
        auto& scenePanel = m_panelManager.addPanel<ScenePanel>();
        scenePanel.setActiveScene(std::move(m_activeScene));
        auto& assetsPanel = m_panelManager.addPanel<AssetsPanel>();

        robot2D::FrameBufferSpecification frameBufferSpecification;
        auto windowSize = m_window.getSize();

        frameBufferSpecification.size = {windowSize.x, windowSize.y};
        m_frameBuffer = robot2D::FrameBuffer::Create(frameBufferSpecification);
        m_window.setView({{0, 0}, {windowSize.x, windowSize.y}});
        m_camera.resize({0, 0, windowSize.x, windowSize.y});
    }

    bool m_leftCtrlPressed = false;

    void Editor::handleEvents(const robot2D::Event& event) {
        if(event.type == robot2D::Event::KeyPressed) {
            if(event.key.code == robot2D::Key::LEFT_CONTROL) {
                m_leftCtrlPressed = true;
            }
            if(event.key.code == robot2D::Key::S) {
                if(m_leftCtrlPressed) {
                    saveScene(m_activeScene->getPath());
                }
            }
        }
        if(event.type == robot2D::Event::KeyReleased) {
            if(event.key.code == robot2D::Key::LEFT_CONTROL) {
                m_leftCtrlPressed = false;
            }
        }

        m_camera.onEvent(event);
    }

    void Editor::handleMessages(const robot2D::Message& message) {}

    void Editor::update(float dt) {
        m_activeScene -> update(dt);
        m_panelManager.update(dt);
        m_dt = dt;
    }

    void Editor::render() {
        if(useGUI) {
            m_frameBuffer -> Bind();
            robot2D::Color clearColor = robot2D::Color::fromGL(0.1, 0.1, 0.1, 1);
            m_window.clear(clearColor);
        }
        m_window.beforeRender();
        m_window.setView(m_camera.getView());

        for(auto& it: m_activeScene->getEntities()) {
            if(!it.hasComponent<SpriteComponent>())
                continue;

            auto& sprite = it.getComponent<SpriteComponent>();
            auto transform = it.getComponent<TransformComponent>();


            robot2D::RenderStates renderStates;
            if(sprite.getTexturePointer() == nullptr) {
                //continue;
                sprite.setTexture(m_textures.get(TextureID::Logo));
            }
            renderStates.texture = &sprite.getTexture();
            renderStates.transform *= transform.getTransform();
            renderStates.color = sprite.getColor();
            m_window.draw(renderStates);
        }

        m_window.afterRender();
        m_window.flushRender();

        if(useGUI) {
            m_frameBuffer->unBind();
            imguiRender();
        }
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
        ImGui::Begin("Scene", &dockspace_open, window_flags);
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
            // TODO make filedialog
            if (ImGui::BeginMenu("File"))
            {
                if(ImGui::MenuItem("Open", "Ctrl+O")) {
                    std::string openPath = "assets/scenes/demoScene.robot2D";
                    const char *patterns[1] = {"*.robot2D"};
                    char *path = tinyfd_openFileDialog("Load Scene", nullptr,
                                                       1, patterns, "Scene", 0);
                    if (path != nullptr) {
                        openPath = std::string(path);
                        openScene(path);
                    }
                }
                if(ImGui::MenuItem("Save", "Ctrl+Shift+S")) {
                    std::string savePath = "assets/scenes/demoScene.robot2D";
                    const char* patterns[1] = {"*.robot2D"};
                    char* path = tinyfd_saveFileDialog("Load Scene", nullptr,
                                          1, patterns, "Scene");
                    if(path != nullptr) {
                        savePath = std::string(path);
                        saveScene(savePath);
                    }
                }
                ImGui::Separator();

                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        ImGui::End();


        m_panelManager.render();

        ImGui::Begin("Statistics ");
        auto stats = m_window.getStats();
        ImGui::Text("Rendering 2D Stats ...");
        ImGui::Text("Quads Count %d", stats.drawQuads);
        ImGui::Text("Draw Calls Count %d", stats.drawCalls);
        ImGui::End();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
        ImGui::Begin("Viewport");
        auto ViewPanelSize = ImGui::GetContentRegionAvail();
        if(m_ViewportSize != robot2D::vec2u { ViewPanelSize.x, ViewPanelSize.y}) {
            m_ViewportSize = {ViewPanelSize.x, ViewPanelSize.y};
            m_frameBuffer -> Resize(m_ViewportSize);
        }
        ImGui::RenderFrameBuffer(m_frameBuffer, {m_ViewportSize.x, m_ViewportSize.y});
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
            {
                const wchar_t* path = (const wchar_t*)payload->Data;
                std::filesystem::path scenePath = std::filesystem::path("assets") / path;
                LOG_INFO("PATH %", scenePath.string().c_str())
                openScene(scenePath.string());
            }
            ImGui::EndDragDropTarget();
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

    bool Editor::openScene(const std::string& path) {
        auto newScene = std::make_shared<Scene>(m_messageBus);
        newScene -> setPath(path);
        if(newScene == nullptr)
            return false;
        m_activeScene.reset();
        m_activeScene = newScene;
        SceneSerializer serializer(m_activeScene);
        bool res = serializer.deserialize(path);
        if(!res)
            return false;
        auto& scenePanel = m_panelManager.getPanel<ScenePanel>();
        scenePanel.setActiveScene(std::move(m_activeScene));
        return res;
    }

    bool Editor::saveScene(const std::string& path) {
        SceneSerializer serializer(m_activeScene);
        return serializer.serialize(path);
    }

}