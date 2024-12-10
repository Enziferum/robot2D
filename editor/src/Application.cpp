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

#include <editor/Application.hpp>
#include <editor/scripting/ScriptingEngine.hpp>

#include <editor/EditorStyles.hpp>
#include <editor/Utils.hpp>
#include <editor/EventBinder.hpp>
#include <editor/PopupManager.hpp>
#include <editor/Exception.hpp>

#include <imgui/imgui.h>

#include <editor/FileApi.hpp>
#include <GLFW/glfw3.h>
#include <robot2D/imgui/GuiFontConfig.hpp>
#include "IconsFontsAwesome5.hpp"
#include <robot2D/Config.hpp>

namespace editor {


    namespace {
        const std::string configPath = "res/robot2D.ini";

        int getDPI()
        {
#ifdef ROBOT2D_LINUX
            return 144;
#endif
#ifdef ROBOT2D_WINDOWS
            const HDC hdc = GetDC(NULL);
            return GetDeviceCaps(hdc, LOGPIXELSX);
#endif
        }
    }

    Application::Application():
            robot2D::Application(),
            m_appConfiguration{},
            m_messageDispatcher{},
            m_logic{m_messageDispatcher},
            m_guiWrapper{},
            m_projectInspector{m_messageBus}
            {}


    void Application::setup() {
        {
            robot2D::Image iconImage;
            iconImage.loadFromFile(m_appConfiguration.logoPath);
            m_window -> setIcon(std::move(iconImage));
        }

        if(!hasFile(configPath)) {
            ///
        }


        {
            /// TODO(a.raag): read from config .ini
            std::string defaultFontPath = "res/fonts/notosans-regular.ttf";
            std::string fontPath1 = std::string{"res/fonts/fa-regular-400.ttf"};
            std::string fontPath2 = std::string{"res/fonts/fa-solid-900.ttf"};
            float fontSize = 18.f;

            {
                float xscale, yscale;

                auto monitor = glfwGetPrimaryMonitor();
                glfwGetMonitorContentScale(monitor, &xscale, &yscale);
                int wMM, hMM;
                glfwGetMonitorPhysicalSize(monitor, &wMM, &hMM);
                auto videoMode = glfwGetVideoMode(monitor);

                double xres = (double)videoMode -> width * 25.4 / (double)wMM;
                double yres = (double)videoMode -> height * 25.4 / (double)hMM;


            }

            float scaleFactor = static_cast<float>(getDPI()) / 96.f;
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
        }


        //////////////////////////////////// Load C# Mono ////////////////////////////////////
        std::string scriptingEngineDLLPath = "res/script/robot2D_ScriptCore";
        m_scriptingEngine.Init(scriptingEngineDLLPath);
        m_scriptingEngine.SetWindow(m_window);
        //////////////////////////////////// Load C# Mono ////////////////////////////////////

        m_editorModule = EditorAssembly::createEditorModule(m_window,
                                                            m_messageBus,
                                                            m_messageDispatcher,
                                                            m_guiWrapper);

        if(!m_editorModule)
            RB2D_EXCEPTION("Can't initialize editorModule");

        m_logic.setup( m_editorModule.get(), &m_scriptingEngine);
        m_editorModule -> setup(m_window, &m_scriptingEngine);


        if(m_logic.getState() == AppState::ProjectInspector) {
            auto windowSize = m_window -> getSize();
            auto monitorSize = m_window -> getMonitorSize();
            auto centerPoint = getCenterPoint(windowSize, monitorSize);

            m_window -> setPosition(centerPoint);
            m_window -> setResizable(false);
            m_window -> setSize(m_appConfiguration.inspectorSize);

            applyStyle(EditorStyle::UE4);
            m_projectInspector.setup(m_window, m_logic.getCache().getProjects());
        }

        m_eventBinder.bindEvent(robot2D::Event::Resized, [this](const robot2D::Event& evt) {
            RB_EDITOR_INFO("Editor's window new size = {0} and {1}", evt.size.widht, evt.size.heigth);
            m_window -> resize({static_cast<int>(evt.size.widht),
                                static_cast<int>(evt.size.heigth)});
            m_window -> setView({{0, 0}, {static_cast<float>(evt.size.widht),
                                          static_cast<float>(evt.size.heigth)}});
        });
    }

    void Application::handleEvents(const robot2D::Event& event) {
        m_eventBinder.handleEvents(event);

        m_guiWrapper.handleEvents(event);
        if(m_logic.getState() == AppState::Editor)
            m_editorModule -> handleEvents(event);
    }

    void Application::handleMessages() {
        robot2D::Message message{};
        while (m_messageBus.pollMessages(message)) {
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
        m_guiWrapper.update(dt);
    }

    void Application::render() {
        m_window -> clear();
        if(m_logic.getState() == AppState::Editor)
            m_editorModule -> render();
        else if(m_logic.getState() == AppState::ProjectInspector)
            m_projectInspector.render();
        PopupManager::getManager() -> onRender();
        m_guiWrapper.render();
        m_window -> display();
    }

    void Application::destroy() {
        m_scriptingEngine.Shutdown();
        m_editorModule -> destroy();
    }

} // namespace editor