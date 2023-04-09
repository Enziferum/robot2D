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

#include <stdexcept>
#include <chrono>
#include <utility>

#include <robot2D/imgui/Gui.hpp>
#include <robot2D/imgui/Api.hpp>

#include <editor/Editor.hpp>
#include <editor/EventBinder.hpp>
#include <editor/scripting/ScriptingEngine.hpp>

///////////////////////////// PANELS /////////////////////////////
#include <editor/panels/ScenePanel.hpp>
#include <editor/panels/AssetsPanel.hpp>
#include <editor/panels/MenuPanel.hpp>
#include <editor/panels/InspectorPanel.hpp>
#include <editor/panels/ViewportPanel.hpp>
#include <editor/panels/GameViewport.hpp>
///////////////////////////// PANELS /////////////////////////////

#include <editor/Components.hpp>
#include <editor/EditorStyles.hpp>
#include <editor/FileApi.hpp>

#include "imgui/imgui_internal.h"



namespace editor {


    namespace fs = std::filesystem;

    namespace {
        robot2D::Spinner g_spinner;
    }

    IEditor::~IEditor() = default;

    Editor::Editor(robot2D::MessageBus& messageBus, robot2D::Gui& gui):
    m_window{nullptr},
    m_messageBus{messageBus},
    m_panelManager{gui},
    m_configuration{},
    m_frameBuffer{nullptr}
    {}

    void Editor::setupBindings() {
        auto dropCallback = [this](std::vector<std::string>&& paths) {
            m_interactor -> notifyObservers(std::move(paths));
        };
        m_window -> addDropCallback(std::move(dropCallback));
        m_interactor -> addObserver(&m_panelManager.getPanel<AssetsPanel>());

        m_eventBinder.bindEvent(robot2D::Event::KeyPressed, BIND_CLASS_FN(onKeyPressed));
        m_eventBinder.bindEvent(robot2D::Event::KeyReleased, BIND_CLASS_FN(onKeyReleased));
        m_eventBinder.bindEvent(robot2D::Event::Resized,
                             [this](const robot2D::Event& evt) {
                                 RB_EDITOR_INFO("New Size = {0} and {1}", evt.size.widht, evt.size.heigth);
                                 m_frameBuffer -> Resize({evt.size.widht,evt.size.heigth});
                             });
        m_eventBinder.bindEvent(robot2D::Event::MousePressed, BIND_CLASS_FN(onMousePressed));
        m_eventBinder.bindEvent(robot2D::Event::MouseReleased, BIND_CLASS_FN(onMouseReleased));

        ShortCut guizmoMove{robot2D::Key::LEFT_SHIFT, robot2D::Key::W};
        guizmoMove.setCallback([this]() {
            m_guizmo2D.setOperationType(Guizmo2D::Operation::Move);
        });
        m_shortcutManager.bind(std::make_pair(EditorShortCutType::GizmoMove, guizmoMove));

        ShortCut guizmoScale{robot2D::Key::LEFT_SHIFT, robot2D::Key::Q};
        guizmoScale.setCallback([this]() {
            m_guizmo2D.setOperationType(Guizmo2D::Operation::Scale);
        });
        m_shortcutManager.bind(std::make_pair(EditorShortCutType::GizmoScale, guizmoScale));

        ShortCut saveScene{robot2D::Key::LEFT_CONTROL, robot2D::Key::S};
        saveScene.setCallback([this]() {
            m_interactor -> saveScene();
        });
        m_shortcutManager.bind(std::make_pair(EditorShortCutType::SaveScene, saveScene));

        ShortCut copyBuffer{robot2D::Key::LEFT_CONTROL, robot2D::Key::C};
        copyBuffer.setCallback([this]() {
            m_interactor -> copyToBuffer();
        });
        m_shortcutManager.bind(std::make_pair(EditorShortCutType::Copy, copyBuffer));

        ShortCut pasteBuffer{robot2D::Key::LEFT_CONTROL, robot2D::Key::V};
        pasteBuffer.setCallback([this]() {
            m_interactor -> pasterFromBuffer();
        });
        m_shortcutManager.bind(std::make_pair(EditorShortCutType::Paste, pasteBuffer));


        ShortCut duplicateEntity{robot2D::Key::LEFT_CONTROL, robot2D::Key::D};
        duplicateEntity.setCallback([this]() {
           auto mousePos = m_window -> getMousePos();
           mousePos = m_editorCamera -> convertPixelToCoords(mousePos);
           m_interactor -> duplicateEntity(mousePos, m_panelManager.getSelectedEntity());
        });
        // TODO(a.raag): clone Entity possibility
        m_shortcutManager.bind(std::make_pair(EditorShortCutType::Duplicate, duplicateEntity));


        ShortCut undoCommand{robot2D::Key::LEFT_CONTROL, robot2D::Key::Z};
        undoCommand.setCallback([this]() {
            m_interactor -> undoCommand();
        });
        // TODO(a.raag): clone Entity possibility
        m_shortcutManager.bind(std::make_pair(EditorShortCutType::Undo, undoCommand));
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
        m_gameFrameBuffer = robot2D::FrameBuffer::Create(frameBufferSpecification);
        m_window -> setView({{0, 0}, windowSize.as<float>()});

        m_editorCamera -> setFrameBuffer(m_frameBuffer);
        m_guizmo2D.setCamera(m_editorCamera);
        m_selectionBox.setCamera(m_editorCamera);
        m_editorCamera -> setViewportSize(windowSize.as<float>());

        applyStyle(EditorStyle::UE4);
        m_needPrepare = false;
        m_window -> setMaximazed(true);
        auto& viewportPanel = m_panelManager.getPanel<ViewportPanel>();
        viewportPanel.set(m_frameBuffer);

        m_panelManager.getPanel<GameViewport>().setFrameBuffer(m_gameFrameBuffer);
    }

    void Editor::setup(robot2D::RenderWindow* window, EditorInteractor* editorInteractor) {
        if(m_window == nullptr)
            m_window = window;

        m_interactor = editorInteractor;

        for(auto& path: m_configuration.texturePaths) {
            auto fullPath = m_configuration.texturesPath + path.second;
            if(!m_textures.loadFromFile(path.first, fullPath)) {
                throw std::runtime_error("Can't load Texture");
            }
        }

        m_editorCamera = std::make_shared<EditorCamera2D>();

        m_panelManager.addPanel<ScenePanel>(m_messageBus, m_messageDispather, m_prefabManager);
        m_panelManager.addPanel<AssetsPanel>(m_messageBus, m_panelManager, m_prefabManager);
        m_panelManager.addPanel<InspectorPanel>(m_editorCamera);
        m_panelManager.addPanel<MenuPanel>(m_messageBus);
        m_panelManager.addPanel<ViewportPanel>(m_panelManager,
                                               m_editorCamera,
                                               m_messageBus,
                                               m_messageDispather,
                                               m_guizmo2D,
                                               m_selectionBox);

        m_panelManager.addPanel<GameViewport>(m_messageBus);

        setupBindings();
        m_sceneGrid.setup();

        g_spinner.setRadius(16.f);
        g_spinner.setThickness(6.f);
        g_spinner.setSpeed(6.f);
        g_spinner.setAngle(270.f / 360.f * 2 * IM_PI);
        g_spinner.setColor(robot2D::Color::White);
        g_spinner.setBackgroundColor(robot2D::Color(255.f, 255.f, 255.f, 0.f));
        g_spinner.setLabel("Spinner");

        m_sceneRender.setup();
    }

    void Editor::handleEvents(const robot2D::Event& event) {
        if(event.type == robot2D::Event::LostFocus) {
            m_state = State::LostFocus;
        }
        if(event.type == robot2D::Event::GainFocus) {
            m_state = State::HasFocus;
        }

        if(m_state == State::LostFocus)
            return;

        m_eventBinder.handleEvents(event);
        if(m_interactor -> getState() != EditorState::Edit) {
            return;
        }

        m_editorCamera -> handleEvents(event);
        m_guizmo2D.handleEvents(event);
        m_selectionBox.handleEvents(event);
        m_shortcutManager.handleEvents(event);
        m_panelManager.getPanel<ViewportPanel>().handleEvents(event);
    }

    void Editor::handleMessages(const robot2D::Message& message) {
        m_messageDispather.process(message);
    }

    void Editor::update(float dt) {
        if(m_state == State::LostFocus)
            return;

        switch(m_interactor -> getState()) {
            default:
                break;
            case EditorState::Load:
                break;
            case EditorState::Edit: {
                m_interactor -> update(dt);
                m_panelManager.getPanel<ViewportPanel>().update(dt);
                m_selectionBox.update(m_window -> getMousePos(), dt);
                m_shortcutManager.update();
                break;
            }
            case EditorState::Run: {
                m_interactor -> update(dt);
                break;
            }
        }
        m_panelManager.update(dt);
        if(m_activeScene)
            m_sceneRender.update(m_activeScene -> getEntities());
    }

    void Editor::render() {
        if(m_configuration.useGUI) {
            if(m_frameBuffer)
                m_frameBuffer -> Bind();
            const auto& clearColor = m_panelManager.getPanel<InspectorPanel>().getColor();
            // TODO: @a.raag reset stats
            m_window -> clear(clearColor);
        }

        m_sceneGrid.render(m_frameBuffer -> getSpecification().size);
        m_window -> beforeRender();
        if (m_editorCamera -> getType() == EditorCameraType::Orthographic)
            m_window -> setView(m_editorCamera -> getView());
        else if (m_editorCamera -> getType() == EditorCameraType::Perspective)
            m_window -> setView3D(m_editorCamera -> getProjectionMatrix(), m_editorCamera -> getViewMatrix());

        if(m_activeScene) {
            m_activeScene -> setRuntimeCamera(false);
            m_window -> draw(*m_activeScene);
        }
        m_window -> draw(m_guizmo2D);
        m_window -> draw(m_selectionBox);
        m_window -> afterRender();

        if(m_configuration.useGUI) {
            if(m_frameBuffer) {
                m_frameBuffer -> unBind();
            }

            m_gameFrameBuffer -> Bind();
            const auto& clearColor = m_panelManager.getPanel<InspectorPanel>().getColor();
            m_window -> clear(clearColor);
            if(m_activeScene) {
                m_window -> beforeRender();
                m_activeScene -> setRuntimeCamera(true);
                m_window -> draw(*m_activeScene);
                m_window -> afterRender();
            }
            m_gameFrameBuffer -> unBind();

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

        // TODO(a.raag): Scoped StyleVars
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.F);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.F);

        if (m_configuration.dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        robot2D::WindowOptions dockWindowOptions{};
        dockWindowOptions.flagsMask = window_flags;
        dockWindowOptions.name = "Scene";
        ImGui::PopStyleVar(2);

        robot2D::createWindow(dockWindowOptions, BIND_CLASS_FN(windowFunction));
    }

    void Editor::windowFunction() {
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f),
                             m_configuration.dockspace_flags);
        }

        auto stats = m_window -> getStats();
        m_panelManager.getPanel<InspectorPanel>().setRenderStats(std::move(stats));
        m_panelManager.render();

        if(m_interactor -> getState() == EditorState::Load) {
            ImGui::OpenPopup("LoadingProject");

            ImVec2 center = ImGui::GetMainViewport() -> GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            ImGuiContext* g = ImGui::GetCurrentContext();
            auto size = g_spinner.calculateSize(g -> Style);
            size *= 2.f;
            ImGui::SetNextWindowSize(size, ImGuiCond_Appearing);

            if (ImGui::BeginPopupModal("LoadingProject", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                auto contentSize = ImGui::GetContentRegionAvail();
                g_spinner.setPosition({ (contentSize.x) / 2.f, (contentSize.y) / 2.f});
                g_spinner.setAngle(270.f / 360.f * 2 * IM_PI);
                g_spinner.draw();
                ImGui::EndPopup();
            }
        }

        if(m_interactor -> getState() == EditorState::Modal && m_popupConfiguration) {
            ImVec2 center = ImGui::GetMainViewport() -> GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            ImGui::OpenPopup(m_popupConfiguration -> title.c_str());

            if (ImGui::BeginPopupModal(m_popupConfiguration -> title.c_str(), nullptr,
                                       ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("Scene has changed. Want to save?");
                ImGui::Separator();

                if (ImGui::Button("Yes", ImVec2(120, 0))) {
                    // setMouseHoverDirectly(false);
                    m_popupConfiguration -> onYes();
                    m_popupConfiguration = nullptr;
                    ImGui::CloseCurrentPopup();
                }

                ImGui::SetItemDefaultFocus();
                ImGui::SameLine();
                if (ImGui::Button("No", ImVec2(120, 0))) {
                    // setMouseHoverDirectly(false);
                    m_popupConfiguration -> onNo();
                    m_popupConfiguration = nullptr;
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }
        }

    }

    void Editor::openScene(Scene::Ptr scene, std::string path) {
        if(m_needPrepare)
            prepare();

        m_activeScene = std::move(scene);
        auto& scenePanel = m_panelManager.getPanel<ScenePanel>();
        scenePanel.setInteractor(m_interactor);

        auto& viewportPanel = m_panelManager.getPanel<ViewportPanel>();
        viewportPanel.set(m_frameBuffer);

        auto assetsPath = combinePath(path, "assets");
        auto& assetsPanel = m_panelManager.getPanel<AssetsPanel>();
        assetsPanel.setAssetsPath(assetsPath);
        assetsPanel.unlock();
    }

    void Editor::onKeyPressed(const robot2D::Event& event) {
        if(event.key.code == robot2D::Key::LEFT_CONTROL)
            m_configuration.m_leftCtrlPressed = true;
        if(event.key.code == robot2D::Key::DEL) {
            m_interactor -> removeSelectedEntities();
        }
    }

    void Editor::onKeyReleased(const robot2D::Event& event) {
        if(event.key.code == robot2D::Key::LEFT_CONTROL)
            m_configuration.m_leftCtrlPressed = false;
        
    }

    void Editor::showPopup(PopupConfiguration* configuration) {
        m_popupConfiguration = configuration;
    }

    void Editor::onMousePressed(const robot2D::Event& event) {
        // no - op
    }

    void Editor::onMouseReleased(const robot2D::Event& event) {
        // no - op

        robot2D::FloatRect rectZone;
        m_interactor -> findSelectEntities(rectZone);
    }

    void Editor::findSelectedEntitiesOnUI(std::vector<robot2D::ecs::Entity>& entities) {
        auto& scenePanel = m_panelManager.getPanel<ScenePanel>();
        scenePanel.processSelectedEntities(entities);
    }

}