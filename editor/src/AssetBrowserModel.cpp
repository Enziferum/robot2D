#include <editor/AssetBrowserModel.hpp>
#include <algorithm>

namespace editor {
    namespace fs = std::filesystem;

    fs::path AssetBrowserModel::resolveWithin(const fs::path& root, const fs::path& path) {
        if(root.empty() || path.empty())
            return {};
        std::error_code error;
        const auto resolved = fs::weakly_canonical(path, error);
        if(error)
            return {};
        auto base = root.begin();
        auto item = resolved.begin();
        for(; base != root.end(); ++base, ++item) {
            if(item == resolved.end() || *base != *item)
                return {};
        }
        return resolved;
    }

    bool AssetBrowserModel::setRoot(const fs::path& root) {
        refresh();
        m_root.clear();
        m_current.clear();
        if(root.empty())
            return false;
        std::error_code error;
        auto resolved = fs::weakly_canonical(root, error);
        if(error || !fs::is_directory(resolved, error))
            return false;
        m_root = resolved;
        m_current = resolved;
        return true;
    }

    bool AssetBrowserModel::navigate(const fs::path& path) {
        const auto resolved = resolveWithin(m_root, path);
        std::error_code error;
        if(resolved.empty() || !fs::is_directory(resolved, error))
            return false;
        m_current = resolved;
        return true;
    }

    bool AssetBrowserModel::recoverCurrent() {
        auto path = m_current;
        while(!path.empty()) {
            if(navigate(path))
                return true;
            if(path == m_root)
                break;
            path = path.parent_path();
        }
        return false;
    }

    const AssetBrowserModel::Listing& AssetBrowserModel::list(const fs::path& path) {
        const auto resolved = resolveWithin(m_root, path);
        if(resolved.empty()) {
            m_unavailable.error = "Folder is outside the asset root";
            return m_unavailable;
        }
        auto found = m_listings.find(resolved);
        const auto now = std::chrono::steady_clock::now();
        if(found != m_listings.end() && now - found -> second.checked < std::chrono::seconds(1))
            return found -> second;
        if(m_listings.size() >= 128)
            m_listings.clear();
        auto& listing = m_listings[resolved];
        listing = {};
        listing.checked = now;
        std::error_code error;
        for(fs::directory_iterator it(resolved, error), end; !error && it != end; it.increment(error))
            listing.entries.push_back(*it);
        if(error)
            listing.error = error.message();
        std::stable_sort(listing.entries.begin(), listing.entries.end(), [](const auto& left, const auto& right) {
            std::error_code first, second;
            const bool a = left.is_directory(first);
            const bool b = right.is_directory(second);
            if(a != b)
                return a;
            return left.path().filename() < right.path().filename();
        });
        return listing;
    }
}
