/*********************************************************************
(c) Alex Raag 2021
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
            case ProjectManagerError::ProjectDerialize:
                return "Cant' Deserialize Project";
            case ProjectManagerError::CreateFolder:
                return "Can't create Project Folder";
            case ProjectManagerError::RemoveFolder:
                return "Can't remove Project Folder";
            default:
                return "Undefined Error";
        }
    }
}

