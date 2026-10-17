#include <editor/runtime/LaunchConfig.hpp>
#include <editor/serializers/YamlFields.hpp>
#include <array>
#include <fstream>
#include <algorithm>
#ifdef _WIN32
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
#else
    #include <unistd.h>
#endif

namespace editor::runtime {
    namespace fs = std::filesystem;
    namespace {
        fs::path canonicalExisting(const fs::path& path) {
#ifdef _WIN32
            const auto handle = CreateFileW(path.c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
            if(handle == INVALID_HANDLE_VALUE)
                throw std::runtime_error("Cannot open player package path: " + path.u8string());
            std::array<wchar_t, 32768> buffer{};
            const auto length = GetFinalPathNameByHandleW(handle, buffer.data(), static_cast<DWORD>(buffer.size()),
                    FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
            CloseHandle(handle);
            if(!length || length >= buffer.size())
                throw std::runtime_error("Cannot resolve player package path: " + path.u8string());
            std::wstring resolved(buffer.data(), length);
            if(resolved.compare(0, 8, L"\\\\?\\UNC\\") == 0) resolved = L"\\\\" + resolved.substr(8);
            else if(resolved.compare(0, 4, L"\\\\?\\") == 0) resolved.erase(0, 4);
            return fs::path(resolved).lexically_normal();
#else
            return fs::canonical(path);
#endif
        }
    }

    fs::path executableDirectory() {
#ifdef _WIN32
        std::array<wchar_t, 32768> buffer{};
        const auto length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
        if(!length || length >= buffer.size()) throw std::runtime_error("Cannot determine player executable path");
        return fs::path(std::wstring(buffer.data(), length)).parent_path();
#else
        std::array<char, 4096> buffer{};
        const auto length = readlink("/proc/self/exe", buffer.data(), buffer.size());
        if(length <= 0 || static_cast<std::size_t>(length) >= buffer.size())
            throw std::runtime_error("Cannot determine player executable path");
        return fs::path(std::string(buffer.data(), length)).parent_path();
#endif
    }
    namespace {
        fs::path packagePath(const fs::path& root, const YAML::Node& node, const char* key, bool directory = false) {
            auto value = yaml::read<std::string>(node, key);
            if(value.empty() || value.find('\0') != std::string::npos || value.find(':') != std::string::npos)
                throw std::runtime_error(std::string(key) + ": invalid package path");
            std::replace(value.begin(), value.end(), '\\', '/');
            auto relative = fs::u8path(value);
            if(relative.is_absolute() || relative.has_root_name())
                throw std::runtime_error(std::string(key) + ": expected a relative package path");
            auto resolved = canonicalExisting(root / relative);
            auto mismatch = std::mismatch(root.begin(), root.end(), resolved.begin(), resolved.end());
            if(mismatch.first != root.end() || resolved == root)
                throw std::runtime_error(std::string(key) + ": path escapes package: " + value);
            if(directory ? !fs::is_directory(resolved) : !fs::is_regular_file(resolved))
                throw std::runtime_error(std::string(key) + ": missing " + resolved.u8string());
            return resolved;
        }
    }
    LaunchConfig loadLaunchConfig(const fs::path& manifest) {
        std::ifstream input(manifest, std::ios::binary);
        if(!input) throw std::runtime_error("Cannot open launch manifest: " + manifest.u8string());
        auto data = YAML::Load(input);
        if(yaml::read<unsigned>(data, "Version") != 1) throw std::runtime_error("Unsupported player manifest version");
        LaunchConfig result;
        result.root = canonicalExisting(manifest).parent_path();
        result.title = yaml::read<std::string>(data, "Title");
        if(result.title.empty() || result.title.find('\0') != std::string::npos)
            throw std::runtime_error("Invalid player title");
        result.scene = packagePath(result.root, data, "StartScene");
        result.scripts = packagePath(result.root, data, "ScriptAssembly");
        result.core = packagePath(result.root, data, "ScriptCore");
        result.monoLib = packagePath(result.root, data, "MonoLib", true);
        result.monoEtc = packagePath(result.root, data, "MonoEtc", true);
        if(!fs::is_regular_file(result.monoLib / "mono/4.5/mscorlib.dll"))
            throw std::runtime_error("MonoLib: missing mono/4.5/mscorlib.dll");
        if(!fs::is_regular_file(result.monoEtc / "mono/config"))
            throw std::runtime_error("MonoEtc: missing mono/config");
        auto window = yaml::required(data, "Window");
        result.size = {yaml::read<unsigned>(window, "Width"), yaml::read<unsigned>(window, "Height")};
        result.fullscreen = yaml::read<bool>(window, "Fullscreen");
        if(!result.size.x || !result.size.y || result.size.x > 16384 || result.size.y > 16384)
            throw std::runtime_error("Window size must be between 1 and 16384");
        return result;
    }
}
