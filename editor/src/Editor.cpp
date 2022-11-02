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
#include <chrono>
#include <utility>

#include <imgui/imgui.h>
#include <robot2D/Extra/Api.hpp>

#include <editor/Editor.hpp>
#include <editor/EventBinder.hpp>
// panels
#include <editor/ScenePanel.hpp>
#include <editor/AssetsPanel.hpp>
#include <editor/MenuPanel.hpp>
#include <editor/InspectorPanel.hpp>
#include <editor/ViewportPanel.hpp>

#include <editor/SceneSerializer.hpp>
#include <editor/Components.hpp>
#include <editor/EditorStyles.hpp>
#include <editor/FileApi.hpp>
#include <editor/Task.hpp>

#include "imgui/imgui_internal.h"

namespace editor {


    bool SpinnerBegin(const char *label, float radius, ImVec2 &pos, ImVec2 &size, ImVec2 &centre) {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        auto& io = ImGui::GetIO();

        ImGuiContext* g = ImGui::GetCurrentContext();
        const ImGuiStyle &style = g -> Style;
        const ImGuiID id = window->GetID(label);

        pos = window->DC.CursorPos;
        size = ImVec2((radius) * 2, (radius + style.FramePadding.y) * 2);

        const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
        ImGui::ItemSize(bb, style.FramePadding.y);

        centre = bb.GetCenter();
        if (!ImGui::ItemAdd(bb, id))
            return false;

        return true;
    }

#define SPINNER_HEADER(pos, size, centre) ImVec2 pos, size, centre; if (!SpinnerBegin(label, radius, pos, size, centre)) { return; }; \
    ImGuiWindow* window = ImGui::GetCurrentWindow();                                                                                  \
    \

    void SpinnerAng(const char* label,
                    float radius,
                    float thickness,
                    const ImColor &color = 0xffffffff,
                    const ImColor &bg = 0xffffff80,
                    float speed = 2.8f, float angle = M_PI)
    {
        SPINNER_HEADER(pos, size, centre);

        // Render
        window->DrawList->PathClear();
        const size_t num_segments = window->DrawList->_CalcCircleAutoSegmentCount(radius);
        float start = (float)ImGui::GetTime()* speed;
        const float bg_angle_offset = M_PI * 2.f / num_segments;
        for (size_t i = 0; i <= num_segments; i++)
        {
            const float a = start + (i * bg_angle_offset);
            window->DrawList->PathLineTo(ImVec2(centre.x + std::cos(a) * radius, centre.y + std::sin(a) * radius));
        }
        window->DrawList->PathStroke(bg, false, thickness);

        window->DrawList->PathClear();
        const float angle_offset = angle / num_segments;
        for (size_t i = 0; i < num_segments; i++)
        {
            const float a = start + (i * angle_offset);
            window->DrawList->PathLineTo(ImVec2(centre.x + std::cos(a) * radius, centre.y + std::sin(a) * radius));
        }
        window->DrawList->PathStroke(color, false, thickness);
    }

    namespace fs = std::filesystem;

    Editor::Editor(robot2D::MessageBus& messageBus, TaskQueue& taskQueue, ImGui::Gui& gui):
    m_state(State::Edit),
    m_window{nullptr},
    m_messageBus{messageBus},
    m_taskQueue{taskQueue},
    m_gui{gui},
    m_panelManager{gui},
    m_sceneManager{m_messageBus},
    m_configuration{},
    m_currentProject{nullptr},
    m_activeScene{nullptr},
    m_frameBuffer{nullptr}
  {}

    void Editor::setup(robot2D::RenderWindow* window) {
        if(m_window == nullptr)
            m_window = window;

        for(auto& it: m_configuration.texturePaths) {
            auto fullPath = m_configuration.texturesPath + it.second;
            if(!m_textures.loadFromFile(it.first, fullPath)) {
                throw std::runtime_error("Can't load Texture");
            }
        }

        m_panelManager.addPanel<ScenePanel>(m_messageDispather);
        m_panelManager.addPanel<AssetsPanel>();
        m_panelManager.addPanel<InspectorPanel>(m_editorCamera);
        m_panelManager.addPanel<MenuPanel>(m_messageBus);
        m_panelManager.addPanel<ViewportPanel>(m_panelManager, m_editorCamera, m_messageBus, nullptr);
    }

    void Editor::handleEvents(const robot2D::Event& event) {
        EventBinder m_eventBinder{event};
        //TODO: @a.raag add short-cut binder
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

        EventBinder eventBinder{event};

        eventBinder.Dispatch(robot2D::Event::Resized,
                             [this](const robot2D::Event& evt) {
            RB_EDITOR_INFO("New Size = {0} and {1}", evt.size.widht, evt.size.heigth);
            m_frameBuffer -> Resize({evt.size.widht,evt.size.heigth});
        });

        m_editorCamera.handleEvents(event);
    }

    void Editor::handleMessages(const robot2D::Message& message) {
        m_messageDispather.process(message);
    }

    void Editor::update(float dt) {
        switch(m_state) {
            case State::Load:
                break;
            case State::Edit: {
                m_activeScene -> update(dt);
                m_panelManager.getPanel<ViewportPanel>().update(dt);
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
            if(m_frameBuffer)
                m_frameBuffer -> Bind();
            const auto& clearColor = m_panelManager.getPanel<InspectorPanel>().getColor();
            // TODO: @a.raag reset stats
            m_window -> clear(clearColor);
        }

        m_window -> beforeRender();
        m_window -> setView3D(m_editorCamera.getProjectionMatrix(), m_editorCamera.getViewMatrix());

        if(m_activeScene) {
            for(auto& it: m_activeScene -> getEntities()) {
                if(!it.hasComponent<SpriteComponent>())
                    continue;

                auto& sprite = it.getComponent<SpriteComponent>();
                robot2D::RenderStates renderStates;
                renderStates.texture = &sprite.getTexture();

                if(it.hasComponent<TransformComponent>()) {
                    auto transform = it.getComponent<TransformComponent>();
                    renderStates.transform *= transform.getTransform();
                }
                else if(it.hasComponent<Transform3DComponent>()) {
                    auto transform = it.getComponent<Transform3DComponent>();
                    renderStates.transform3D = transform.getTransform();
                }

                renderStates.color = sprite.getColor();
                renderStates.entityID = it.getIndex();
                m_window -> draw(renderStates);
            }
        }

        m_window -> afterRender();

        if(m_configuration.useGUI) {
            guiRender();
            if(m_frameBuffer) {
                m_frameBuffer -> unBind();
            }
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

            ui_toolbar();

            auto stats = m_window -> getStats();
            m_panelManager.getPanel<InspectorPanel>().setRenderStats(std::move(stats));
            m_panelManager.render();

            if(m_state == State::Load) {
                ImGui::OpenPopup("LoadingProject");

                ImVec2 center = ImGui::GetMainViewport() -> GetCenter();
                ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

                if (ImGui::BeginPopupModal("LoadingProject", NULL, ImGuiWindowFlags_MenuBar))
                {
                    SpinnerAng("SpinnerAng270NoBg", 16, 6, ImColor(255, 255, 255),
                               ImColor(255, 255, 255, 0),
                               6 * 1.f, 270.f / 360.f * 2 * IM_PI );
                    ImGui::EndPopup();
                }
            }
        });
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

    void Editor::createProject(Project::Ptr project) {
        m_currentProject = project;
        if(!m_sceneManager.add(std::move(project))) {
            RB_EDITOR_ERROR("Can't Create Scene. Reason: {0}",
                            errorToString(m_sceneManager.getError()));
            return;
        }
        m_activeScene = m_sceneManager.getActiveScene();
        m_window -> setMaximazed(true);

        prepare();
        openScene();
    }

    class SceneLoadTask: public ITask {
    public:
        SceneLoadTask(ITaskFunction::Ptr function,
                      SceneManager& sceneManager,
                      Project::Ptr project):
                ITask{std::move(function)},
                m_sceneManager{sceneManager},
                m_project{std::move(project)}{}
        ~SceneLoadTask() override = default;

        void execute() override {
            if(!m_sceneManager.load(std::move(m_project))) {
                RB_EDITOR_ERROR("Can't Load Scene. Reason: {0}",
                                errorToString(m_sceneManager.getError()));
                return;
            }
        }
    private:
        SceneManager& m_sceneManager;
        Project::Ptr m_project;
    };


    void Editor::loadProject(Project::Ptr project) {
        m_currentProject = project;
        //m_state = State::Load;
//
//        auto loadLambda = [this](const SceneLoadTask& task) {
//            m_activeScene = m_sceneManager.getActiveScene();
//            m_window -> setMaximazed(true);
//
//            prepare();
//            openScene();
//            m_state = State::Edit;
//        };
//
//        m_taskQueue.addAsyncTask<SceneLoadTask>(loadLambda,
//                                              m_sceneManager, project);

        if(!m_sceneManager.load(std::move(project))) {
            RB_EDITOR_ERROR("Can't Load Scene. Reason: {0}",
                            errorToString(m_sceneManager.getError()));
            return;
        }

        m_activeScene = m_sceneManager.getActiveScene();
        m_window -> setMaximazed(true);

        prepare();
        openScene();
        m_state = State::Edit;
    }

    void Editor::prepare() {
        auto windowSize = m_window -> getSize();

        robot2D::FrameBufferSpecification frameBufferSpecification;
        frameBufferSpecification.attachments = {
                robot2D::FrameBufferTextureFormat::RGBA8,
                robot2D::FrameBufferTextureFormat::RED_INTEGER,
                robot2D::FrameBufferTextureFormat::Depth
        };
        frameBufferSpecification.size = windowSize.as<int>();

        m_frameBuffer = robot2D::FrameBuffer::Create(frameBufferSpecification);
        m_window -> setView({{0, 0}, windowSize.as<float>()});
        m_editorCamera.setViewportSize(windowSize.as<float>());

        applyStyle(EditorStyle::GoldBlack);
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
}