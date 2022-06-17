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
#include <map>

#include <imgui/imgui.h>
#include <robot2D/Extra/Api.hpp>
#include <tfd/tinyfiledialogs.h>

#include <editor/Editor.hpp>
#include <editor/EventBinder.hpp>
// Panels
#include <editor/ScenePanel.hpp>
#include <editor/AssetsPanel.hpp>
#include <editor/MenuPanel.hpp>
#include <editor/InspectorPanel.hpp>
#include <editor/ViewportPanel.hpp>


#include <editor/SceneSerializer.hpp>
#include <editor/Components.hpp>
#include <editor/EditorStyles.hpp>
#include <editor/FileApi.hpp>

namespace editor {
    namespace fs = std::filesystem;

    Editor::Editor(robot2D::MessageBus& messageBus):
    m_state(State::Edit),
    m_window{nullptr},
    m_messageBus{messageBus},
    m_currentProject{nullptr},
    m_activeScene{nullptr},
    m_frameBuffer{nullptr},
    m_ViewportSize{},
    m_configuration{},
    m_sceneManager{m_messageBus} {}

    void Editor::setup(robot2D::RenderWindow* window) {
        if(m_window == nullptr)
            m_window = window;

        for(auto& it: m_configuration.texturePaths) {
            auto fullPath = m_configuration.texturesPath + it.second;
            if(!m_textures.loadFromFile(it.first, fullPath)) {
                throw std::runtime_error("Can't load Texture");
            }
        }

        m_panelManager.addPanel<ScenePanel>();
        m_panelManager.addPanel<AssetsPanel>();
        m_panelManager.addPanel<InspectorPanel>(m_camera);
        m_panelManager.addPanel<MenuPanel>();
        m_panelManager.addPanel<ViewportPanel>(nullptr);
    }

    void Editor::prepare() {
        auto windowSize = m_window -> getSize();

        robot2D::FrameBufferSpecification frameBufferSpecification;
        frameBufferSpecification.size = windowSize.as<int>();

        m_frameBuffer = robot2D::FrameBuffer::Create(frameBufferSpecification);
        m_window -> setView({{0, 0}, windowSize.as<float>()});
        m_camera.resize({0, 0, windowSize.as<float>().x,
                         windowSize.as<float>().y});

        applyStyle(EditorStyle::GoldBlack);
    }

    void Editor::handleEvents(const robot2D::Event& event) {
        EventBinder m_eventBinder{event};
        m_eventBinder.Dispatch(robot2D::Event::KeyPressed,
                               [this](const robot2D::Event& event){
            if(event.key.code == robot2D::Key::LEFT_CONTROL)
                m_configuration.m_leftCtrlPressed = true;
            if(event.key.code == robot2D::Key::S) {
                if(m_configuration.m_leftCtrlPressed)
                    m_sceneManager.save(std::move(m_activeScene));
            }
        });

        m_eventBinder.Dispatch(robot2D::Event::KeyReleased,
                               [this](const robot2D::Event& event){
            if(event.key.code == robot2D::Key::LEFT_CONTROL)
                m_configuration.m_leftCtrlPressed = false;
        });


        m_camera.onEvent(event);
    }

    void Editor::handleMessages(const robot2D::Message& message) {}

    void Editor::update(float dt) {
        switch(m_state) {
            case State::Edit: {
                m_activeScene -> update(dt);
                break;
            }
            case State::Run: {
                m_activeScene -> updateRuntime(dt);
                break;
            }
        }
        m_panelManager.update(dt);
    }

    void Editor::render() {
        if(m_configuration.useGUI) {
            m_frameBuffer -> Bind();
            const auto& clearColor = m_panelManager.getPanel<InspectorPanel>().getColor();
            m_window -> clear(clearColor);
        }
        m_window -> beforeRender();
        m_window -> setView(m_camera.getView());

        for(auto& it: m_activeScene -> getEntities()) {
            if(!it.hasComponent<SpriteComponent>())
                continue;

            auto& sprite = it.getComponent<SpriteComponent>();
            auto transform = it.getComponent<TransformComponent>();

            robot2D::RenderStates renderStates;
            renderStates.texture = &sprite.getTexture();
            renderStates.transform *= transform.getTransform();
            renderStates.color = sprite.getColor();
            m_window -> draw(renderStates);
        }

        m_window -> afterRender();

        if(m_configuration.useGUI) {
            m_frameBuffer -> unBind();
            guiRender();
        }
    }

    void Editor::guiRender() {
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse
                | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport -> WorkPos);
        ImGui::SetNextWindowSize(viewport -> WorkSize);
        ImGui::SetNextWindowViewport(viewport -> ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.F);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.F);

        if (m_configuration.dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        ImGui::WindowOptions dockWindowOptions{};
        dockWindowOptions.flagsMask = window_flags;
        dockWindowOptions.name = "Scene";

        ImGui::createWindow(dockWindowOptions, [this]() {

            ImGui::PopStyleVar(2);

            ImGuiIO& io = ImGui::GetIO();
            if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
            {
                ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
                ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f),
                                 m_configuration.dockspace_flags);
            }

            // Todo like panels
            mainMenubar();
            ui_toolbar();

            auto stats = m_window -> getStats();
            m_panelManager.getPanel<InspectorPanel>().setRenderStats(std::move(stats));
            m_panelManager.render();
        });
    }

    //Todo Use Menubar to Open Projects, and not scenes
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
                    const char* path = tinyfd_openFileDialog("Load Scene", nullptr,
                                                       1,
                                                       patterns,
                                                       "Scene",
                                                       0);
                    if (path != nullptr) {
                        openPath = std::string(path);
                        openScene();
                    }
                }

                if(ImGui::MenuItem("Save", "Ctrl+Shift+S")) {
                    std::string savePath = "assets/scenes/demoScene.robot2D";
                    const char* patterns[1] = {"*.scene"};
                    const char* path = tinyfd_saveFileDialog("Load Scene", nullptr,
                                                       1, patterns, "Scene");
                    if(path != nullptr) {
                        savePath = std::string(path);
                    }
                }

                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
    }

    void Editor::ui_toolbar() {
        const auto& colors = ::ImGui::GetStyle().Colors;
        const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
        const auto& buttonActive = colors[ImGuiCol_ButtonActive];

        ImGui::WindowOptions windowOptions {
                {
                        {ImGuiStyleVar_WindowPadding, {0, 2}},
                        {ImGuiStyleVar_ItemInnerSpacing, {}}
                },
                {
                        {ImGuiCol_Button, robot2D::Color::fromGL(0.F, 0.F, 0.F, 0.F)},
                        {ImGuiCol_ButtonHovered, robot2D::Color::fromGL(buttonHovered.x,buttonHovered.z, 0.5F)},
                        {ImGuiCol_ButtonActive,robot2D::Color::fromGL(buttonActive.x,buttonActive.z, 0.5F)}
                }
        };
        windowOptions.name = "##toolbar";
        windowOptions.flagsMask = ImGuiWindowFlags_NoScrollbar;

        ImGui::createWindow(windowOptions, [this]() {
            auto texID = m_state == State::Edit ? EditorConfiguration::TextureID::RunButton
                                                : EditorConfiguration::TextureID::EditButton;
            float size = ::ImGui::GetWindowHeight() - 4.F;
            ImGui::SetCursorPosX((::ImGui::GetWindowContentRegionMax().x * 0.5F) - (size * 0.5F));
            if(ImGui::ImageButton(m_textures.get(texID), robot2D::vec2f{size, size}))
            {
                if (m_state == State::Edit) {
                    onSceneRun();
                    return;
                }

                if (m_state == State::Run) {
                    onSceneEdit();
                    return;
                }
            }
        });
    }

    void Editor::onSceneRun() {
        m_state = State::Run;
        // 1. load dll
        // 2. setup
    }

    void Editor::onSceneEdit() {
        m_state = State::Edit;
        // unload dll
    }


    bool Editor::createScene() {
        if(!m_sceneManager.add(std::move(m_currentProject))) {
            RB_EDITOR_ERROR("Can't Create Scene. Reason: {0}",
                            errorToString(m_sceneManager.getError()));
            return false;
        }
        m_activeScene = m_sceneManager.getActiveScene();
        openScene();

        return true;
    }

    void Editor::openScene() {
        auto& scenePanel = m_panelManager.getPanel<ScenePanel>();
        scenePanel.setActiveScene(m_activeScene);

        auto& viewportPanel = m_panelManager.getPanel<ViewportPanel>();
        viewportPanel.set(m_activeScene, m_frameBuffer);

        auto projectPath = m_currentProject -> getPath();
        auto assetsPath = combinePath(projectPath, "assets");
        auto& assetsPanel = m_panelManager.getPanel<AssetsPanel>();
        assetsPanel.setAssetsPath(assetsPath);
    }

    void Editor::createProject(Project::Ptr project) {
        m_currentProject = project;
        if(!m_sceneManager.add(std::move(project))) {
            RB_EDITOR_ERROR("Can't Create Scene. Reason: {0}",
                            errorToString(m_sceneManager.getError()));
            return;
        }
        m_activeScene = m_sceneManager.getActiveScene();
        openScene();
        m_window -> setMaximazed(true);
        prepare();
    }

    void Editor::loadProject(Project::Ptr project) {
        m_currentProject = project;

        if(!m_sceneManager.load(std::move(project))) {
            RB_EDITOR_ERROR("Can't Load Scene. Reason: {0}",
                            errorToString(m_sceneManager.getError()));
            return;
        }
        prepare();

        m_activeScene = m_sceneManager.getActiveScene();
        openScene();

        m_window -> setMaximazed(true);
    }
}