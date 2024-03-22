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

#include "Buffer.hpp"
#include "Uuid.hpp"
#include "ProjectDescription.hpp"

namespace editor {

    enum MessageID {
        CreateProject = robot2D::Message::Count,
        AddProject,
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
        EntityRestore,
        PanelEntitySelected,
        PanelEntityNeedSelect,
        ShowInspector,
        ToolbarPressed,
        InstrumentPressed,
        Undo,
        Redo,
        GenerateProject,
        ExportProject,
        PrefabLoad,
        PrefabAssetPressed,
        TileElementSelected,
        PrefabAssetModificated,
        AnimationPlay
    };

    struct ProjectMessage {
        ProjectDescription description;

        void unpack(void* rawBuffer) {
            Buffer buffer{ rawBuffer };
            StringBuffer str_buff1;
            StringBuffer str_buff2;
            buffer.unpackFromBuffer(str_buff1);
            buffer.unpackFromBuffer(str_buff2);

            description.name = str_buff1.as_str();
            description.path = str_buff2.as_str();
        }
    };

    struct ShowInspectorMessage {
        bool showAlways;

        void unpack(void* rawBuffer) {}
    };

    struct MenuProjectMessage {
        std::string path;

        void unpack(void* rawBuffer) {
            Buffer buffer{ rawBuffer };
            StringBuffer str_buff1;
            buffer.unpackFromBuffer(str_buff1);
            path = str_buff1.as_str();
        }
    };

    struct EntitySelection {
        uint32_t entityID;

        void unpack(void* rawBuffer) {}
    };

    struct EntityDuplication {
        UUID entityID;

        void unpack(void* rawBuffer) {}
    };

    struct EntityRemovement {
        UUID entityID;

        void unpack(void* rawBuffer) {}
    };

    struct EntityRestorement {
        UUID entityID;

        void unpack(void* rawBuffer) {}
    };

    struct ToolbarMessage {
        int pressedType;

        void unpack(void* rawBuffer) {}
    };

    struct OpenSceneMessage {
        std::string path;

        void unpack(void* rawBuffer) {
            Buffer buffer{ rawBuffer };
            StringBuffer str_buff1;
            buffer.unpackFromBuffer(str_buff1);
            path = str_buff1.as_str();
        }
    };

    struct CreateSceneMessage {
        std::string path;

        void unpack(void* rawBuffer) {
            Buffer buffer{ rawBuffer };
            StringBuffer str_buff1;
            buffer.unpackFromBuffer(str_buff1);
            path = str_buff1.as_str();
        }
    };

    struct UndoRedoMessage{};

    struct GenerateProjectMessage{
        std::string genPath;

        void unpack(void* rawBuffer) {
            Buffer buffer{ rawBuffer };
            StringBuffer str_buff1;
            buffer.unpackFromBuffer(str_buff1);
            genPath = str_buff1.as_str();
        }
    };

    struct ExportProjectMessage{
        std::string startSceneID;

        void unpack(void* rawBuffer) {
            Buffer buffer{ rawBuffer };
            StringBuffer str_buff1;
            buffer.unpackFromBuffer(str_buff1);
            startSceneID = str_buff1.as_str();
        }
    };

    struct PanelEntitySelectedMessage {
        robot2D::ecs::Entity entity;

        void unpack(void* rawBuffer) {}
    };

    struct PrefabLoadMessage {
        std::string path;

        void unpack(void* rawBuffer) {
            Buffer buffer{ rawBuffer };
            StringBuffer str_buff1;
            buffer.unpackFromBuffer(str_buff1);
            path = str_buff1.as_str();
        }
    };

    struct PrefabAssetPressedMessage {
        std::string localPath;

        void unpack(void* rawBuffer) {
            Buffer buffer{ rawBuffer };
            StringBuffer str_buff1;
            buffer.unpackFromBuffer(str_buff1);
            localPath = str_buff1.as_str();
        }
    };

    struct PrefabAssetModificatedMessage {
        UUID prefabUUID;
        robot2D::ecs::Entity prefabEntity;

        void unpack(void* rawBuffer) {}
    };


    struct TileElementSelectedMessage {
        robot2D::vec2i tileIndex;

        void unpack(void* rawBuffer) {}
    };

    struct AnimationPlayMessage {
        robot2D::ecs::Entity entity;
        enum class Mode: int {
            Play = 0,
            Stop = 1
        } mode;

        void unpack(void* rawBuffer) {}
    };

    struct InstrumentMessage {
        enum class Type {
            Move = 0,
            Scale,
            Rotate
        } type;

        void unpack(void* rawBuffer) {}
    };
}