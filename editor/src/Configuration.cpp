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

#include <editor/Configuration.hpp>
#include <robot2D/Util/Logger.hpp>

namespace editor {

    namespace {
        const std::string editorVersion = "0.1";
        const std::string projectExtension = ".robot2D";
        const std::string defaultName = "Scene.scene";
        const std::string cachePath = "editor.cache";
    }

    Configuration::Configuration(): m_properties() {
        setup();
    }

    std::tuple<bool, std::string> Configuration::getValue(const ConfigurationKey& key) {
        auto found = m_properties.find(key);
        if(found == m_properties.end())
            return {false, ""};
        else {
            return {true, m_properties[key]};
        }
    }

    void Configuration::setup() {
        if(!m_properties.insert(std::pair<ConfigurationKey, std::string>(ConfigurationKey::Version,
                                                                         editorVersion)).second){
            RB_EDITOR_ERROR("Configuration setup error");
            return;
        }

        if(!m_properties.insert(std::pair<ConfigurationKey, std::string>(ConfigurationKey::ProjectExtension,
                                                                         projectExtension)).second){
            RB_EDITOR_ERROR("Configuration setup error");
            return;
        }

        if(!m_properties.insert(std::pair<ConfigurationKey, std::string>(ConfigurationKey::DefaultSceneName,
                                                                         defaultName)).second){
            RB_EDITOR_ERROR("Configuration setup error");
            return;
        }

        if(!m_properties.insert(std::pair<ConfigurationKey, std::string>(ConfigurationKey::CachePath,
                                                                         cachePath)).second){
            RB_EDITOR_ERROR("Configuration setup error");
            return;
        }
    }
}