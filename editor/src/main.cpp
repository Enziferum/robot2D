//
// Created by Necromant on 03.10.2021.
//

#include <robot2D/Util/Logger.hpp>
#include <editor/Editor.hpp>

#include <exception>

int main() {
    editor::Editor editor;

    try {
        editor.run();
    }
    catch(const std::exception& exception) {
        LOG_ERROR_E(exception.what())
    }
}