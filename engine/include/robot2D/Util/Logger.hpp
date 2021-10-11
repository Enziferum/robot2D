/*********************************************************************
(c) Alex Raag 2021
https://github.com/Enziferum
robot2D - Zlib license.
This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:
1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.
2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any
source distribution.
*********************************************************************/

#pragma once
#include <iostream>

#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
//#include <spdlog/fmt/bundled/format.h>
#include <fmt/format.h>
#pragma warning(pop)

#include <robot2D/Core/Vector2.hpp>

namespace fmt {
    // todo remove presentation for int param
    template <>
    struct formatter<::robot2D::vec2u> {
        // Presentation format: 'f' - fixed, 'e' - exponential.
        char presentation = 'f';

        // Parses format specifications of the form ['f' | 'e'].
        constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
            // [ctx.begin(), ctx.end()) is a character range that contains a part of
            // the format string starting from the format specifications to be parsed,
            // e.g. in
            //
            //   fmt::format("{:f} - point of interest", point{1, 2});
            //
            // the range will contain "f} - point of interest". The formatter should
            // parse specifiers until '}' or the end of the range. In this example
            // the formatter should parse the 'f' specifier and return an iterator
            // pointing to '}'.

            // Parse the presentation format and store it in the formatter:
            auto it = ctx.begin(), end = ctx.end();
            if (it != end && (*it == 'f' || *it == 'e')) presentation = *it++;

            // Check if reached the end of the range:
            if (it != end && *it != '}')
                throw format_error("invalid format");

            // Return an iterator past the end of the parsed range:
            return it;
        }

        // Formats the point p using the parsed format specification (presentation)
        // stored in this formatter.
        template <typename FormatContext>
        auto format(const robot2D::vec2u& value, FormatContext& ctx) -> decltype(ctx.out()) {
            // ctx.out() is an output iterator to write to.
            //= 'f' ? "({:.1f}, {:.1f})" : "({:.1e}, {:.1e})"
            return format_to(
                    ctx.out(),
                    "({:d}, {:d})",
                    value.x, value.y);
        }
    };

    template <>
    struct formatter<robot2D::vec2f> {
        // Presentation format: 'f' - fixed, 'e' - exponential.
        char presentation = 'f';

        // Parses format specifications of the form ['f' | 'e'].
        constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
            // [ctx.begin(), ctx.end()) is a character range that contains a part of
            // the format string starting from the format specifications to be parsed,
            // e.g. in
            //
            //   fmt::format("{:f} - point of interest", point{1, 2});
            //
            // the range will contain "f} - point of interest". The formatter should
            // parse specifiers until '}' or the end of the range. In this example
            // the formatter should parse the 'f' specifier and return an iterator
            // pointing to '}'.

            // Parse the presentation format and store it in the formatter:
            auto it = ctx.begin(), end = ctx.end();
            if (it != end && (*it == 'f' || *it == 'e')) presentation = *it++;

            // Check if reached the end of the range:
            if (it != end && *it != '}')
                throw format_error("invalid format");

            // Return an iterator past the end of the parsed range:
            return it;
        }

        // Formats the point p using the parsed format specification (presentation)
        // stored in this formatter.
        template <typename FormatContext>
        auto format(const robot2D::vec2u& value, FormatContext& ctx) -> decltype(ctx.out()) {
            // ctx.out() is an output iterator to write to.
            return format_to(
                    ctx.out(),
                    presentation == 'f' ? "({:.1f}, {:.1f})" : "({:.1e}, {:.1e})",
                    value.x, value.y);
        }
    };
}



namespace logger {
    enum class logType {
        error,
        warn,
        info
    };

    extern bool debug;

    template<template<typename, typename ...> class Container,
            typename ValueType, typename ... Args>
    std::ostream &operator<<(std::ostream &os, const Container<ValueType, Args...> &c) {
        for (const auto &it: c)
            os << it;
        return os;
    }

    template<typename K, typename V>
    std::ostream &operator<<(std::ostream &os, const std::pair<K, V> &pair) {
        os << '[' << pair.first << ':' << pair.second << ']';
        return os;
    }

    void tprintf(logType type, const char *msg);

    template<typename T, typename ...Args>
    void tprintf(logType type, const char *format, const T &value, Args...args) {
        for (; *format != '\0'; format++) {
            if (*format == '%') {
                if (type == logType::info)
                    std::cout << value;
                if (type == logType::error)
                    std::cerr << value;
                tprintf(type, format + 1, args...);
                return;
            }
            if (type == logType::info)
                std::cout << *format;
            if (type == logType::error)
                std::cerr << *format;
        }
    }

    template<typename ...Args>
    void log(logType type, const char* msg, Args... args) {
        if (debug)
            tprintf(type, msg, args...);
    }


    class Log
    {
    public:
        static void Init();

        static std::shared_ptr<spdlog::logger>& getCoreLogger() { return s_CoreLogger; }
        static std::shared_ptr<spdlog::logger>& getEditorLogger() { return s_EditorLogger; }
        static std::shared_ptr<spdlog::logger>& getClientLogger() { return s_ClientLogger; }
    private:
        static std::shared_ptr<spdlog::logger> s_CoreLogger;
        static std::shared_ptr<spdlog::logger> s_EditorLogger;
        static std::shared_ptr<spdlog::logger> s_ClientLogger;
    };

}

#define LOG_ERROR_E(msg, ...) logger::log(logger::logType::error, msg, #__VA_ARGS__);
#define LOG_ERROR(msg, ...) logger::log(logger::logType::error, msg, __VA_ARGS__);
#define LOG_WARN(msg, ...) logger::log(logger::logType::warn, msg, __VA_ARGS__);
#define LOG_INFO(msg, ...) logger::log(logger::logType::info, msg, __VA_ARGS__);
#define LOG_INFO_E(msg, ...) logger::log(logger::logType::info, msg, #__VA_ARGS__);

// Core log macros
#define RB_CORE_TRACE(...)    ::logger::Log::getCoreLogger()->trace(__VA_ARGS__)
#define RB_CORE_INFO(...)     ::logger::Log::getCoreLogger()->info(__VA_ARGS__)
#define RB_CORE_WARN(...)     ::logger::Log::getCoreLogger()->warn(__VA_ARGS__)
#define RB_CORE_ERROR(...)    ::logger::Log::getCoreLogger()->error(__VA_ARGS__)
#define RB_CORE_CRITICAL(...) ::logger::Log::getCoreLogger()->critical(__VA_ARGS__)

#define RB_EDITOR_TRACE(...)    ::logger::Log::getEditorLogger()->trace(__VA_ARGS__)
#define RB_EDITOR_INFO(...)     ::logger::Log::getEditorLogger()->info(__VA_ARGS__)
#define RB_EDITOR_WARN(...)     ::logger::Log::getEditorLogger()->warn(__VA_ARGS__)
#define RB_EDITOR_ERROR(...)    ::logger::Log::getEditorLogger()->error(__VA_ARGS__)
#define RB_EDITOR_CRITICAL(...) ::logger::Log::getEditorLogger()->critical(__VA_ARGS__)

// Client log macros
#define RB_TRACE(...)         ::logger::Log::getClientLogger()->trace(__VA_ARGS__)
#define RB_INFO(...)          ::logger::Log::getClientLogger()->info(__VA_ARGS__)
#define RB_WARN(...)          ::logger::Log::getClientLogger()->warn(__VA_ARGS__)
#define RB_ERROR(...)         ::logger::Log::getClientLogger()->error(__VA_ARGS__)
#define RB_CRITICAL(...)      ::logger::Log::getClientLogger()->critical(__VA_ARGS__)