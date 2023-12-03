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

        robot2D::Font* addFont(const std::string& id) {
            std::lock_guard<std::mutex> lockGuard{m_mutex};
            return m_fonts.add(id);
        }

        robot2D::Font& getFont(const std::string& id) {
            std::lock_guard<std::mutex> lockGuard{m_mutex};
            return m_fonts.get(id);
        }

        bool hasFont(const std::string& id) const {
            std::lock_guard<std::mutex> lockGuard{m_mutex};
            return m_fonts.has(id);
        }


    private:
        mutable std::mutex m_mutex;
        robot2D::ResourceHandler<robot2D::Image, std::string> m_images;
        robot2D::ResourceHandler<robot2D::Font, std::string> m_fonts;
    };
}