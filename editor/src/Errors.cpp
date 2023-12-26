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

#include <editor/Errors.hpp>

namespace editor {

    std::string errorToString(const EditorCacheError& error) {
        switch(error) {
            case EditorCacheError::NoCacheFile:
                return "Don't Have Cache file";
            case EditorCacheError::EditorTagNone:
                return "Cache file don't have tag Editor";
            case EditorCacheError::ProjectsTagNone:
                return "Cache file don't have tag Projects";
            default:
                return "Undefined Error";
        }
    }

    std::string errorToString(const ProjectManagerError& error) {
        switch(error) {
            case ProjectManagerError::ProjectSerialize:
                return "Can't Serialize Project";
            case ProjectManagerError::ProjectDeserialize:
                return "Cant' Deserialize Project";
            case ProjectManagerError::CreateFolder:
                return "Can't create Project Folder";
            case ProjectManagerError::RemoveFolder:
                return "Can't remove Project Folder";
            default:
                return "Undefined Error";
        }
    }

    std::string errorToString(const SceneManagerError& error) {
        switch(error) {
            case SceneManagerError::MemoryAlloc:
                return "Can't allocate memory";
            case SceneManagerError::SceneSerialize:
                return "Can't Serialize Scene";
            case SceneManagerError::SceneDeserialize:
                return "Can't Deserialize Scene";
            default:
                return "Undefined Error";
        }
    }

    std::string errorToString(const ProjectSerializerError& error) {
        switch (error) {
            case ProjectSerializerError::ProjectNull:
                return "Can't ProjectNull";
            case ProjectSerializerError::FileNotOpened:
                return "Can't FileNotOpened";
            case ProjectSerializerError::NotValidProject:
                return "Can't NotValidProject";
            default:
                return "Undefined Error";
        }
    }

    std::string errorToString(const SceneSerializerError& error) {
        switch (error) {
            case SceneSerializerError::NotSceneTag:
                return "No Scene Tag";
            case SceneSerializerError::NoFileOpen:
                return "Can't file open";
            default:
                return "Undefined Error";
        }
    }

}

