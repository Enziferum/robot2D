/*********************************************************************
(c) Alex Raag 2021
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

#include <filesystem>

#include <robot2D/Graphics/Texture.hpp>
#include <robot2D/Util/ResourceHandler.hpp>

#include "IPanel.hpp"

namespace editor {

    struct AssetsPanelConfiguration {
        enum class ResourceIconType {
            File,
            Scene,
            Directory
        };

        const std::string iconsPath = "res/icons/";
        const float m_padding = 16.F;
        const float m_thumbnaleSize = 128.F;
        const std::unordered_map<ResourceIconType, std::string> iconPaths = {
                {ResourceIconType::File, "FileIcon.png"},
                {ResourceIconType::Directory, "DirectoryIcon.png"}
        };
    };

    class AssetsPanel: public IPanel {
    public:
        AssetsPanel();
        ~AssetsPanel() override = default;

        void setAssetsPath(const std::string& path);
        void render() override;
        void unlock() { wasSet = true; }
    private:
        std::filesystem::path m_currentPath;
        std::filesystem::path m_assetsPath;
        robot2D::ResourceHandler<robot2D::Texture, AssetsPanelConfiguration::ResourceIconType> m_assetsIcons;
        AssetsPanelConfiguration m_configuration;
        bool wasSet{false};
    };
}