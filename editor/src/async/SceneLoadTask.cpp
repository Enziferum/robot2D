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
    namespace {
        void loadEntityAssets(const Scene::Ptr& scene, SceneEntity entity);
    }
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

    void loadSceneAssets(const Scene::Ptr& scene) {
        if(!scene) return;
        for(auto entity: scene -> getRootEntities())
            loadEntityAssets(scene, entity);
    }

    namespace {
        void loadEntityAssets(const Scene::Ptr& scene, SceneEntity entity) {
            if(entity.hasComponent<DrawableComponent>()) {
                auto& drawable = entity.getComponent<DrawableComponent>();
                auto& localTexturePath = drawable.getTexturePath();
                if(!localTexturePath.empty()) {
                    fs::path texturePath{localTexturePath};
                    auto id = texturePath.filename().string();
                    auto image = ResourceManager::getManager() -> addImage(id);
                    if(image) {
                        auto absolutePath = combinePath(scene -> getAssociatedProjectPath(),
                                texturePath.string());
                        if(!image -> loadFromFile(absolutePath)) {
                            RB_EDITOR_WARN("SceneLoadTask::loadAssets: can't load image by path {0}", absolutePath);
                        }
                    }
                }
            }
            if(entity.hasComponent<TextComponent>()) {
                auto& text = entity.getComponent<TextComponent>();
                auto& localFontPath = text.getFontPath();
                if(!localFontPath.empty()) {
                    fs::path fontPath{localFontPath};
                    auto id = fontPath.filename().string();
                    auto font = ResourceManager::getManager() -> addFont(id);
                    if(font) {
                        auto absolutePath = combinePath(scene -> getAssociatedProjectPath(), fontPath.string());
                        if(!font -> loadFromFile(absolutePath)) {
                            RB_EDITOR_WARN("SceneLoadTask::loadAssets: can't load font by path {0}", absolutePath);
                        }
                    }
                }
            }
            if(entity.hasComponent<AnimationComponent>()) {
                auto resourceManager = ResourceManager::getManager();
                if(!resourceManager)
                    return;

                auto& animationPaths = resourceManager
                    -> getAnimationsPathToLoad(entity.getComponent<IDComponent>().ID);
                AnimationParser animationParser;
                for(auto& localAnimationPath: animationPaths) {
                    fs::path animationPath{localAnimationPath};
                    auto id = animationPath.filename().string();
                    auto absolutePath = combinePath(scene -> getAssociatedProjectPath(),
                            animationPath.string());
                    Animation loaded;
                    if(!animationParser.loadFromFile(absolutePath, &loaded)) {
                        RB_EDITOR_WARN("SceneLoadTask::loadAssets: can't load animation by path {0}", absolutePath);
                        continue;
                    }
                    loaded.filePath = absolutePath;

                    auto localImagePath = loaded.texturePath;
                    fs::path imagePath{localImagePath};
                    auto imageId = imagePath.filename().string();
                    auto* image = resourceManager -> addImage(imageId);
                    if(!image && !resourceManager -> hasImage(imageId))
                        continue;
                    auto absoluteImagePath =
                        combinePath(scene -> getAssociatedProjectPath(), imagePath.string());

                    if(image && !image -> loadFromFile(absoluteImagePath)) {
                        RB_EDITOR_WARN("SceneLoadTask::loadAssets: can't load animation's image by path {0}",
                            absoluteImagePath);
                        continue;
                    }
                    const auto& loadedImage = resourceManager -> getImage(imageId);
                    if(!loadedImage.getSize().x || !loadedImage.getSize().y)
                        continue;
                    auto* animation = resourceManager -> addAnimation(entity.getUUID(), id);
                    if(animation)
                        *animation = std::move(loaded);
                }

            }

            if(entity.hasChildren()) {
                for(auto& child: entity.getChildren())
                    loadEntityAssets(scene, child);
            }
        }
    }
}
