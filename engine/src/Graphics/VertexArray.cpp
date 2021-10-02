//
// Created by Necromant on 02.10.2021.
//

#include <robot2D/Graphics/VertexArray.hpp>
#include "Desktop/OpenGLVertexArray.hpp"

namespace robot2D {
    VertexArray::Ptr VertexArray::Create() {
        return std::make_shared<OpenGLVertexArray>();
    }
}