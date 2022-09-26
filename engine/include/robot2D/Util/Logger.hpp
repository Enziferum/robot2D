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
#include <robot2D/Config.hpp>

// Microsoft Visual Studio Warning
#ifdef _MSVC
    #pragma warning(push, 0)
#endif

#include <spdlog/spdlog.h>
#if defined(ROBOT2D_LINUX) || defined(ROBOT2D_WINDOWS)
    #include <spdlog/fmt/ostr.h>
    #include <spdlog/fmt/bundled/format.h>
#elif defined(ROBOT2D_MACOS)
    #include <fmt/format.h>
#endif
#ifdef _MSVC
    #pragma warning(pop)
#endif

#include <robot2D/Core/Vector2.hpp>

namespace fmt {
    template <>
    struct formatter<::robot2D::vec2u> {
        // Presentation format: 'd'.
        char presentation = 'd';

        // Parses format specifications of the form ['f' | 'e'].
        constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
            auto it = ctx.begin(), end = ctx.end();
            if (it != end && (*it == 'd' )) presentation = *it++;

            // Check if reached the end of the range:
            if (it != end && *it != '}')
                throw format_error("invalid format");

            // Return an iterator past the end of the parsed range:
            return it;
        }

        template <typename FormatContext>
        auto format(const robot2D::vec2u& value, FormatContext& ctx) -> decltype(ctx.out()) {
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
            auto it = ctx.begin(), end = ctx.end();
            if (it != end && (*it == 'f' || *it == 'e')) presentation = *it++;

            // Check if reached the end of the range:
            if (it != end && *it != '}')
                throw format_error("invalid format");

            // Return an iterator past the end of the parsed range:
            return it;
        }

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

// Core log macros
#define RB_CORE_TRACE(...)    ::logger::Log::getCoreLogger()->trace(__VA_ARGS__)
#define RB_CORE_INFO(...)     ::logger::Log::getCoreLogger()->info(__VA_ARGS__)
#define RB_CORE_WARN(...)     ::logger::Log::getCoreLogger()->warn(__VA_ARGS__)
#define RB_CORE_ERROR(...)    ::logger::Log::getCoreLogger()->error(__VA_ARGS__)
#define RB_CORE_CRITICAL(...) ::logger::Log::getCoreLogger()->critical(__VA_ARGS__)

// Editor log macros
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