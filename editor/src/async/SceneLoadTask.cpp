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
#include <filesystem>
#include <chrono>

#include <editor/async/SceneLoadTask.hpp>
#include <editor/serializers/SceneSerializer.hpp>
#include <editor/ResouceManager.hpp>
#include <editor/FileApi.hpp>
#include <editor/AnimationParser.hpp>

namespace editor {
    namespace fs = std::filesystem;

    SceneLoadTask::SceneLoadTask(ITaskFunction::Ptr function,
        Scene::Ptr scene,
        IScriptInteractorFrom::WeakPtr scriptInteractorFrom,
        SceneLoadChainCallback&& callback) :
        ITask{ function },
        m_scene{ scene },
        m_scriptInteractorFrom{scriptInteractorFrom},
        m_chainCallback{ std::move(callback) } {
    }


    void SceneLoadTask::execute() {
        const auto started = std::chrono::steady_clock::now();
        RB_EDITOR_INFO("[SceneLoad] Started; path='{}'", m_scene -> getPath());
        SceneSerializer serializer{m_scene };

        auto interactor = m_scriptInteractorFrom.lock();
        if(!interactor) {
            fail("Script interactor is unavailable");
            RB_EDITOR_ERROR("[SceneLoad] Failed; path='{}'; reason=script interactor unavailable",
                m_scene -> getPath());
            return;
        }

        if(!serializer.deserialize(m_scene -> getPath(), interactor)) {
            fail(serializer.getErrorMessage());
            RB_EDITOR_ERROR("[SceneLoad] Deserialize failed; path='{}'; reason='{}'; detail='{}'",
                m_scene -> getPath(), errorToString(serializer.getError()),
                serializer.getErrorMessage());
            return;
        }

        RB_EDITOR_INFO("[SceneLoad] Deserialize completed; path='{}'; entities={}; loading assets",
            m_scene -> getPath(), m_scene -> getEntities().size());
        loadAssets();
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - started).count();
        RB_EDITOR_INFO("[SceneLoad] CPU loading completed; path='{}'; elapsed_ms={}",
            m_scene -> getPath(), elapsed);
    }

    void SceneLoadTask::loadAssets() {
        loadSceneAssets(m_scene);
    }

}
