/*********************************************************************
   (c) Alex Raag 2024
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
#include <editor/async/ExportTask.hpp>

namespace editor {

    ExportTask::ExportTask(ITaskFunction::Ptr function, const ExportOptions& exportOptions) :
        ITask(function), m_exportOptions{exportOptions} {

    }

    void ExportTask::execute() {
        if(m_exportOptions.outputFolder.empty()) {
            fail("Build output directory is not selected");
            return;
        }
        if(m_exportOptions.startScene.empty()) {
            fail("Build start scene is not selected");
            return;
        }
        fail("Build is unavailable: standalone player and the integrated script/package pipeline are not ready "
            "(PLAYER-01/SCRIPT-01/BUILD-01). No game output was created.");
    }
}