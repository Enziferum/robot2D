#pragma once
#include <utility>

namespace editor {
    #define BIND_CLASS_FN(fn)[this](auto&&... args) -> decltype(auto) { \
            return this -> fn(std::forward<decltype(args)>(args)...);      \
    }
}