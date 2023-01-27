#include <robot2D/Graphics/GL.hpp>
#include <robot2D/Util/Logger.hpp>

namespace robot2D {
    bool check_gl_errors(const char* file, std::uint_fast32_t line) {
        auto error = glGetError();
        if(error == GL_NO_ERROR)
            return true;
        switch(error) {
            case GL_INVALID_ENUM:
                break;
            case GL_INVALID_OPERATION:
                break;
            default:
                break;
        }
        return false;
    }
}