#pragma once

#include <chrono>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace editor {
    // Main-thread directory snapshots, shared by the tree and card view.
    class AssetBrowserModel {
    public:
        struct Listing {
            std::vector<std::filesystem::directory_entry> entries;
            std::string error;
            std::chrono::steady_clock::time_point checked;
        };
        bool setRoot(const std::filesystem::path& root);
        bool navigate(const std::filesystem::path& path);
        const std::filesystem::path& root() const {
            return m_root;
        }
        const std::filesystem::path& current() const {
            return m_current;
        }
        bool recoverCurrent();
        const Listing& list(const std::filesystem::path& path);
        void refresh() {
            m_listings.clear();
        }
        static std::filesystem::path resolveWithin(const std::filesystem::path& root,
            const std::filesystem::path& path);
    private:
        std::filesystem::path m_root;
        std::filesystem::path m_current;
        std::map<std::filesystem::path, Listing> m_listings;
        Listing m_unavailable;
    };
}
