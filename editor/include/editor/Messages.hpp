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
#include <string>
#include <robot2D/Core/Message.hpp>
#include <robot2D/Ecs/Entity.hpp>
#include "Uuid.hpp"
#include "ProjectDescription.hpp"

namespace editor {

    enum MessageID {
        CreateProject = robot2D::Message::Count,
        DeleteProject,
        LoadProject,
        SaveProject,
        OpenProject,
        OpenScene,
        CreateScene,
        SaveScene,
        EntitySelected,
        EntityDuplicate,
        EntityRemove,
        PanelEntitySelected,
        PanelEntityNeedSelect,
        ShowInspector,
        ToolbarPressed,
        InstrumentPressed,
        Undo,
        Redo,
        GenerateProject,
        ExportProject,
        PrefabLoad
    };

    struct ProjectMessage {
        ProjectDescription description;
    };

    struct ShowInspectorMessage {
        bool showAlways;
    };

    struct MenuProjectMessage {
        std::string path;
    };

    struct EntitySelection {
        uint32_t entityID;
    };

    struct EntityDuplication {
        UUID entityID;
    };

    struct EntityRemovement {
        UUID entityID;
    };

    struct ToolbarMessage {
        int pressedType;
    };

    struct OpenSceneMessage {
        std::string path;
    };

    struct CreateSceneMessage {
        std::string path;
    };

    struct UndoRedoMessage{};

    struct GenerateProjectMessage{};

    struct ExportProjectMessage{
        std::string startSceneID;
    };

    struct PanelEntitySelectedMessage {
        robot2D::ecs::Entity entity;
    };

    struct PrefabLoadMessage {
        std::string path;
    };

    struct InstrumentMessage {
        enum class Type {
            Move = 0,
            Scale,
            Rotate
        } type;
    };
}