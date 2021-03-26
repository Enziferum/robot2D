//
// Created by support on 26.03.2021.
//

#include <robot2D/Core/App.h>
#include <robot2D/Editor/EditorState.h>

using namespace robot2D;

int main() {

    App app;
    app.register_state<EditorState>(0, app);
    app.run();

    return 0;
}
