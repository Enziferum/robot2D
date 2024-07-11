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

#include <stdexcept>
#include <utility>

#include <robot2D/Core/Assert.hpp>
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
#include <editor/panels/UtilPanel.hpp>
#include <editor/panels/ViewportPanel.hpp>
#include <editor/panels/GameViewport.hpp>
///////////////////////////// PANELS /////////////////////////////

#include <editor/Components.hpp>
#include <editor/EditorStyles.hpp>
#include <editor/FileApi.hpp>

#include "imgui/imgui_internal.h"
#include "editor/panels/AnimationPanel.hpp"


namespace editor {

    namespace {
        robot2D::Spinner g_spinner;
    }

    IEditor::~IEditor() = default;

    Editor::Editor(robot2D::MessageBus& messageBus, robot2D::Gui& gui):
    m_window{nullptr},
    m_messageBus{messageBus},
    m_panelManager{gui},
    m_configuration{},
    m_frameBuffer{nullptr},
    m_cameraManipulator{messageBus}
    {}

    void Editor::setupBindings() {
        auto dropCallback = [this](std::vector<std::string>&& paths) {
            for(auto& observer: m_observers)
                observer -> notify(paths);
        };
        m_window -> addDropCallback(std::move(dropCallback));
        m_observers.emplace_back(&m_panelManager.getPanel<AssetsPanel>());

        m_eventBinder.bindEvent(robot2D::Event::KeyPressed, BIND_CLASS_FN(onKeyPressed));
        m_eventBinder.bindEvent(robot2D::Event::KeyReleased, BIND_CLASS_FN(onKeyReleased));
        m_eventBinder.bindEvent(robot2D::Event::Resized, [this](const robot2D::Event& evt) {
            m_frameBuffer -> Resize({evt.size.widht,evt.size.heigth});
        });
        m_eventBinder.bindEvent(robot2D::Event::MousePressed, BIND_CLASS_FN(onMousePressed));
        m_eventBinder.bindEvent(robot2D::Event::MouseReleased, BIND_CLASS_FN(onMouseReleased));
        m_eventBinder.bindEvent(robot2D::Event::MouseMoved, BIND_CLASS_FN(onMouseMoved));

  /*      m_cameraManipulator.setButtonCallback([this](SceneEntity entity) {
            m_interactor -> setMainCamera(entity);
            auto* msg = m_messageBus.postMessage<PanelEntitySelectedMessage>(MessageID::PanelEntitySelected);
            msg -> entity = entity;
        });*/
    }

    void Editor::setupShortCuts() {
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
            m_interactor -> duplicateEntity(mousePos);
        });
        m_shortcutManager.bind(std::make_pair(EditorShortCutType::Duplicate, duplicateEntity));


        ShortCut undoCommand{robot2D::Key::LEFT_CONTROL, robot2D::Key::Z};
        undoCommand.setCallback([this]() {
            m_interactor -> undoCommand();
        });
        m_shortcutManager.bind(std::make_pair(EditorShortCutType::Undo, undoCommand));
    }

    void Editor::setupSpinner() {
        g_spinner.setRadius(16.f);
        g_spinner.setThickness(6.f);
        g_spinner.setSpeed(6.f);
        g_spinner.setAngle(270.f / 360.f * 2 * IM_PI);
        g_spinner.setColor(robot2D::Color::White);
        g_spinner.setBackgroundColor(robot2D::Color(255.f, 255.f, 255.f, 0.f));
        g_spinner.setLabel("Spinner");
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
        auto scriptInteractor = editorInteractor -> getScriptInteractor();

        /// TODO(a.raag): assert with FILENAME LINE
        assert(scriptInteractor && "scriptInteractor must be valid");
        scriptInteractor -> setEditorCamera(m_editorCamera);



        m_panelManager.addPanel<ScenePanel>(m_messageBus, m_messageDispather, m_prefabManager);
        m_panelManager.addPanel<AssetsPanel>(m_messageBus, m_panelManager, m_prefabManager);
        m_panelManager.addPanel<InspectorPanel>(m_messageDispather, m_messageBus, m_prefabManager, m_panelManager);
        m_panelManager.addPanel<UtilPanel>(m_editorCamera);
        m_panelManager.addPanel<MenuPanel>(m_messageBus, m_interactor);
        m_panelManager.addPanel<ViewportPanel>(m_interactor,
                                               m_editorCamera,
                                               m_messageBus,
                                               m_messageDispather,
                                               m_guizmo2D,
                                               m_cameraManipulator,
                                               m_selectionCollider);

        m_panelManager.addPanel<GameViewport>(m_messageBus);
        m_panelManager.addPanel<AnimationPanel>(m_messageBus, m_messageDispather, m_interactor);

        setupBindings();
        setupShortCuts();
        setupSpinner();

        m_sceneGrid.setup();
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

        if(m_interactor -> getState() == EditorState::Edit) {
            m_editorCamera -> handleEvents(event);
            m_shortcutManager.handleEvents(event);
            m_guizmo2D.handleEvents(event);
            m_cameraManipulator.handleEvents(event);
            m_panelManager.getPanel<ViewportPanel>().handleEvents(event);
            m_eventBinder.handleEvents(event);
        }
        else if(m_interactor -> getState() == EditorState::Run) {
            m_interactor -> handleEventsRuntime(event);
        }
    }

    void Editor::handleMessages(const robot2D::Message& message) {
        robot2D::Message* msg = const_cast<robot2D::Message*>(&message);
        m_messageDispather.process(*msg);
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
                m_cameraManipulator.update(m_window -> getMousePos(), dt);
                m_shortcutManager.update();
                break;
            }
            case EditorState::Run: {
                m_interactor -> update(dt);
                break;
            }
        }
        m_panelManager.update(dt);
    }

    void Editor::render() {
        if(m_configuration.useGUI) {
            if(m_frameBuffer)
                m_frameBuffer -> Bind();
            const auto& clearColor = m_panelManager.getPanel<UtilPanel>().getColor();
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
        m_window -> draw(m_cameraManipulator);
        if(m_selectionCollider.isShown())
            m_window -> draw(m_selectionCollider);

        if(m_objectManipulator.isShown())
            m_window -> draw(m_objectManipulator);

        m_window -> afterRender();

        if(m_configuration.useGUI) {
            if(m_frameBuffer)
                m_frameBuffer -> unBind();

            m_gameFrameBuffer -> Bind();
            const auto& clearColor = m_panelManager.getPanel<UtilPanel>().getColor();
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
        auto stats = m_window -> getStats();
        m_panelManager.getPanel<UtilPanel>().setRenderStats(std::move(stats));
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
    }


    void Editor::prepareView() {
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
        m_cameraManipulator.setCamera(m_editorCamera);
        m_editorCamera -> setViewportSize(windowSize.as<float>());

        applyStyle(EditorStyle::UE4);

#ifdef ROBOT2D_LINUX
        m_window -> setResizable(true);
        m_window -> setSize({2560, 1920});
#else
        m_window -> setMaximazed(true);
#endif
        auto& viewportPanel = m_panelManager.getPanel<ViewportPanel>();
        viewportPanel.setFramebuffer(m_frameBuffer);

        m_panelManager.getPanel<GameViewport>().setFrameBuffer(m_gameFrameBuffer);
        m_needPrepareView = false;
    }


    void Editor::openScene(Scene::Ptr scene, std::string path) {
        if(m_needPrepareView)
            prepareView();

        m_activeScene = std::move(scene);
        auto& scenePanel = m_panelManager.getPanel<ScenePanel>();
        scenePanel.setInteractor(m_interactor);

        auto& inspectorPanel = m_panelManager.getPanel<InspectorPanel>();
        inspectorPanel.setInteractor(m_interactor);

        auto& viewportPanel = m_panelManager.getPanel<ViewportPanel>();
        viewportPanel.setFramebuffer(m_frameBuffer);

        auto assetsPath = combinePath(path, "assets");
        auto& assetsPanel = m_panelManager.getPanel<AssetsPanel>();
        assetsPanel.setAssetsPath(assetsPath);
        assetsPanel.switchState(AssetsPanel::State::Active);

        m_interactor -> setEditorCamera(m_editorCamera);
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


    void Editor::onMouseMoved(const robot2D::Event& event) {
        auto& viewportPanel = m_panelManager.getPanel<ViewportPanel>();
        if(m_leftMousePressed && viewportPanel.isActive()) {
            robot2D::vec2f movePos{event.move.x, event.move.y};
            movePos = m_editorCamera -> convertPixelToCoords(movePos);
            auto pos = m_selectionCollider.getPosition();
            m_selectionCollider.setSize({movePos.x - pos.x, movePos.y - pos.y});
        }

    }

    void Editor::onMousePressed(const robot2D::Event& event) {
        auto& viewportPanel = m_panelManager.getPanel<ViewportPanel>();
        if(event.mouse.btn == robot2D::mouse2int(robot2D::Mouse::MouseLeft)
                        && !m_interactor -> hasSelectedEntities() && !m_guizmo2D.isActive() && viewportPanel.isActive()) {
            m_leftMousePressed = true;
            auto mousePos = m_editorCamera -> convertPixelToCoords(
                {static_cast<float>(event.mouse.x), static_cast<float>(event.mouse.y)
            });
            m_selectionCollider.setPosition(mousePos);
            m_selectionCollider.setIsShown(true);
        }
    }

    void Editor::onMouseReleased(const robot2D::Event& event) {
        auto& viewportPanel = m_panelManager.getPanel<ViewportPanel>();
        if(m_leftMousePressed &&
            event.mouse.btn == robot2D::mouse2int(robot2D::Mouse::MouseLeft) && viewportPanel.isActive()
                                                                                && !m_guizmo2D.isActive()) {
            m_leftMousePressed = false;
            m_selectionCollider.setIsShown(false);
            m_interactor -> findSelectEntities(m_selectionCollider.getRect());
            m_selectionCollider.reset();
        }
    }


    void Editor::findSelectedEntitiesOnUI(std::vector<ITreeItem::Ptr>&& items) {
        auto& scenePanel = m_panelManager.getPanel<ScenePanel>();
        scenePanel.processSelectedEntities(std::move(items));

        auto& inspectorPanel = m_panelManager.getPanel<InspectorPanel>();
        /// TODO(a.raag): if has more than 1 entity ??
        //inspectorPanel.setSelected(entities.back());
    }

    DeletedEntitiesRestoreUIInformation
    Editor::removeEntitiesOnUI(std::list<ITreeItem::Ptr>&& uiItems) {
        auto& panel = m_panelManager.getPanel<ScenePanel>();
        return panel.removeEntitiesOnUI(std::move(uiItems));
    }

    void Editor::restoreEntitiesOnUI(DeletedEntitiesRestoreUIInformation& restoreUiInformation) {
        auto& panel = m_panelManager.getPanel<ScenePanel>();
        panel.restoreEntitiesOnUI(restoreUiInformation);
    }

    void Editor::clearSelectionOnUI() {
        auto& scenePanel = m_panelManager.getPanel<ScenePanel>();
        scenePanel.clearSelection();

        auto& inspectorPanel = m_panelManager.getPanel<InspectorPanel>();
        inspectorPanel.clearSelection();
    }


    void Editor::setMainCameraEntity(SceneEntity entity) {
        m_cameraManipulator.setManipulatedEntity(entity);
    }


    void Editor::showPopup(PopupConfiguration* configuration) {
        m_popupConfiguration = configuration;
        auto* manager = PopupManager::getManager();
        manager -> beginPopup(this);
    }

    void Editor::onPopupRender() {
        if(!m_popupConfiguration)
            return;

        ImVec2 center = ImGui::GetMainViewport() -> GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::OpenPopup(m_popupConfiguration -> title.c_str());
        auto* manager = PopupManager::getManager();

        if (ImGui::BeginPopupModal(m_popupConfiguration -> title.c_str(), nullptr,
                                   ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Scene has changed. Want to save?");
            ImGui::Separator();

            if (ImGui::Button("Yes", ImVec2(120, 0))) {
                // setMouseHoverDirectly(false);
                m_popupConfiguration -> onYes();
                m_popupConfiguration = nullptr;
                manager -> endPopup();
                ImGui::CloseCurrentPopup();
            }

            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("No", ImVec2(120, 0))) {
                // setMouseHoverDirectly(false);
                m_popupConfiguration -> onNo();
                m_popupConfiguration = nullptr;
                manager -> endPopup();
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

}