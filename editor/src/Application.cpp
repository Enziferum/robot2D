/*********************************************************************
   (c) Alex Raag 2025
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

#include <editor/Application.hpp>
#include <editor/scripting/ScriptingEngine.hpp>

#include <editor/EditorStyles.hpp>
#include <editor/Utils.hpp>
#include <editor/EventBinder.hpp>
#include <editor/PopupManager.hpp>
#include <editor/Exception.hpp>

#include <imgui/imgui.h>

#include <editor/FileApi.hpp>
#include <editor/LocalResourceManager.hpp>
#include <robot2D/imgui/GuiFontConfig.hpp>
#include "IconsFontsAwesome5.hpp"

#include <editor/EditorConfig.hpp>


namespace editor {


    namespace {
        const std::string configPath = "robot2D.ini";

        int getDPI() {
#ifdef _WIN32
            const HDC hdc = GetDC(nullptr);
            if(!hdc)
                return 96;
            const int dpi = GetDeviceCaps(hdc, LOGPIXELSX);
            ReleaseDC(nullptr, hdc);
            return dpi > 0 ? dpi : 96;
#else
            return 96;
#endif
        }


    }



    Application::Application() :
        robot2D::Application(),
        m_appConfiguration{},
        m_messageDispatcher{},
        m_logic{m_messageDispatcher},
        m_guiWrapper{},
        m_projectInspector{m_messageBus} {
    }


    void Application::setup() {
        RB_EDITOR_INFO("[Application] Setup started");
        {
            robot2D::Image iconImage;
            iconImage.loadFromFile(m_appConfiguration.logoPath);
            m_window -> setIcon(std::move(iconImage));
        }


        {
            /// TODO(a.raag): read from config .ini
            std::string defaultFontPath = "res/fonts/notosans-regular.ttf";
            std::string fontPath1 = std::string{"res/fonts/fa-regular-400.ttf"};
            std::string fontPath2 = std::string{"res/fonts/fa-solid-900.ttf"};
            float fontSize = 18.f;
            float scaleFactor = static_cast<float>(getDPI()) / 96.f;

            RB_EDITOR_INFO("[Application] GUI setup started; dpi_scale={}", scaleFactor);
            m_guiWrapper.setup(*m_window, false);
            std::vector<robot2D::GuiFontConfig> guiFontConfigs;

            robot2D::GuiFontConfig defaultFontConfig;
            defaultFontConfig.mode = robot2D::GuiFontConfig::Mode::File;
            defaultFontConfig.isDefault = true;
            defaultFontConfig.path = defaultFontPath;
            defaultFontConfig.size = fontSize * scaleFactor;

            robot2D::GuiFontConfig iconFontConfig1;
            iconFontConfig1.mode = robot2D::GuiFontConfig::Mode::File;
            iconFontConfig1.isDefault = false;
            iconFontConfig1.mergeFont = true;
            iconFontConfig1.path = fontPath1;
            iconFontConfig1.size = fontSize * scaleFactor * 0.75f;

            robot2D::GuiFontConfig iconFontConfig2;
            iconFontConfig2.mode = robot2D::GuiFontConfig::Mode::File;
            iconFontConfig2.isDefault = false;
            iconFontConfig2.mergeFont = true;
            iconFontConfig2.path = fontPath2;
            iconFontConfig2.size = fontSize * scaleFactor * 0.75f;


            guiFontConfigs.push_back(defaultFontConfig);
            guiFontConfigs.push_back(iconFontConfig1);
            guiFontConfigs.push_back(iconFontConfig2);

            m_guiWrapper.setupFonts(std::move(guiFontConfigs));

            auto& io = ImGui::GetIO();
            io.IniFilename = nullptr;

            auto& config = EditorConfig::getConfig();
            if(!hasFile(configPath)) {
                if(!config.createDefaultConfig(configPath))
                    RB_EDITOR_WARN("Can't create editor config: {0}", configPath);
            }
            else {
                if(!config.loadConfig(configPath))
                    RB_EDITOR_WARN("Can't load editor config; using defaults: {0}", configPath);
            }
            if(config.layoutCreated)
                ImGui::LoadIniSettingsFromMemory(config.layoutSettings.data(), config.layoutSettings.size());
            RB_EDITOR_INFO("[Application] GUI setup completed; config='{}'; layout_restored={}",
                configPath, config.layoutCreated);
        }


        //////////////////////////////////// Load C# Mono ////////////////////////////////////
        std::string scriptingEngineDLLPath = "res/script/robot2D_ScriptCore";
        RB_EDITOR_INFO("[Application] Initializing Mono; module='{}'", scriptingEngineDLLPath);
        m_scriptingEngine.Init(scriptingEngineDLLPath);
        RB_EDITOR_INFO("[Application] Mono initialization returned");
        m_scriptingEngine.SetWindow(m_window);
        //////////////////////////////////// Load C# Mono ////////////////////////////////////

        RB_EDITOR_INFO("[Application] Creating editor module");
        m_editorModule = EditorAssembly::createEditorModule(m_window,
                m_messageBus,
                m_messageDispatcher,
                m_guiWrapper);

        if(!m_editorModule)
            RB2D_EXCEPTION("Can't initialize editorModule");

        RB_EDITOR_INFO("[Application] Setting up editor module");
        m_editorModule -> setup(m_window, &m_scriptingEngine);
        RB_EDITOR_INFO("[Application] Editor module setup completed");
        RB_EDITOR_INFO("[Application] Setting up application logic; project loading is now allowed");
        m_logic.setup( m_editorModule.get(), &m_scriptingEngine);


        if(m_logic.getState() == AppState::ProjectInspector) {
            auto windowSize = m_window -> getSize();
            auto monitorSize = m_window -> getMonitorSize();
            auto centerPoint = getCenterPoint(windowSize, monitorSize);

            m_window -> setPosition(centerPoint);
            m_window -> setResizable(false);
            m_window -> setSize(m_appConfiguration.inspectorSize);

            applyStyle(EditorStyle::SlateBlue);
            const auto& cache = m_logic.getCache();
            m_projectInspector.setup(cache.getProjects(), cache.showInspectorOnStartup());
        }

        m_eventBinder.bind<robot2D::Event::Resized>([this](const robot2D::Event::Resized& evt) {
            RB_EDITOR_INFO("Editor's window new size = {0} and {1}", evt.width, evt.height);
            m_window -> resize({static_cast<int>(evt.width),
                                static_cast<int>(evt.height)});
            m_window -> setView({{0, 0}, {static_cast<float>(evt.width),
                                          static_cast<float>(evt.height)}});
        });
        RB_EDITOR_INFO("[Application] Setup completed; editor_active={}",
            m_logic.getState() == AppState::Editor);
    }

    void Application::handleEventFrame(const robot2D::FrameEvents& events) {
        if(m_logic.getState() == AppState::Editor) m_editorModule -> beginEventFrame();
        for(const auto& event : events.events) {
            handleEvents(event);
            if(!m_running) break;
        }
    }

    void Application::handleEvents(const robot2D::Event& event) {
        m_guiWrapper.handleEvents(event);
        m_eventBinder.dispatch(event);
        if(m_logic.getState() == AppState::Editor) m_editorModule -> handleEvents(event);
        if(event.getIf<robot2D::Event::Closed>()) {
            if(m_logic.getState() == AppState::Editor) {
                m_window -> cancelClose();
                m_editorModule -> closeCurrentProject([this]() { m_running = false; });
            }
            else m_running = false;
        }
    }

    void Application::handleMessages() {
        robot2D::Message message{};
        while(m_messageBus.pollMessages(message)) {
            if(m_logic.getState() == AppState::Editor)
                m_editorModule -> handleMessages(message);

            m_messageDispatcher.process(message);
        }
    }

    void Application::update(float dt) {
        TaskQueue::GetQueue() -> process();
        if(m_logic.getState() == AppState::Editor)
            m_editorModule -> update(dt);
    }

    void Application::guiUpdate(float dt) {
        if(m_editorLayoutActive && m_logic.getState() != AppState::Editor) {
            if(saveEditorLayout())
                m_editorLayoutActive = false;
        }
        m_guiWrapper.update(dt);
        if(ImGui::GetIO().WantSaveIniSettings && m_logic.getState() == AppState::Editor)
            saveEditorLayout();
    }

    bool Application::saveEditorLayout() {
        auto& config = EditorConfig::getConfig();
        if(!config.layoutCreated)
            return false;
        std::size_t size = 0;
        auto* settings = ImGui::SaveIniSettingsToMemory(&size);
        config.layoutSettings.assign(settings, size);
        if(!config.saveConfig(configPath)) {
            RB_EDITOR_WARN("Can't save editor layout: {0}", configPath);
            ImGui::GetIO().WantSaveIniSettings = true;
            return false;
        }
        ImGui::GetIO().WantSaveIniSettings = false;
        return true;
    }

    void Application::render() {
        m_window -> clear();
        if(m_logic.getState() == AppState::Editor) {
            m_editorModule -> render();
            m_editorLayoutActive = true;
        }
        else if(m_logic.getState() == AppState::ProjectInspector)
            m_projectInspector.render();
        PopupManager::getManager() -> onRender();
        m_guiWrapper.render();
        m_window -> display();
    }

    void Application::destroy() {
        RB_EDITOR_INFO("[Application] Destroy started");
        if(m_editorLayoutActive)
            saveEditorLayout();
        RB_EDITOR_INFO("[Application] Editor module destroy started; waiting for background work");
        m_editorModule -> destroy();
        m_editorModule.reset();
        LocalResourceManager::getManager() -> releaseGraphicsResources();
        RB_EDITOR_INFO("[Application] Editor module released; Mono shutdown started");
        m_scriptingEngine.Shutdown();
        RB_EDITOR_INFO("[Application] Mono shutdown completed");
    }

} // namespace editor
