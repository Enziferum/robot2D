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

#include <editor/async/SceneLoadTask.hpp>
#include <editor/serializers/SceneSerializer.hpp>
#include <editor/ResouceManager.hpp>
#include <editor/FileApi.hpp>
#include <editor/AnimationParser.hpp>

namespace editor {
    namespace fs = std::filesystem;

    SceneLoadTask::SceneLoadTask(ITaskFunction::Ptr function,
                                 Scene::Ptr scene,
                                 SceneLoadChainCallback&& callback):
    ITask{ function },
    m_scene{ scene },
    m_chainCallback{ std::move(callback) }
    {}


    void SceneLoadTask::execute()  {

        SceneSerializer serializer{m_scene };
        if(!serializer.deserialize(m_scene -> getPath())) {
            RB_EDITOR_ERROR("SceneLoadTask: Can't Deserialze Scene");
            return;
        }

        loadAssets();
    }

    void SceneLoadTask::loadAssets() {
        for(auto& entity: m_scene -> getEntities())
            loadAssets(entity);
    }

    void SceneLoadTask::loadAssets(SceneEntity& entity) {
        if(entity.hasComponent<DrawableComponent>()) {
            auto& drawable = entity.getComponent<DrawableComponent>();
            auto& localTexturePath = drawable.getTexturePath();
            if(!localTexturePath.empty()) {
                fs::path texturePath{localTexturePath};
                auto id = texturePath.filename().string();
                auto image = ResourceManager::getManager() -> addImage(id);
                if(image) {
                    auto absolutePath = combinePath(m_scene -> getAssociatedProjectPath(),
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
                    auto absolutePath = combinePath(m_scene -> getAssociatedProjectPath(), fontPath.string());
                    if(!font -> loadFromFile(absolutePath)) {
                        RB_EDITOR_WARN("SceneLoadTask::loadAssets: can't load image by path {0}", absolutePath);
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
                auto animation = resourceManager -> addAnimation(entity.getComponent<IDComponent>().ID, id);
                if(!animation)
                    continue;
                auto absolutePath = combinePath(m_scene -> getAssociatedProjectPath(),
                                                animationPath.string());
                if(!animationParser.loadFromFile(absolutePath, animation))
                    RB_EDITOR_WARN("SceneLoadTask::loadAssets: can't load animation by path {0}", absolutePath);
                animation -> filePath = absolutePath;

                auto localImagePath = animation -> texturePath;
                fs::path imagePath{localImagePath};
                auto imageId = imagePath.filename().string();
                auto* image = resourceManager -> addImage(imageId);
                if(!image)
                    continue;
                auto absoluteImagePath =
                        combinePath(m_scene -> getAssociatedProjectPath(), imagePath.string());

                if(!image -> loadFromFile(absoluteImagePath)) {
                    RB_EDITOR_WARN("SceneLoadTask::loadAssets: can't load animation's image by path {0}", absoluteImagePath);
                }
            }

        }

        if(entity.hasChildren()) {
            for(auto& child: entity.getChildren())
                loadAssets(child);
        }
    }
}