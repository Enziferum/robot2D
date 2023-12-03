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
#include <robot2D/Graphics/Texture.hpp>
#include <robot2D/Graphics/Font.hpp>
#include <robot2D/Util/ResourceHandler.hpp>

namespace editor {
    class LocalResourceManager {
    public:
        static LocalResourceManager* getManager() {
            static LocalResourceManager manager;
            return &manager;
        }

        bool hasTexture(const std::string& id) const {
            return m_textures.has(id);
        }

        bool hasFont(const std::string& id) const {
            return m_fonts.has(id);
        }

        robot2D::Texture* addTexture(const std::string& id) {
            return m_textures.add(id);
        }

        robot2D::Texture& getTexture(const std::string& id) {
            return m_textures.get(id);
        }

        robot2D::Font* addFont(const std::string& id) {
            return m_fonts.add(id);
        }

        robot2D::Font& getFont(const std::string& id) {
            return m_fonts.get(id);
        }

    private:
        robot2D::ResourceHandler<robot2D::Texture, std::string> m_textures;
        robot2D::ResourceHandler<robot2D::Font, std::string> m_fonts;
    };
}