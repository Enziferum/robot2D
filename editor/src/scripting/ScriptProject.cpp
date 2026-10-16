#include <editor/scripting/ScriptProject.hpp>
#include <editor/ScriptBuildConfig.hpp>
#include <algorithm>
#include <vector>
#include <array>
#include <cerrno>
#include <cstring>
#ifdef _WIN32
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
#else
    #include <spawn.h>
    #include <sys/wait.h>
    #include <unistd.h>
extern char** environ;
#endif

namespace editor {
    namespace {
        constexpr std::size_t outputLimit = 4 * 1024 * 1024;
        void append(std::string& output, const char* data, std::size_t size) {
            if(output.size() < outputLimit) output.append(data, std::min(size, outputLimit - output.size()));
        }
#ifdef _WIN32
        std::wstring quote(const std::string& input) {
            auto value = std::filesystem::u8path(input).wstring();
            std::wstring output = L"\"";
            unsigned slashes = 0;
            for(wchar_t c: value) {
                if(c == L'\\') {
                    ++slashes; continue;
                }
                output.append(c == L'"' ? slashes * 2 + 1 : slashes, L'\\');
                slashes = 0; output += c;
            }
            output.append(slashes * 2, L'\\'); output += L'"'; return output;
        }
#endif
        ScriptBuildResult run(const std::vector<std::string>& args) {
            ScriptBuildResult result;
#ifdef _WIN32
            HANDLE readHandle = nullptr, writeHandle = nullptr;
            SECURITY_ATTRIBUTES security{sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
            if(!CreatePipe(&readHandle, &writeHandle, &security, 0)) {
                result.diagnostic = "Cannot create compiler output pipe"; return result;
            }
            if(!SetHandleInformation(readHandle, HANDLE_FLAG_INHERIT, 0)) {
                CloseHandle(readHandle); CloseHandle(writeHandle);
                result.diagnostic = "Cannot configure compiler output pipe"; return result;
            }
            STARTUPINFOW startup{}; startup.cb = sizeof(startup); startup.dwFlags = STARTF_USESTDHANDLES;
            startup.hStdOutput = writeHandle; startup.hStdError = writeHandle;
            startup.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
            PROCESS_INFORMATION process{};
            std::wstring command;
            for(const auto& arg: args) {
                if(!command.empty()) command += L' '; command += quote(arg);
            }
            const auto executable = std::filesystem::u8path(args.front()).wstring();
            const bool started = CreateProcessW(executable.c_str(), command.data(), nullptr, nullptr, TRUE,
                    CREATE_NO_WINDOW, nullptr, nullptr, &startup, &process);
            const auto error = GetLastError(); CloseHandle(writeHandle);
            if(!started) {
                CloseHandle(readHandle);
                result.diagnostic = "Cannot start script builder; Windows error=" + std::to_string(error);
                return result;
            }
            std::array<char, 4096> buffer{}; DWORD count = 0;
            while(ReadFile(readHandle, buffer.data(), static_cast<DWORD>(buffer.size()), &count, nullptr) && count)
                append(result.diagnostic, buffer.data(), count);
            CloseHandle(readHandle); WaitForSingleObject(process.hProcess, INFINITE);
            DWORD code = 1; GetExitCodeProcess(process.hProcess, &code);
            CloseHandle(process.hThread); CloseHandle(process.hProcess);
            result.success = code == 0;
#else
            int handles[2];
            if(pipe(handles)) {
                result.diagnostic = std::strerror(errno); return result;
            }
            posix_spawn_file_actions_t actions;
            int error = posix_spawn_file_actions_init(&actions);
            if(error) {
                close(handles[0]); close(handles[1]); result.diagnostic = std::strerror(error); return result;
            }
            error = posix_spawn_file_actions_adddup2(&actions, handles[1], STDOUT_FILENO);
            if(!error) error = posix_spawn_file_actions_adddup2(&actions, handles[1], STDERR_FILENO);
            if(!error) error = posix_spawn_file_actions_addclose(&actions, handles[0]);
            if(!error) error = posix_spawn_file_actions_addclose(&actions, handles[1]);
            std::vector<char*> argv;
            for(const auto& arg: args) argv.push_back(const_cast<char*>(arg.c_str()));
            argv.push_back(nullptr);
            pid_t process = 0;
            if(!error) error = posix_spawnp(&process, argv.front(), &actions, nullptr, argv.data(), environ);
            posix_spawn_file_actions_destroy(&actions); close(handles[1]);
            if(error) {
                close(handles[0]); result.diagnostic = std::strerror(error); return result;
            }
            std::array<char, 4096> buffer{};
            for(;;) {
                const auto count = read(handles[0], buffer.data(), buffer.size());
                if(count > 0) append(result.diagnostic, buffer.data(), static_cast<std::size_t>(count));
                else if(count < 0 && errno == EINTR) continue;
                else break;
            }
            close(handles[0]); int status = 0; pid_t waited;
            do { waited = waitpid(process, &status, 0); } while(waited < 0 && errno == EINTR);
            result.success = waited == process && WIFEXITED(status) && WEXITSTATUS(status) == 0;
#endif
            if(!result.success && result.diagnostic.empty()) result.diagnostic = "Script builder exited unsuccessfully";
            return result;
        }
        std::vector<std::string> command(const std::filesystem::path& root, const std::string& name,
            const ScriptProjectTools& tools) {
            return {tools.python.u8string(), "-X", "utf8", tools.generator.u8string(), "--project-root",
                    root.u8string(),
                    "--name", name, "--core", tools.core.u8string()};
        }
    }
    ScriptProjectTools scriptProjectTools() {
#ifdef _WIN32
        std::array<wchar_t, 32768> executable{};
        const auto length = GetModuleFileNameW(nullptr, executable.data(), static_cast<DWORD>(executable.size()));
        if(!length || length >= executable.size()) return {};
        return scriptProjectTools(std::filesystem::path(std::wstring(executable.data(), length)).parent_path());
#else
        std::array<char, 4096> executable{};
        const auto length = readlink("/proc/self/exe", executable.data(), executable.size());
        if(length <= 0 || static_cast<std::size_t>(length) >= executable.size()) return {};
        return scriptProjectTools(std::filesystem::path(std::string(executable.data(), length)).parent_path());
#endif
    }
    ScriptProjectTools scriptProjectTools(const std::filesystem::path& editorDirectory) {
        return {RB2D_SCRIPT_PYTHON, editorDirectory / "res/gen/gen_project.py",
                editorDirectory / "res/script/robot2D_ScriptCore.dll", RB2D_SCRIPT_MCS, RB2D_SCRIPT_MONO};
    }
    ScriptBuildResult buildProjectScripts(const std::filesystem::path& root, const std::string& name,
        const ScriptProjectTools& tools) {
        auto args = command(root, name, tools);
        args.insert(args.end(), {"--compiler", tools.compiler.u8string(), "--mono", tools.mono.u8string()});
        auto result = run(args);
        if(result.success) {
            result.assembly = root / "assets/scripts/bin" / std::filesystem::u8path(name + ".dll");
            if(!std::filesystem::is_regular_file(result.assembly)) {
                result.success = false; result.assembly.clear(); result.diagnostic += "\nCompiler produced no assembly";
            }
        }
        return result;
    }
    ScriptBuildResult generateProjectScripts(const std::filesystem::path& root, const std::string& name,
        const std::filesystem::path& sources, const ScriptProjectTools& tools) {
        auto args = command(root, name, tools);
        args.insert(args.end(), {"--generate", "--sources", sources.u8string()});
        auto generated = run(args);
        if(!generated.success) return generated;
        auto result = buildProjectScripts(root, name, tools);
        result.diagnostic = generated.diagnostic + result.diagnostic;
        return result;
    }
}
