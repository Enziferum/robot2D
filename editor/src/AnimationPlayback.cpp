#include <editor/AnimationPlayback.hpp>
#include <editor/Components.hpp>
#include <editor/LocalResourceManager.hpp>

namespace editor {

    bool playAnimation(SceneEntity entity, const std::string& name) {
        if(!entity || name.empty() || !entity.hasComponent<AnimatorComponent>() ||
            !entity.hasComponent<AnimationComponent>() || !entity.hasComponent<DrawableComponent>() ||
            !entity.hasComponent<TransformComponent>())
            return false;
        auto& component = entity.getComponent<AnimationComponent>();
        auto* manager = LocalResourceManager::getManager();
        const auto* clips = manager -> findAnimations(entity.getUUID());
        if(clips && std::count_if(clips -> begin(), clips -> end(), [&name](const auto& clip) {
            return clip.name == name;
        }) > 1)
            return false;
        auto* clip = manager -> getAnimation(entity.getUUID(), name);
        if(!clip) {
            clip = component.getAnimation();
            if(!clip || clip -> name != name)
                return false;
        }
        if(clip -> frames.empty() || clip -> framesPerSecond <= 0)
            return false;
        for(const auto& frame: clip -> frames) {
            if(frame.frame.width <= 0 || frame.frame.height <= 0)
                return false;
        }
        auto& animator = entity.getComponent<AnimatorComponent>();
        if(animator.playing() && animator.animationName() == name && component.getAnimation() == clip)
            return true;
        animator.Stop({});
        component.setAnimation(clip);
        animator.Play(name);
        if(const auto* frame = clip -> getFrame(0))
            component.setTextureRect(*frame);
        return true;
    }

    bool stopAnimation(SceneEntity entity, const std::string& name) {
        if(!entity || !entity.hasComponent<AnimatorComponent>())
            return false;
        auto& animator = entity.getComponent<AnimatorComponent>();
        if(!name.empty() && animator.animationName() != name)
            return false;
        animator.Stop(name);
        return true;
    }

}
