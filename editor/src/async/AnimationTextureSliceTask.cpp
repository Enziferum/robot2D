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
#include <unordered_set>
#include <editor/async/AnimationTextureSliceTask.hpp>
#include <editor/ResouceManager.hpp>
#include "robot2D/Util/Logger.hpp"

namespace util {
    struct HashFunction {
        size_t operator()(const robot2D::IntRect& rect) const
        {
            // Compute individual hash values for first, second and third
            // http://stackoverflow.com/a/1646913/126995
            size_t res = 17;
            res = res * 31 + std::hash<int>()( rect.lx );
            res = res * 31 + std::hash<int>()( rect.ly );
            res = res * 31 + std::hash<int>()( rect.width );
            res = res * 31 + std::hash<int>()( rect.height );
            return res;
        }
    };
}

namespace editor {




    AnimationTextureSliceTask::AnimationTextureSliceTask(
        editor::ITaskFunction::Ptr function,
        const std::string& fileName, 
        const std::string& filePath):
        ITask(function), m_fileName{fileName}, m_filePath{filePath} 
    {

    }

    void AnimationTextureSliceTask::execute() {
        auto* manager = ResourceManager::getManager();
        auto* image = manager -> addImage(m_fileName);
        if(!image -> loadFromFile(m_filePath)) {
            RB_EDITOR_ERROR("AnimationTextureSliceTask: Can't loadImage by path {0}", m_filePath);
            return;
        }


        auto imageSize = image -> getSize();
        auto frames = m_sheetCutter.cutFrames({0, 0, imageSize.x, imageSize.y},
                                               *image, {});

        std::unordered_set<robot2D::IntRect, util::HashFunction> uniqueFrames;
        for (const auto& frame: frames)
            uniqueFrames.insert(frame);

        std::vector<robot2D::IntRect> sortFrames;
        sortFrames.assign(uniqueFrames.begin(), uniqueFrames.end());
        std::sort(sortFrames.begin(), sortFrames.end(), [](const robot2D::IntRect &l,
                                                           const robot2D::IntRect &r) {
            return l.lx < r.lx;
        });

        for(auto& frame: sortFrames)
            m_frameRects.emplace_back(frame);
    }

} // namespace editor