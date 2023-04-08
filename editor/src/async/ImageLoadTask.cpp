#include <robot2D/Graphics/Image.hpp>
#include <robot2D/Util/Logger.hpp>

#include <editor/async/ImageLoadTask.hpp>

namespace editor {

    ImageLoadTask::ImageLoadTask(ITaskFunction::Ptr function, const std::string& path, robot2D::ecs::Entity entity):
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
        int a = 12;
    }

} // namespace editor