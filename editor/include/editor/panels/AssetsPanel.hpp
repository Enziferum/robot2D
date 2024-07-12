/*********************************************************************
(c) Alex Raag 2024
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
#include <robot2D/Core/MessageBus.hpp>
#include <editor/Observer.hpp>
#include <editor/UIManager.hpp>
#include <editor/PrefabManager.hpp>
#include <editor/UIInteractor.hpp>

#include "IPanel.hpp"

namespace editor {

    struct AssetsPanelConfiguration {
        enum class ResourceIconType {
            File,
            Scene,
            Image,
            Script,
            Directory,
            Font,
            Prefab,
            Animation
        };

        const std::string iconsPath = "res/icons/";
        const float m_padding = 16.F;
        const float m_thumbnaleSize = 128.F;
        const std::unordered_map<ResourceIconType, std::string> iconPaths = {
                { ResourceIconType::File, "FileIcon.png"},
                { ResourceIconType::Directory, "DirectoryIcon.png"},
                { ResourceIconType::Scene, "studio.png"},
                { ResourceIconType::Image, "image.png"},
                { ResourceIconType::Script, "script.png"},
                { ResourceIconType::Font, "text.png"},
                { ResourceIconType::Prefab, "content.png"},
                { ResourceIconType::Animation, "animation.png"}
        };
    };


    class AssetsPanel: public IPanel, public Observer {
    public:
        enum class State {
            Loading,
            Active
        };
    public:
        AssetsPanel(robot2D::MessageBus& messageBus,
                    UIInteractor::Ptr uiInteractor,
                    IUIManager& iuiManager,
                    PrefabManager& prefabManager);
        AssetsPanel(const AssetsPanel& other) = delete;
        AssetsPanel& operator=(const AssetsPanel& other) = delete;
        AssetsPanel(AssetsPanel&& other) = delete;
        AssetsPanel& operator=(AssetsPanel&& other) = delete;
        ~AssetsPanel() override = default;

        void switchState(State state) { m_state = state; }
        void setAssetsPath(const std::string& path);
        void render() override;
    private:
        void dropFiles(const std::vector<std::string>& paths);
        void uiAssetsCreation();
        void processAssets();
        /// \brief in panel drag and out of panel
        void processDragDrop(const std::filesystem::directory_entry& directoryEntry,
                             std::filesystem::path& path);
    private:
        enum class AssetType {
            Scene,
            Folder
        };
        struct AssetItem {
            AssetType assetType;
            std::string name;
            std::filesystem::path absolutePath;
            std::filesystem::path relativePath;
        };
    private:
        robot2D::MessageBus& m_messageBus;
        UIInteractor::Ptr m_interactor;
        IUIManager& m_uiManager;
        PrefabManager& m_prefabManager;

        std::filesystem::path m_currentPath;
        std::filesystem::path m_assetsPath;
        robot2D::ResourceHandler<robot2D::Texture,
                AssetsPanelConfiguration::ResourceIconType> m_assetsIcons;

        AssetsPanelConfiguration m_configuration;
        std::vector<AssetItem> m_assetItems;
        std::pair<std::filesystem::path, bool> m_itemEditName;


        State m_state = State::Loading;
        bool m_itemClicked{false};
        bool m_visible{false};
    };
}