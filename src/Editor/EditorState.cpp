//
// Created by support on 26.03.2021.
//

#include "robot2D/Editor/EditorState.h"
#include "robot2D/Editor/ImGui-Robot.h"

namespace robot2D{
    EditorState::EditorState(IStateMachine& machine):
    State(machine) {
        setup();
    }

    void EditorState::setup() {

    }

    void EditorState::handleEvents(const Event& event) {

    }

    void EditorState::update(float dt) {

    }

    void EditorState::render() {

    }

    void EditorState::saveScene() {

    }

    void EditorState::undo() {

    }

    void EditorState::redo() {

    }

}