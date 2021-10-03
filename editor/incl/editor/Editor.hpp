//
// Created by Necromant on 03.10.2021.
//

#pragma once

#include <robot2D/Graphics/RenderWindow.hpp>
#include <robot2D/Core/MessageBus.hpp>
#include "Wrapper.hpp"
#include "PanelManager.hpp"

namespace editor {
    class Editor {
    public:
        Editor();
        Editor(const Editor&)=delete;
        Editor(const Editor&&)=delete;
        Editor& operator=(const Editor&)=delete;
        Editor& operator=(const Editor&&)=delete;
        ~Editor() = default;

        void run();
    private:
        void setup();
        void handleEvents();
        void handleMessages();
        void update(float dt);
        void render();
    private:
        robot2D::RenderWindow m_window;
        robot2D::MessageBus m_messageBus;
        ImGui::Wrapper m_guiWrapper;
        PanelManager m_panelManager;
    };
}