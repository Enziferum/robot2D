#include <editor/ClassID.hpp>
#include <atomic>
namespace editor {
    namespace {
        std::atomic<int> g_last_class_index;
    }

    class_id::class_id(const char* name) : m_name{name}, m_index{++g_last_class_index} {
    }

}
