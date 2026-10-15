/*********************************************************************
   (c) Alex Raag 2026
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

#include <editor/FileApi.hpp>
#include <stdexcept>

namespace editor {
    namespace fs = std::filesystem;

    namespace {
        fs::path assetPath(const std::string& value) {
            if(value.empty() || value.find('\0') != std::string::npos)
                throw std::invalid_argument("Empty asset path or embedded NUL");
            auto portable = value;
            for(auto& character: portable)
                if(character == '\\') character = '/';
            return fs::u8path(portable);
        }
    }

    std::string projectAssetReference(const std::string& projectPath, const std::string& path) {
        const auto root = fs::weakly_canonical(fs::absolute(assetPath(projectPath)));
        const auto input = assetPath(path);
        const auto full = fs::weakly_canonical(input.is_absolute() ? input : root / input);
        const auto relative = full.lexically_relative(root);
        if(relative.empty() || relative.is_absolute() || *relative.begin() != "assets")
            throw std::invalid_argument("Asset must be inside project assets: " + path);
        for(const auto& part: relative)
            if(part == "..")
                throw std::invalid_argument("Asset escapes project: " + path);
        return relative.generic_u8string();
    }

    std::string resolveProjectAsset(const std::string& projectPath, const std::string& reference) {
        const auto relative = projectAssetReference(projectPath, reference);
        return (fs::absolute(assetPath(projectPath)) / fs::u8path(relative)).lexically_normal().u8string();
    }

    std::string importProjectAsset(const std::string& projectPath, const std::string& source,
        const std::string& destinationDirectory) {
        const auto from = fs::weakly_canonical(fs::absolute(assetPath(source)));
        if(!fs::is_regular_file(from))
            throw std::invalid_argument("Asset source is not a regular file: " + source);
        const auto reference = projectAssetReference(projectPath,
                (assetPath(destinationDirectory) / from.filename()).generic_u8string());
        const auto target = fs::u8path(resolveProjectAsset(projectPath, reference));
        if(fs::exists(target) && fs::equivalent(from, target))
            return reference;
        fs::create_directories(target.parent_path());
        fs::copy_file(from, target, fs::copy_options::none);
        return reference;
    }

    void copyProjectAssets(const std::string& projectPath, const std::string& destinationRoot) {
        const auto source = fs::weakly_canonical(fs::absolute(assetPath(projectPath)) / "assets");
        const auto target = fs::weakly_canonical(fs::absolute(assetPath(destinationRoot)) / "assets");
        const auto relativeTarget = target.lexically_relative(source);
        if(target == source || (!relativeTarget.empty() && !relativeTarget.is_absolute()
            && *relativeTarget.begin() != ".."))
            throw std::invalid_argument("Asset package destination is inside source assets");
        if(!fs::is_directory(source))
            throw std::invalid_argument("Project assets directory is missing: " + source.u8string());
        if(fs::exists(target))
            throw std::invalid_argument("Asset package destination already exists: " + target.u8string());
        // Validate before writing so links cannot make the package depend on external files.
        for(const auto& entry: fs::recursive_directory_iterator(source)) {
            if(entry.is_symlink() || (!entry.is_directory() && !entry.is_regular_file()))
                throw std::invalid_argument("Unsupported asset package entry: " + entry.path().u8string());
        }
        fs::create_directories(target.parent_path());
        fs::copy(source, target, fs::copy_options::recursive);
    }

    bool createDirectory(const std::string& path) {
        if(path.empty())
            return false;
        return fs::create_directories(fs::path(path));
    }

    bool createDirectory(const std::string& basePath, const std::string& appendPath) {
        auto fullPath = fs::path(basePath);
        fullPath /= fs::path(appendPath);
        return fs::create_directories(fullPath);
    }

    bool deleteDirectory(const std::string& path) {
        if(path.empty())
            return false;

        fs::remove_all(fs::path(path));
        return true;
    }

    std::string addFilename(const std::string& path, const std::string& filename) {
        if(path.empty())
            return "";
        if(filename.empty())
            return "";
        fs::path resultPath = fs::path(path);
        resultPath.append(filename);
        return resultPath.string();
    }

    std::string combinePath(const std::string& basePath, const std::string& appendPath) {
        auto fullPath = fs::path(basePath);
        fullPath /= fs::path(appendPath);
        return fullPath.string();
    }

    bool hasFile(const std::string& path) {
        fs::path filePath{path};
        if(exists(filePath))
            return true;

        return false;
    }

    std::string getFileName(const std::string& path) {
        fs::path filePath{path};
        if(!hasFile(path))
            return {};
        return filePath.filename().string();
    }

    std::string getFileExtension(const std::string& path) {
        fs::path filePath{path};
        if(!hasFile(path))
            return {};
        return filePath.extension().string();
    }


    std::string cutPath(const std::string& fullPath, const std::string& anchor) {
        std::string path;
        fs::path fsPath{fullPath};
        auto relative = fsPath.relative_path().string();
        auto pos = relative.find(anchor);
        path = relative.substr(pos, relative.size() - pos);
        return path;
    }
}

