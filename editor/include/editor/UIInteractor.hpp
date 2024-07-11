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

#pragma once

#include <vector>
#include <list>
#include <string>
#include <set>
#include <functional>
#include <editor/SceneEntity.hpp>

#include <editor/panels/ITreeItem.hpp>
#include "Uuid.hpp"
#include "DeletedEntitesRestoreInformation.hpp"
#include "CommandStack.hpp"
#include "ExportOptions.hpp"
#include "ScriptInteractor.hpp"

namespace editor {

    class UIInteractor {
    public:
        using Ptr = UIInteractor*;

        virtual ~UIInteractor() = 0;

        virtual std::vector<SceneEntity>& getSelectedEntities()  = 0;
        virtual SceneEntity findEntity(const robot2D::vec2i& mousePos) = 0;

        virtual void restoreDeletedEntities(DeletedEntitiesRestoreInformation& restoreInformation,
                                            DeletedEntitiesRestoreUIInformation& restoreUiInformation) = 0;


        virtual std::string getAssociatedProjectPath() const = 0;
        virtual const std::list<SceneEntity>& getEntities() const = 0;
        virtual void addEmptyEntity() = 0;
        virtual SceneEntity addButton() = 0;
        virtual SceneEntity createEmptyEntity() = 0;
        virtual SceneEntity duplicateEmptyEntity(SceneEntity entity) = 0;
        virtual bool setBefore(SceneEntity sourceEntity, SceneEntity target) = 0;
        virtual void removeEntityChild(SceneEntity entity) = 0;
        virtual bool isRunning() const = 0;
        virtual SceneEntity getEntity(UUID uuid) = 0;

        virtual void uiSelectedEntities(std::set<ITreeItem::Ptr>& uiItems, bool isAll) = 0;

        virtual void exportProject(const ExportOptions& exportOptions) = 0;
        virtual const std::vector<class_id>& getCommandStack() const = 0;
        virtual IScriptInteractorFrom::Ptr getScriptInteractor() = 0;
    };

} // namespace editor