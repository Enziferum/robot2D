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

#include <editor/AnimationManager.hpp>

namespace editor {

    void AnimationManager::addAnimation(robot2D::ecs::Entity associatedEntity,
                                        const std::string& path, const std::string& name) {

        Animation animation;
        animation.associatedEntity = associatedEntity;
        animation.filePath = path;
        animation.name = name;
        m_animations[name] = animation;
    }

    Animation* AnimationManager::getAnimation(const std::string& name) const {
        return nullptr;
    }


    bool AnimationManager::saveToFile(const std::string& path) {
        return true;
    }

    bool AnimationManager::loadFromFile(const std::string& path) {
        return true;
    }

    bool AnimationManager::saveAnimation(const Animation* animation) {
        return m_animationParser.saveToFile(animation -> filePath, animation);
    }

} // namespace editor