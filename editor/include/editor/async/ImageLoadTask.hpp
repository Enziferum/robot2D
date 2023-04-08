#pragma once

#include <string>
#include <editor/Task.hpp>
#include <robot2D/Graphics/Image.hpp>
#include <robot2D/Ecs/Entity.hpp>

namespace editor {
    class ImageLoadTask: public ITask {
    public:
        ImageLoadTask(ITaskFunction::Ptr function, const std::string& path,
                      robot2D::ecs::Entity entity);
        ~ImageLoadTask() override = default;

        void execute() override;
        const robot2D::Image& getImage() const { return m_image; }
        robot2D::ecs::Entity getEntity() const { return std::move(m_entity); }
    private:
        std::string m_imagePath;
        robot2D::Image m_image;
        robot2D::ecs::Entity m_entity;
    };
}