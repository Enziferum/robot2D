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

#include <robot2D/Graphics/Image.hpp>
#include <robot2D/Util/Logger.hpp>

#include <editor/async/ImageLoadTask.hpp>

namespace editor {

    ImageLoadTask::ImageLoadTask(ITaskFunction::Ptr function, const std::string& path, SceneEntity entity):
    ITask(function),
    m_imagePath{path},
    m_entity(std::move(entity))
    {
    }

    void ImageLoadTask::execute() {
        if(!m_image.loadFromFile(m_imagePath)) {
            RB_EDITOR_ERROR("Can't load Image async, path = {0}", m_imagePath);
            return;
        }
    }

} // namespace editor