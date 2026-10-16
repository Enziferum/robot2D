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

#include <string>
#include <list>
#include <algorithm>
#include <vector>
#include <iterator>
#include <cstdint>

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
                frame.frameIndex = index++;
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

        // Uniform grid, row-major, with an explicit bound against accidental huge allocations.
        bool sliceGrid(robot2D::vec2i imageSize, robot2D::vec2i cellSize) {
            if(imageSize.x <= 0 || imageSize.y <= 0 || cellSize.x <= 0 || cellSize.y <= 0 ||
                imageSize.x % cellSize.x || imageSize.y % cellSize.y)
                return false;
            const auto columns = imageSize.x / cellSize.x;
            const auto rows = imageSize.y / cellSize.y;
            if(static_cast<std::uint64_t>(columns) * rows > 65536)
                return false;
            frames.clear();
            for(int row = 0; row < rows; ++row) {
                for(int column = 0; column < columns; ++column)
                    addFrame({column* cellSize.x, row* cellSize.y, cellSize.x, cellSize.y});
            }
            return true;
        }

        bool moveFrame(int from, int to) {
            const auto count = static_cast<int>(frames.size());
            if(from < 0 || to < 0 || from >= count || to >= count)
                return false;
            const auto source = std::next(frames.begin(), from);
            const auto destination = std::next(frames.begin(), to + (from < to ? 1 : 0));
            frames.splice(destination, frames, source);
            setFrameIndices();
            return true;
        }

        bool removeFrame(int index) {
            if(index < 0 || static_cast<std::size_t>(index) >= frames.size())
                return false;
            frames.erase(std::next(frames.begin(), index));
            setFrameIndices();
            return true;
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
            if(animationFrames.empty())
                return;
            const auto selected = [&animationFrames](const auto& item) {
                return std::any_of(animationFrames.begin(), animationFrames.end(), [&item](const auto& candidate) {
                    return candidate.frameIndex == item.frameIndex;
                });
            };
            const auto first = std::find_if(frames.begin(), frames.end(), selected);
            if(first == frames.end())
                return;
            // Insert before erasing: the insertion iterator must not be invalidated.
            const auto inserted = frames.insert(first, replace);
            for(auto it = std::next(inserted); it != frames.end();) {
                if(selected(*it))
                    it = frames.erase(it);
                else
                    ++it;
            }
            setFrameIndices();
        }
    };


} // namespace editor