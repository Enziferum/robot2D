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
#include <vector>
#include <robot2D/Graphics/Rect.hpp>
#include "editor/Task.hpp"
#include "editor/SpriteCutter.hpp"

namespace editor {

    class AnimationTextureSliceTask: public ITask {
    public:
        AnimationTextureSliceTask(ITaskFunction::Ptr function,
                                  const std::string& fileName, const std::string& filePath);
        AnimationTextureSliceTask(const AnimationTextureSliceTask& other) = delete;
        AnimationTextureSliceTask& operator=(const AnimationTextureSliceTask& other) = delete;
        AnimationTextureSliceTask(AnimationTextureSliceTask&& other) = delete;
        AnimationTextureSliceTask& operator=(AnimationTextureSliceTask&& other) = delete;
        ~AnimationTextureSliceTask() override = default;

        void execute() override;
        [[nodiscard]] const std::string& getFileName() const { return m_fileName; }
        [[nodiscard]] const std::string& getFilePath() const { return m_filePath; }
        [[nodiscard]] const std::vector<robot2D::IntRect>& getRects() const { return m_frameRects; }
    private:
        std::vector<robot2D::IntRect> m_frameRects;
        SpriteCutter m_sheetCutter;
        std::string m_fileName;
        std::string m_filePath;
    };

} // namespace editor