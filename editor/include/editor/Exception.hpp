#pragma once
#include <string>
#include <stdexcept>

namespace editor {
    struct rb2d_base_exception: public std::runtime_error {
        rb2d_base_exception(const std::string& msg, const std::string& file, int line): std::runtime_error(msg) {
            m_realMessage = file + ':' + std::to_string(line) + ' ' + msg;
        }

        const char* what() const noexcept override {
            return m_realMessage.c_str();
        }
    private:
        std::string m_realMessage;
    };

    #define RB2D_EXCEPTION(message) throw rb2d_base_exception{message, __FILE__, __LINE__}
}