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
#if defined(ROBOT2D_WINDOWS)
  #include <spdlog/fmt/ostr.h>
#elif defined(ROBOT2D_LINUX)
  #include <fmt/format.h>
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

        // Parses the optional decimal presentation 'd'.
        constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin()) {
            auto it = ctx.begin(), end = ctx.end();
            if(it != end && (*it == 'd'))   presentation = *it++;

            // Check if reached the end of the range:
            if(it != end && *it != '}')
                throw format_error("invalid format");

            // Return an iterator past the end of the parsed range:
            return it;
        }

        template <typename FormatContext>
        auto format(const robot2D::vec2u& value, FormatContext& ctx) const -> decltype(ctx.out()) {
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
            if(it != end && (*it == 'f' || *it == 'e'))  presentation = *it++;

            // Check if reached the end of the range:
            if(it != end && *it != '}')
                throw format_error("invalid format");

            // Return an iterator past the end of the parsed range:
            return it;
        }

        template <typename FormatContext>
        auto format(const robot2D::vec2f& value, FormatContext& ctx) const -> decltype(ctx.out()) {
            // ctx.out() is an output iterator to write to.
            if(presentation == 'e')
                return format_to(ctx.out(), "({:.1e}, {:.1e})", value.x, value.y);
            return format_to(ctx.out(), "({:.1f}, {:.1f})", value.x, value.y);
        }
    };
}



namespace logger {
    /**
     * @brief Provides the shared core, editor and client logging channels.
     * @details Initialization is once per process; getters initialize lazily.
     * All channels share thread-safe console/file sinks and flush each message.
     * Do not remove their registry entries or reset their slots during use.
     */
    class Log {
    public:
        /**
         * @brief Initializes all channels once, truncating the selected file for this session.
         * @param filePath Log destination; relative paths use the current working directory.
         * @details Later calls preserve the channels, path and file contents, ignoring filePath.
         * Concurrent initialization is serialized. A failed attempt publishes no channels and
         * may be retried. Call explicitly before logging to choose a non-default destination.
         * @throws std::invalid_argument If the first initialization receives an empty path.
         * @throws spdlog::spdlog_ex For sink/registration errors, including occupied channel names.
         * @throws std::filesystem::filesystem_error If resolving the destination fails.
         */
        static void Init(const std::string& filePath = "robot2D.log");

        /// @brief Returns the session's absolute log path, initializing with defaults if needed.
        static const std::string& getFilePath() {
            Init(); return s_FilePath;
        }

        /// @brief Returns the core channel; initialization failures propagate.
        /// @warning Replacing the slot is supported only while no threads use the channel.
        static std::shared_ptr<spdlog::logger>& getCoreLogger() {
            Init(); return s_CoreLogger;
        }
        /// @brief Returns the editor channel; initialization failures propagate.
        /// @warning Replacing the slot is supported only while no threads use the channel.
        static std::shared_ptr<spdlog::logger>& getEditorLogger() {
            Init(); return s_EditorLogger;
        }
        /// @brief Returns the client channel; initialization failures propagate.
        /// @warning Replacing the slot is supported only while no threads use the channel.
        static std::shared_ptr<spdlog::logger>& getClientLogger() {
            Init(); return s_ClientLogger;
        }
    private:
        static std::shared_ptr<spdlog::logger> s_CoreLogger;
        static std::string s_FilePath;
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
