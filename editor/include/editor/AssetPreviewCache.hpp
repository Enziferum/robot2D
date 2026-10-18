#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <chrono>
#include <robot2D/Graphics/Image.hpp>
#include <robot2D/Graphics/Texture.hpp>
#include <editor/Task.hpp>

namespace editor {
    class AssetThumbnailTask final : public ITask {
    public:
        AssetThumbnailTask(ITaskFunction::Ptr callback, std::filesystem::path path,
            std::weak_ptr<int> lifetime);
        void execute() override;
        const robot2D::Image& image() const {
            return m_image;
        }
    private:
        std::filesystem::path m_path;
        std::weak_ptr<int> m_lifetime;
        robot2D::Image m_image;
    };

    enum class PreviewStatus { Pending, Decoded, Ready, Failed };
    struct AssetPreview {
        PreviewStatus status{PreviewStatus::Pending};
        robot2D::Image image;
        std::unique_ptr<robot2D::Texture> texture;
        std::string error;
        std::filesystem::file_time_type modified;
        std::uintmax_t fileSize{};
        std::uint64_t lastFrame{};
        TaskID taskID{};
        std::chrono::steady_clock::time_point checked;
    };

    // All cache operations and callbacks run on the main thread; only decoding runs in TaskQueue.
    class AssetPreviewCache {
    public:
        explicit AssetPreviewCache(std::size_t capacity = 64);
        void setRoot(const std::filesystem::path& root);
        void beginFrame();
        AssetPreview* request(const std::filesystem::path& path);
        robot2D::Texture* texture(AssetPreview& preview);
        void invalidate(const std::filesystem::path& path);
        void clear();
        std::size_t size() const;
    private:
        struct State {
            std::unordered_map<std::string, std::shared_ptr<AssetPreview> > entries;
            std::shared_ptr<int> lifetime{std::make_shared<int>(0)};
            std::unordered_set<TaskID> pending;
        };
        std::shared_ptr<State> m_state;
        std::vector<std::shared_ptr<AssetPreview> > m_retired;
        std::filesystem::path m_root;
        std::size_t m_capacity;
        std::uint64_t m_frame{1};
        int m_requests{};
        int m_uploads{};
    };
}
