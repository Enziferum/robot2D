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

#pragma once

#include <string>
#include <list>
#include <algorithm>

#include <robot2D/Graphics/Texture.hpp>
#include <robot2D/Graphics/Rect.hpp>
#include <robot2D/Ecs/Entity.hpp>

namespace editor {

    struct AnimationFrame {
        int frameIndex{0};
        float duration{0.f};
        robot2D::IntRect frame;
        robot2D::IntRect flipFrame;

        void addRect(const robot2D::IntRect& rect) {
            robot2D::IntRect _flipFrame(rect.lx + rect.width, rect.ly, -rect.width,
                                       rect.height);
            frame = rect;
            flipFrame = _flipFrame;
        }
    };

    struct Animation {
        int m_currentFrame{0};
        int framesPerSecond{0};
        float totalDuration{0.f};
        std::list<AnimationFrame> frames;
        bool isLooped{false};
        bool isFlipped{false};

        robot2D::Texture* texture{nullptr};
        std::string texturePath;
        std::string name;
        std::string filePath;

        robot2D::ecs::Entity associatedEntity;

        void setFrameIndices() {
            int index = 0;
            for(auto& frame: frames)
                frame.frameIndex = ++index;
        }

        void addFrame(const robot2D::IntRect& frame) {
            robot2D::IntRect flipFrame(frame.lx + frame.width, frame.ly, -frame.width,
                             frame.height);
            AnimationFrame animationFrame;
            animationFrame.frame = frame;
            animationFrame.flipFrame = flipFrame;
            animationFrame.frameIndex = static_cast<int>(frames.size());
            frames.emplace_back(animationFrame);
        }

        robot2D::IntRect* getFrame(int frameIndex) {
            auto found = std::find_if(frames.begin(), frames.end(),
                                      [&frameIndex](const auto& frame) {
                return frame.frameIndex == frameIndex;
            });
            if(found == frames.end())
                return nullptr;
            return isFlipped ? &(*found).flipFrame : &(*found).frame;
        }

        /// TODO(a.raag): replaceFrames fix bug
        void replaceFrames(std::vector<AnimationFrame>& animationFrames, AnimationFrame& replace) {
            using Iterator = std::list<AnimationFrame>::iterator;
            Iterator first;
            Iterator last = frames.end();
            Iterator newLast = frames.end();
            first = std::find_if(frames.begin(), frames.end(),
                                 [&animationFrames](const auto& item) {
                 return animationFrames[0].frameIndex == item.frameIndex;
            });

            replace.frameIndex = first -> frameIndex;

            for(auto& frame: animationFrames) {
                newLast = frames.erase(std::remove_if(frames.begin(),
                                            frames.end(), [&frame](const auto& item) {
                    return frame.frameIndex == item.frameIndex;
                }), frames.end());
            }
            if(last == newLast)
                frames.emplace_back(replace);
            else
                frames.insert(first, replace);
        }
    };


} // namespace editor