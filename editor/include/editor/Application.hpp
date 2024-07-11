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

#pragma once

#include <robot2D/Application.hpp>
#include <robot2D/Core/MessageBus.hpp>
#include <robot2D/imgui/Gui.hpp>

#include "ApplicationLogic.hpp"
#include "ProjectInspector.hpp"
#include "ProjectManager.hpp"
#include "MessageDispather.hpp"
#include "Editor.hpp"
#include "EditorLogic.hpp"
#include "TaskQueue.hpp"
#include "EventBinder.hpp"
#include "EditorAssembly.hpp"
#include <editor/scripting/ScriptingEngine.hpp>

namespace editor {

    struct ApplicationConfiguration {
        const std::string logoPath = "res/textures/logo.png";
        const robot2D::vec2u inspectorSize = {600, 400 };
    };

    class Application: public robot2D::Application {
    public:
        Application();
        Application(const Application& other) = delete;
        Application& operator=(const Application& other) = delete;
        Application(Application&& other) = delete;
        Application& operator=(Application&& other) = delete;
        ~Application() override = default;

        void setup() override;
        void destroy() override;

        void handleEvents(const robot2D::Event& event) override;
        void handleMessages() override;
        void update(float dt) override;
        void guiUpdate(float dt)override;
        void render() override;
    private:
        ApplicationConfiguration m_appConfiguration;
        MessageDispatcher m_messageDispatcher;
        ApplicationLogic m_logic;
        ScriptEngine m_scriptingEngine;

        robot2D::MessageBus m_messageBus;
        robot2D::Gui m_guiWrapper;

        IEditorModule::Ptr m_editorModule;

        ProjectInspector m_projectInspector;
        EventBinder m_eventBinder;
    };
}

