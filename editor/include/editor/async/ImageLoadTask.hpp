#pragma once

#include <editor/Task.hpp>
#include <robot2D/Graphics/Image.hpp>

namespace editor {
    class ImageLoadTask: public ITask {
    public:
        ImageLoadTask();
        ~ImageLoadTask() override = default;
    };
}