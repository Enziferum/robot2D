#include <robot2D/Util/Logger.hpp>
#include <spdlog/sinks/null_sink.h>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <array>
#include <thread>
#include <exception>

namespace {
    void require(bool condition, const char* message) {
        if(!condition)
            throw std::runtime_error(message);
    }

    std::string readLog(const std::string& path) {
        std::ifstream input{path};
        require(input.is_open(), "Log file was not created");
        return {std::istreambuf_iterator<char>{input}, std::istreambuf_iterator<char>{}};
    }

    void requireUnregistered() {
        for(const auto* name : {"Robot2DCore", "Robot2DEditor", "Robot2DUser"})
            require(!spdlog::get(name), "Failed initialization left a registered channel");
    }

    void lazy() {
        RB_CORE_INFO("lazy-core-token");
        RB_EDITOR_WARN("lazy-editor-token");
        RB_ERROR("lazy-client-token");
        const auto& path = logger::Log::getFilePath();
        require(std::filesystem::path{path}.is_absolute(), "Log path must be absolute");
        const auto text = readLog(path);
        require(text.find("lazy-core-token") != std::string::npos, "Core message was lost");
        require(text.find("lazy-editor-token") != std::string::npos, "Editor message was lost");
        require(text.find("lazy-client-token") != std::string::npos, "Client message was lost");
    }

    void repeat() {
        std::filesystem::remove("ignored.log");
        logger::Log::Init("session.log");
        auto core = logger::Log::getCoreLogger();
        const auto path = logger::Log::getFilePath();
        RB_INFO("before-repeat-token");
        logger::Log::Init("ignored.log");
        logger::Log::Init();
        require(logger::Log::getCoreLogger() == core, "Repeated Init replaced a channel");
        require(logger::Log::getFilePath() == path, "Repeated Init changed the log path");
        require(!std::filesystem::exists("ignored.log"), "Repeated Init opened another file");
        require(readLog(path).find("before-repeat-token") != std::string::npos, "Repeated Init truncated the log");
    }

    void retry() {
        std::filesystem::create_directory("blocked");
        bool failed = false;
        try {
            logger::Log::Init("blocked");
        }
        catch(const spdlog::spdlog_ex&) {
            failed = true;
        }
        require(failed, "Opening a directory as the log must fail");
        requireUnregistered();
        logger::Log::Init("recovered.log");
        RB_INFO("retry-token");
        require(readLog(logger::Log::getFilePath()).find("retry-token") != std::string::npos,
            "Retry did not recover logging");
    }

    void emptyPath() {
        bool failed = false;
        try {
            logger::Log::Init("");
        }
        catch(const std::invalid_argument&) {
            failed = true;
        }
        require(failed, "Empty path must be rejected");
        requireUnregistered();
        logger::Log::Init("valid.log");
        require(std::filesystem::exists(logger::Log::getFilePath()), "Retry after an empty path failed");
    }

    void conflict() {
        auto sink = std::make_shared<spdlog::sinks::null_sink_mt>();
        auto existing = std::make_shared<spdlog::logger>("Robot2DEditor", sink);
        spdlog::register_logger(existing);
        {
            std::ofstream file{"preserved.log"};
            file << "keep-existing-file";
        }
        bool failed = false;
        try {
            logger::Log::Init("preserved.log");
        }
        catch(const spdlog::spdlog_ex&) {
            failed = true;
        }
        require(failed, "Occupied channel name must be rejected");
        require(spdlog::get("Robot2DEditor") == existing, "Foreign logger was replaced");
        require(!spdlog::get("Robot2DCore") && !spdlog::get("Robot2DUser"), "Conflict published partial channels");
        require(readLog("preserved.log") == "keep-existing-file", "Conflict truncated the existing file");
        spdlog::drop("Robot2DEditor");
        logger::Log::Init("recovered.log");
        require(logger::Log::getEditorLogger() != existing, "Retry did not create a new editor channel");
    }

    void concurrent() {
        std::array<std::exception_ptr, 8> errors{};
        std::array<std::thread, 8> threads;
        for(std::size_t i = 0; i < threads.size(); ++i) {
            threads[i] = std::thread([i, &errors]() {
                try {
                    logger::Log::Init("concurrent.log");
                    RB_INFO("thread-token-{}", i);
                }
                catch(...) {
                    errors[i] = std::current_exception();
                }
            });
        }
        for(auto& thread : threads)
            thread.join();
        for(const auto& error : errors) {
            if(error)
                std::rethrow_exception(error);
        }
        const auto text = readLog(logger::Log::getFilePath());
        for(std::size_t i = 0; i < threads.size(); ++i)
            require(text.find("thread-token-" + std::to_string(i)) != std::string::npos,
                "Concurrent logging lost a message");
        require(logger::Log::getCoreLogger() == spdlog::get("Robot2DCore"),
            "Concurrent initialization published a different channel");
    }
}

int main(int argc, char** argv) {
    try {
        require(argc == 2, "Expected one logger scenario");
        const std::string scenario{argv[1]};
        if(scenario == "lazy") lazy();
        else if(scenario == "repeat") repeat();
        else if(scenario == "retry") retry();
        else if(scenario == "empty-path") emptyPath();
        else if(scenario == "conflict") conflict();
        else if(scenario == "concurrent") concurrent();
        else throw std::runtime_error("Unknown logger scenario");
        return 0;
    }
    catch(const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
