//
// Created by support on 26.03.2021.
//

#pragma once

#include <robot2D/Core/IStateMachine.h>
#include <robot2D/Core/State.h>

#include "PanelManager.h"
#include "EditorScene.h"

namespace robot2D{
    class EditorState: public State{
    public:
        EditorState(IStateMachine& machine);
        ~EditorState()override = default;

        virtual void handleEvents(const Event& event)override;
        virtual void update(float dt)override;
        virtual void render()override;
    private:
        void setup();

        // logic for keymaps //

        void saveScene();
        void undo();
        void redo();

        // logic for keymaps //
    private:
        EditorScene m_editorScene;
        PanelManager m_panelManager;
    };
}
