/*********************************************************************
   (c) Alex Raag 2023
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

#include <robot2D/Util/Logger.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <filesystem>
#include <array>
#include <mutex>
#include <atomic>
#include <stdexcept>

namespace logger {
    std::shared_ptr<spdlog::logger> Log::s_CoreLogger;
    std::shared_ptr<spdlog::logger> Log::s_EditorLogger;
    std::shared_ptr<spdlog::logger> Log::s_ClientLogger;
    std::string Log::s_FilePath;

    void Log::Init(const std::string& filePath) {
        static std::atomic<bool> initialized{false};
        if(initialized.load(std::memory_order_acquire))
            return;

        static std::mutex initializationMutex;
        std::lock_guard<std::mutex> lock(initializationMutex);
        if(initialized.load(std::memory_order_relaxed))
            return;
        if(filePath.empty())
            throw std::invalid_argument("Log file path must not be empty");

        constexpr std::array<const char*, 3> names{{"Robot2DCore", "Robot2DEditor", "Robot2DUser"}};
        for(const auto* name : names) {
            if(spdlog::get(name))
                throw spdlog::spdlog_ex(std::string{"Logger name already registered: "} + name);
        }

        auto absolutePath = std::filesystem::absolute(filePath).string();
        std::vector<spdlog::sink_ptr> logSinks;
        logSinks.emplace_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        logSinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(absolutePath, true));
        logSinks[0] -> set_pattern("%^[%T.%e] [%l] [thread %t] %n: %v%$");
        logSinks[1] -> set_pattern("[%Y-%m-%d %T.%e] [%l] [thread %t] %n: %v");

        std::array<std::shared_ptr<spdlog::logger>, 3> loggers;
        for(std::size_t i = 0; i < names.size(); ++i) {
            loggers[i] = std::make_shared<spdlog::logger>(names[i], logSinks.begin(), logSinks.end());
            loggers[i] -> set_level(spdlog::level::trace);
            // Keep the existing immediate visibility required by the editor console.
            loggers[i] -> flush_on(spdlog::level::trace);
        }

        try {
            for(const auto& log : loggers)
                spdlog::register_logger(log);
            loggers[0] -> info("[Logging] Session started; file='{}'; working_directory='{}'",
                absolutePath, std::filesystem::current_path().string());
        }
        catch(...) {
            for(const auto& log : loggers) {
                if(spdlog::get(log -> name()) == log)
                    spdlog::drop(log -> name());
            }
            throw;
        }

        s_FilePath = std::move(absolutePath);
        s_CoreLogger = std::move(loggers[0]);
        s_EditorLogger = std::move(loggers[1]);
        s_ClientLogger = std::move(loggers[2]);
        initialized.store(true, std::memory_order_release);
    }
}
