#pragma once

#include <string>
#include <editor/Task.hpp>
#include <robot2D/Graphics/Image.hpp>
#include <robot2D/Ecs/Entity.hpp>

namespace editor {
    class FontLoadTask: public ITask {
    public:
        FontLoadTask(ITaskFunction::Ptr function,
                        const std::string& path,
                      robot2D::ecs::Entity entity);
        ~FontLoadTask() override = default;

        void execute() override;
        const robot2D::Image& getFont() const { return m_font; }
        robot2D::ecs::Entity getEntity() const { return std::move(m_entity); }
    private:
        std::string m_imagePath;
        robot2D::Image m_font;
        robot2D::ecs::Entity m_entity;
    };
}