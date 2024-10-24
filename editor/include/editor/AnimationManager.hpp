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

#include <unordered_map>
#include <robot2D/Ecs/Entity.hpp>

#include "Animation.hpp"
#include "AnimationParser.hpp"

namespace editor {

    class AnimationManager {
    public:
        ~AnimationManager() = default;
        AnimationManager(const AnimationManager& other) = delete;
        AnimationManager& operator=(const AnimationManager& other) = delete;
        AnimationManager(AnimationManager&& other) = delete;
        AnimationManager& operator=(AnimationManager&& other) = delete;

        static AnimationManager* getManager() {
            static AnimationManager animationManager;
            return &animationManager;
        }

        std::unordered_map<std::string, Animation>& getAnimations() {
            return m_animations;
        }

        std::vector<Animation> getAnimations(robot2D::ecs::Entity entity) {
            return {};
        }


        void addAnimation(robot2D::ecs::Entity associatedEntity,
                          const std::string& path, const std::string& name);
        Animation* getAnimation(const std::string& name) const;


        bool saveToFile(const std::string& path);
        bool loadFromFile(const std::string& path);

        bool saveAnimation(const Animation* animation);
    private:
        AnimationManager() = default;
    private:
        std::unordered_map<std::string, Animation> m_animations;
        AnimationParser m_animationParser;
    };


} // namespace editor