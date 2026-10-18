#include <editor/AssetPreviewCache.hpp>
#include <editor/AssetBrowserModel.hpp>
#include <editor/TaskQueue.hpp>
#include <algorithm>
#include <cstdint>

namespace editor {
    namespace fs = std::filesystem;

    AssetThumbnailTask::AssetThumbnailTask(ITaskFunction::Ptr callback, fs::path path,
        std::weak_ptr<int> lifetime) : ITask(std::move(callback)), m_path(std::move(path)),
        m_lifetime(std::move(lifetime)) {
    }

    void AssetThumbnailTask::execute() {
        if(m_lifetime.expired()) {
            markCancelled();
            return;
        }
        std::error_code error;
        const auto bytes = fs::file_size(m_path, error);
        if(error || bytes > 16 * 1024 * 1024) {
            fail("Preview unavailable: file missing, unreadable or larger than 16 MiB");
            return;
        }
        robot2D::Image source;
        if(!source.loadFromFile(m_path.u8string())) {
            fail("Could not decode preview: " + m_path.u8string());
            return;
        }
        if(m_lifetime.expired()) {
            markCancelled();
            return;
        }
        const auto size = source.getSize();
        const auto channels = static_cast<unsigned int>(source.getColorFormat());
        if(size.x == 0 || size.y == 0 || size.x > 8192 || size.y > 8192 || channels < 1 || channels > 4) {
            fail("Preview dimensions or channels unsupported");
            return;
        }
        const auto& pixels = static_cast<const robot2D::Image&>(source).getBuffer();
        if(pixels.size() < static_cast<std::size_t>(size.x) * size.y * channels) {
            fail("Preview pixel buffer incomplete");
            return;
        }
        const float factor = std::min(1.f, 128.f / static_cast<float>(std::max(size.x, size.y)));
        const unsigned int width = std::max(1u, static_cast<unsigned int>(size.x * factor));
        const unsigned int height = std::max(1u, static_cast<unsigned int>(size.y * factor));
        std::vector<std::uint8_t> thumbnail(static_cast<std::size_t>(width) * height * 4);
        for(unsigned int y = 0; y < height; ++y) {
            for(unsigned int x = 0; x < width; ++x) {
                const auto left = x * size.x / width;
                const auto right = (x + 1) * size.x / width;
                const auto top = y * size.y / height;
                const auto bottom = (y + 1) * size.y / height;
                std::uint64_t alpha = 0;
                std::uint64_t red = 0, green = 0, blue = 0;
                for(unsigned int row = top; row < bottom; ++row) {
                    for(unsigned int col = left; col < right; ++col) {
                        const auto input = (static_cast<std::size_t>(row) * size.x + col) * channels;
                        const auto a = channels == 2 || channels == 4 ? pixels[input + channels - 1] : 255;
                        alpha += a;
                        red += static_cast<std::uint64_t>(pixels[input]) * a;
                        green += static_cast<std::uint64_t>(pixels[input + (channels >= 3 ? 1 : 0)]) * a;
                        blue += static_cast<std::uint64_t>(pixels[input + (channels >= 3 ? 2 : 0)]) * a;
                    }
                }
                const auto output = (static_cast<std::size_t>(y) * width + x) * 4;
                thumbnail[output] = alpha ? static_cast<std::uint8_t>(red / alpha) : 0;
                thumbnail[output + 1] = alpha ? static_cast<std::uint8_t>(green / alpha) : 0;
                thumbnail[output + 2] = alpha ? static_cast<std::uint8_t>(blue / alpha) : 0;
                const auto samples = std::max(1u, (right - left) * (bottom - top));
                thumbnail[output + 3] = static_cast<std::uint8_t>(alpha / samples);
            }
        }
        m_image.create({width, height}, thumbnail.data(), robot2D::ImageColorFormat::RGBA,
            robot2D::ImageParameter::ClampToEdge);
    }

    AssetPreviewCache::AssetPreviewCache(std::size_t capacity) : m_state(std::make_shared<State>()),
        m_capacity(std::max(std::size_t {1}, capacity)) {
    }

    void AssetPreviewCache::clear() {
        for(const auto& [key, value]: m_state -> entries)
            m_retired.push_back(value);
        m_state = std::make_shared<State>();
    }

    void AssetPreviewCache::setRoot(const fs::path& root) {
        clear();
        m_root = root;
    }

    void AssetPreviewCache::beginFrame() {
        // Previous ImGui draw commands have been submitted before this frame begins.
        m_retired.clear();
        for(auto it = m_state -> pending.begin(); it != m_state -> pending.end();) {
            const auto status = TaskQueue::GetQueue() -> getStatus(*it);
            if(status == TaskStatus::Cancelled || status == TaskStatus::Bad)
                it = m_state -> pending.erase(it);
            else
                ++it;
        }
        for(const auto& [key, preview]: m_state -> entries) {
            if(preview -> status == PreviewStatus::Pending && !m_state -> pending.count(preview -> taskID)) {
                preview -> status = PreviewStatus::Failed;
                preview -> error = "Preview task was discarded; refresh to retry";
            }
        }
        ++m_frame;
        m_requests = 0;
        m_uploads = 0;
    }

    std::size_t AssetPreviewCache::size() const {
        return m_state -> entries.size();
    }

    void AssetPreviewCache::invalidate(const fs::path& path) {
        const auto key = AssetBrowserModel::resolveWithin(m_root, path).generic_u8string();
        const auto found = m_state -> entries.find(key);
        if(found != m_state -> entries.end()) {
            m_retired.push_back(found -> second);
            m_state -> entries.erase(found);
        }
    }

    AssetPreview* AssetPreviewCache::request(const fs::path& path) {
        const auto resolved = AssetBrowserModel::resolveWithin(m_root, path);
        if(resolved.empty())
            return nullptr;
        const auto key = resolved.generic_u8string();
        const auto now = std::chrono::steady_clock::now();
        auto found = m_state -> entries.find(key);
        std::error_code error;
        if(found != m_state -> entries.end()) {
            auto& preview = *found -> second;
            preview.lastFrame = m_frame;
            if(now - preview.checked < std::chrono::seconds(1))
                return &preview;
            preview.checked = now;
            const auto modified = fs::last_write_time(resolved, error);
            const auto bytes = error ? 0 : fs::file_size(resolved, error);
            if(!error && modified == preview.modified && bytes == preview.fileSize)
                return &preview;
            invalidate(resolved);
        }
        if(m_requests >= 2 || m_state -> pending.size() >= 4)
            return nullptr;
        if(m_state -> entries.size() >= m_capacity) {
            auto victim = m_state -> entries.end();
            for(auto it = m_state -> entries.begin(); it != m_state -> entries.end(); ++it) {
                if(it -> second -> lastFrame != m_frame &&
                    (victim == m_state -> entries.end() || it -> second -> lastFrame < victim -> second -> lastFrame))
                    victim = it;
            }
            if(victim == m_state -> entries.end())
                return nullptr;
            m_state -> entries.erase(victim);
        }
        auto preview = std::make_shared<AssetPreview>();
        preview -> checked = now;
        preview -> lastFrame = m_frame;
        preview -> modified = fs::last_write_time(resolved, error);
        preview -> fileSize = error ? 0 : fs::file_size(resolved, error);
        if(error) {
            preview -> status = PreviewStatus::Failed;
            preview -> error = error.message();
            m_state -> entries.emplace(key, preview);
            return preview.get();
        }
        m_state -> entries.emplace(key, preview);
        ++m_requests;
        const std::weak_ptr<State> state = m_state;
        const std::weak_ptr<AssetPreview> entry = preview;
        auto handle = std::make_shared<TaskID>(0);
        auto callback = [state, entry, handle](const AssetThumbnailTask& task) {
            if(const auto owner = state.lock()) {
                owner -> pending.erase(*handle);
                if(const auto result = entry.lock(); result && result -> status == PreviewStatus::Pending) {
                    if(task.getOutcome() == TaskOutcome::Succeeded) {
                        result -> image = task.image();
                        result -> status = PreviewStatus::Decoded;
                    }
                    else {
                        result -> error = task.getErrorMessage();
                        result -> status = PreviewStatus::Failed;
                    }
                }
            }
        };
        const auto id = TaskQueue::GetQueue() -> addAsyncTaskWithCompletion<AssetThumbnailTask>(
            std::move(callback), resolved, std::weak_ptr<int>{m_state -> lifetime});
        *handle = id;
        preview -> taskID = id;
        if(id != InvalidTaskId)
            m_state -> pending.insert(id);
        if(id == InvalidTaskId) {
            preview -> status = PreviewStatus::Failed;
            preview -> error = "Preview queue is stopping";
        }
        return preview.get();
    }

    robot2D::Texture* AssetPreviewCache::texture(AssetPreview& preview) {
        if(preview.status == PreviewStatus::Decoded && m_uploads < 2) {
            auto texture = std::make_unique<robot2D::Texture>();
            texture -> create(preview.image);
            preview.texture = std::move(texture);
            preview.status = PreviewStatus::Ready;
            ++m_uploads;
        }
        return preview.texture.get();
    }
}
