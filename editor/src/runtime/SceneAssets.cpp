#include <editor/runtime/SceneAssets.hpp>
#include <editor/ResouceManager.hpp>
#include <editor/LocalResourceManager.hpp>
#include <editor/AnimationParser.hpp>
#include <editor/FileApi.hpp>
#include <robot2D/Util/Logger.hpp>
#include <stdexcept>
namespace editor {
    namespace fs = std::filesystem;
    namespace { void loadEntityAssets(const Scene::Ptr& scene, SceneEntity entity); }
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
                    const auto key = resourceFileKey(combinePath(scene -> getAssociatedProjectPath(),
                            localTexturePath));
                    if(!ResourceManager::getManager() -> loadImage(key))
                        RB_EDITOR_WARN("SceneLoadTask::loadAssets: can't load image by path {0}", key);
                }
            }
            if(entity.hasComponent<TextComponent>()) {
                auto& text = entity.getComponent<TextComponent>();
                auto& localFontPath = text.getFontPath();
                if(!localFontPath.empty()) {
                    const auto key = resourceFileKey(combinePath(scene -> getAssociatedProjectPath(), localFontPath));
                    if(!ResourceManager::getManager() -> loadFont(key))
                        RB_EDITOR_WARN("SceneLoadTask::loadAssets: can't load font by path {0}", key);
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

                    const auto imageId = resourceFileKey(combinePath(scene -> getAssociatedProjectPath(),
                            loaded.texturePath));
                    if(!resourceManager -> loadImage(imageId)) {
                        RB_EDITOR_WARN("SceneLoadTask::loadAssets: can't load animation's image by path {0}", imageId);
                        continue;
                    }
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

namespace editor {
    void activateEntityAssets(const Scene::Ptr& scene, SceneEntity entity, bool strict) {
        auto* manager = ResourceManager::getManager();
        auto* localManager = LocalResourceManager::getManager();
        if(entity.hasComponent<DrawableComponent>()) {
            auto& drawable = entity.getComponent<DrawableComponent>();
            auto id = drawable.getTexturePath().empty() ? std::string{} : resourceFileKey(
                combinePath(scene -> getAssociatedProjectPath(), drawable.getTexturePath()));
            if(localManager -> hasTexture(id)) {
                drawable.setTexture(localManager -> getTexture(id));
            }
            else {
                if(manager -> hasImage(id)) {
                    auto& image = manager -> getImage(id);
                    auto* texture = image.getSize().x && image.getSize().y
                            ? localManager -> addTexture(id) : nullptr;
                    if(texture) {
                        texture -> create(image);
                        drawable.setTexture(*texture);
                    }
                }
            }

        }

        if(entity.hasComponent<TextComponent>()) {
            auto& text = entity.getComponent<TextComponent>();
            auto id = text.getFontPath().empty() ? std::string{} : resourceFileKey(
                combinePath(scene -> getAssociatedProjectPath(), text.getFontPath()));
            if(localManager -> hasFont(id)) {
                text.setFont(localManager -> getFont(id));
            }
            else if(manager -> hasFont(id)) {
                auto& font = manager -> getFont(id);
                auto* localFont = !font.getImages().empty()
                        ? localManager -> addFont(id) : nullptr;
                if(localFont) {
                    localFont -> clone(font);
                    text.setFont(*localFont);
                }
            }
        }

        if(entity.hasComponent<AnimationComponent>()) {
            const auto animations = manager -> getAnimationsCopy(entity.getUUID());
            for(const auto& copyAnimation: animations) {
                if(copyAnimation.frames.empty())
                    continue;
                auto* animation = localManager -> addAnimation(entity.getComponent<IDComponent>().ID);
                *animation = copyAnimation;

                auto imageId = resourceFileKey(combinePath(scene -> getAssociatedProjectPath(),
                        animation -> texturePath));

                if(localManager -> hasTexture(imageId))
                    animation -> texture = &localManager -> getTexture(imageId);
                else {
                    if(!manager -> hasImage(imageId)) {
                        if(strict) throw std::runtime_error("Missing animation texture: " + animation -> texturePath);
                        continue;
                    }
                    auto& image = manager -> getImage(imageId);
                    auto* localTexture = localManager -> addTexture(imageId);
                    if(localTexture) {
                        localTexture -> create(image);
                        animation -> texture = localTexture;
                    }
                }
            }
            auto& animationComponent = entity.getComponent<AnimationComponent>();
            const auto* localAnimations = localManager -> findAnimations(entity.getUUID());
            if(localAnimations && !localAnimations -> empty())
                animationComponent.setAnimation(&localManager -> getAnimations(entity.getUUID()).front());
        }

        if(strict) {
            if(entity.hasComponent<DrawableComponent>()) {
                const auto& drawable = entity.getComponent<DrawableComponent>();
                if(!drawable.getTexturePath().empty() && !drawable.hasTexture())
                    throw std::runtime_error("Missing texture: " + drawable.getTexturePath());
            }
            if(entity.hasComponent<TextComponent>() && !entity.getComponent<TextComponent>().getFont())
                throw std::runtime_error("Missing font: " + entity.getComponent<TextComponent>().getFontPath());
            if(entity.hasComponent<AnimationComponent>()) {
                const auto& paths = manager -> getAnimationsPathToLoad(entity.getUUID());
                const auto* loaded = localManager -> findAnimations(entity.getUUID());
                if(!paths.empty() && (!loaded || loaded -> size() != paths.size()))
                    throw std::runtime_error("Missing/invalid animation for entity: " + entity.getName());
            }
        }
    }
}
