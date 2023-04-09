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

#include "Configuration.hpp"
#include "EditorCache.hpp"
#include "MessageDispather.hpp"
#include "Messages.hpp"
#include "ProjectManager.hpp"
#include "EditorAssembly.hpp"

namespace editor {
    enum class AppState {
        ProjectInspector,
        Editor
    };

    class ApplicationLogic {
    public:
        ApplicationLogic(MessageDispatcher& messageDispatcher);
        ~ApplicationLogic() = default;

        void setup(IEditorOpener* editorModule);
        const AppState& getState() const { return m_state; }
        const EditorCache& getCache() const { return m_editorCache; }
    private:
        void createProject(const ProjectMessage& projectDescription);
        void deleteProject(const ProjectMessage& projectDescription);
        void loadProject(const ProjectMessage& projectDescription);
    private:
        MessageDispatcher& m_messageDispatcher;
        Configuration m_configuration;
        EditorCache m_editorCache;
        ProjectManager m_projectManager;
        AppState m_state;
        IEditorOpener* m_editorOpener{nullptr};
    };
}