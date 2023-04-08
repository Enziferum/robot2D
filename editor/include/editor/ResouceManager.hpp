#pragma once

#include <string>
#include <mutex>

#include <robot2D/Graphics/Texture.hpp>
#include <robot2D/Graphics/Font.hpp>
#include <robot2D/Util/ResourceHandler.hpp>

namespace editor {
    class ResourceManager {
    public:
        static ResourceManager* getManager() {
            static ResourceManager resourceManager;
            return &resourceManager;
        }

        robot2D::Image* addImage(const std::string& id) {
            std::lock_guard<std::mutex> lockGuard{m_mutex};
            return m_images.add(id);
        }

        robot2D::Image& getImage(const std::string& id) {
            std::lock_guard<std::mutex> lockGuard{m_mutex};
            return m_images.get(id);
        }

        bool hasImage(const std::string& id) const {
            std::lock_guard<std::mutex> lockGuard{m_mutex};
            return m_images.has(id);
        }

    private:
        mutable std::mutex m_mutex;
        robot2D::ResourceHandler<robot2D::Image, std::string> m_images;
        robot2D::ResourceHandler<robot2D::Font, std::string> m_fonts;
    };
}