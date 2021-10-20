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
#include <editor/MenuPanel.hpp>
#include <editor/ViewportPanel.hpp>
#include <editor/InspectorPanel.hpp>

#include <editor/Wrapper.hpp>
#include <editor/SceneSerializer.hpp>
#include <editor/Components.hpp>
#include <editor/EditorStyles.hpp>

namespace editor {
    namespace fs = std::filesystem;
    // develop only flag
    constexpr bool useGUI = true;
    const std::string texturesPath = "res/textures/";
    bool m_leftCtrlPressed = false;
    static bool opt_padding = true;
    static bool dockspace_open = false;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    std::unordered_map<TextureID, std::string> texturePaths = {
            {TextureID::Logo, "logo.png"},
    };

    Editor::Editor(robot2D::RenderWindow& window, robot2D::MessageBus& messageBus): m_state(State::Edit),
    m_window{window},
    m_messageBus{messageBus},
    m_activeScene{ nullptr }, m_frameBuffer{nullptr}, m_ViewportSize{},
    m_currentProject{nullptr} {}

    void Editor::setup() {
        for(auto& it: texturePaths) {
            auto fullPath = texturesPath + it.second;
            if(!m_textures.loadFromFile(it.first, fullPath)) {
                throw std::runtime_error("Can't load Texture");
            }
        }

        //auto& panel = m_panelManager.addPanel<ComponentPanel>();
        auto& scenePanel = m_panelManager.addPanel<ScenePanel>();
        auto& assetsPanel = m_panelManager.addPanel<AssetsPanel>();
        auto& inspectorPanel = m_panelManager.addPanel<InspectorPanel>(m_camera);
        auto& menuPanel = m_panelManager.addPanel<MenuPanel>();
    }

    void Editor::prepare() {
        auto windowSize = m_window.getSize();

        robot2D::FrameBufferSpecification frameBufferSpecification;
        frameBufferSpecification.size = {windowSize.x, windowSize.y};

        m_frameBuffer = robot2D::FrameBuffer::Create(frameBufferSpecification);
        m_window.setView({{0, 0}, {windowSize.x, windowSize.y}});
        m_camera.resize({0, 0, windowSize.x, windowSize.y});

        applyStyle(EditorStyle::GoldBlack);
    }

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
        switch(m_state) {
            case State::Edit: {
                m_activeScene -> update(dt);
                m_panelManager.update(dt);
                break;
            }
            case State::Run: {
                m_activeScene ->updateRuntime(dt);
                break;
            }
        }
    }

    void Editor::render() {
        if(useGUI) {
            m_frameBuffer -> Bind();
            const auto& clearColor = m_panelManager.getPanel<InspectorPanel>().getColor();
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
            guiRender();
        }
    }

    void Editor::guiRender() {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        ImGui::Begin("Scene", &dockspace_open, window_flags);
        ImGui::PopStyleVar(2);

        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        mainMenubar();

        auto stats = m_window.getStats();
        m_panelManager.getPanel<InspectorPanel>().setRenderStats(std::move(stats));

        m_panelManager.render();
        ImGui::End();

        /// Viewport Panel ///

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
#ifdef ROBOT2D_WINDOWS
                const wchar_t* path = (const wchar_t*)payload->Data;
#else
                const char* path = (const char*)payload->Data;
#endif
                std::filesystem::path scenePath = std::filesystem::path("assets") / path;
                RB_EDITOR_INFO("PATH {0}", scenePath.string().c_str());
                openScene(scenePath.string());
            }
            ImGui::EndDragDropTarget();
        }
        ImGui::End();
        ImGui::PopStyleVar();

        /// Viewport Panel ///
    }

    void Editor::mainMenubar() {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if(ImGui::MenuItem("New", "Ctrl+N")) {
                    createScene();
                }
                ImGui::Separator();

                if(ImGui::MenuItem("Open", "Ctrl+O")) {
                    std::string openPath = "assets/scenes/demoScene.robot2D";
                    const char* patterns[1] = {"*.scene"};
                    char *path = tinyfd_openFileDialog("Load Scene", nullptr,
                                                       1, patterns, "Scene", 0);
                    if (path != nullptr) {
                        openPath = std::string(path);
                        openScene(path);
                    }
                }

                if(ImGui::MenuItem("Save", "Ctrl+Shift+S")) {
                    std::string savePath = "assets/scenes/demoScene.robot2D";
                    const char* patterns[1] = {"*.scene"};
                    char* path = tinyfd_saveFileDialog("Load Scene", nullptr,
                                                       1, patterns, "Scene");
                    if(path != nullptr) {
                        savePath = std::string(path);
                        saveScene(savePath);
                    }
                }

                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
    }

//// TODO Move Scene Processing to SceneManager ////
    bool Editor::createScene() {
        if(m_activeScene != nullptr) {
            m_activeScene.reset();
            // todo show dialog
        }
        m_activeScene = std::make_shared<Scene>(m_messageBus);
        auto& scenePanel = m_panelManager.getPanel<ScenePanel>();
        scenePanel.setActiveScene(std::move(m_activeScene));
        return true;
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


    void Editor::createProject(Project::Ptr project) {
        m_currentProject = project;
        prepare();

        auto& assetsPanel = m_panelManager.getPanel<AssetsPanel>();
        auto path = m_currentProject -> getPath();
        auto dirPath = fs::path(path);
        dirPath.append("assets");
        assetsPanel.setAssetsPath(dirPath.string());
    }

    void Editor::loadProject(Project::Ptr project) {
        m_currentProject = project;
        prepare();

        auto path = project->getPath();
        fs::path dirPath(path);
        auto localPath = std::filesystem::path("assets/scenes/" + project->getStartScene());
        dirPath.append(localPath.string());

        openScene(dirPath.string());

        auto& assetsPanel = m_panelManager.getPanel<AssetsPanel>();
        dirPath = fs::path(path);
        dirPath.append("assets");

        assetsPanel.setAssetsPath(dirPath.string());
    }

}