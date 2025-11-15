#include <atomic>
#include <robot2D/Ecs/ClassID.hpp>

namespace robot2D::ecs {
    namespace
    {
        std::atomic<int> g_last_class_index;
    }

    class_id::class_id(const char* name): m_name{name}, m_index{++g_last_class_index} {}

} // namespace robot2D::ecs