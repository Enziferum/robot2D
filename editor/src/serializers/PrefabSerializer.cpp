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

#include <fstream>
#include "yaml-cpp/yaml.h"
#include <robot2D/Util/Logger.hpp>

#include "editor/serializers/PrefabSerializer.hpp"
#include "editor/serializers/EntitySerializer.hpp"

#include "editor/scripting/ScriptingEngine.hpp"
#include "editor/Components.hpp"

namespace YAML {
    template<>
    struct convert<editor::UUID> {
        static Node encode(const editor::UUID &uuid) {
            Node node;
            node.push_back((uint64_t) uuid);
            return node;
        }

        static bool decode(const Node &node, editor::UUID &uuid) {
            uuid = node.as<uint64_t>();
            return true;
        }
    };
}

namespace editor {
    PrefabSerializer::PrefabSerializer() = default;

    bool PrefabSerializer::serialize(editor::Prefab::Ptr prefab, IScriptInteractorFrom::Ptr scriptInteractor) {
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Prefab" << YAML::Value << prefab -> getEntity().getComponent<TagComponent>().getTag();
        out << YAML::Key << "PrefabUUID" << YAML::Value << prefab -> getUUID();
        out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

        auto entitySerializer = getSerializer<EntityYAMLSerializer>();
        if(!entitySerializer -> serialize(out, prefab -> getEntity(), scriptInteractor)) {
            m_error = PrefabError::BadSerialization;
            return false;
        }

        out << YAML::EndSeq;
        out << YAML::EndMap;

        std::ofstream ofstream(prefab -> getPath());
        if(!ofstream.is_open()) {
            m_error = PrefabError::NoFileOpen;
            return false;
        }

        ofstream << out.c_str();
        ofstream.close();

        return true;
    }

    bool PrefabSerializer::deserialize(Prefab::Ptr prefab, IScriptInteractorFrom::Ptr scriptInteractor) {
        YAML::Node data;
        try {
            std::ifstream ifstream(prefab -> getPath());
            if(!ifstream.is_open()) {
                m_error = PrefabError::NoFileOpen;
                return false;
            }

            std::stringstream sstr;
            sstr << ifstream.rdbuf();
            ifstream.close();
            data = YAML::Load(sstr.str());
        }
        catch (...) {
            RB_EDITOR_CRITICAL("YAML Exception");
            exit(2);
        }

        if(!data["Prefab"]) {
            return false;
        }

        std::string prefabName = data["Prefab"].as<std::string>();
        uint64_t uuid = data["PrefabUUID"].as<uint64_t>();
        prefab -> setUUID(uuid);

        auto prefabEntities = data["Entities"];

        auto entitySerializer = getSerializer<EntityYAMLSerializer>();

        /// TODO(a.raag): special deserialze for prefab
        if(!prefabEntities)
            return false;

        std::vector<ChildInfo> children;
        for(const auto& entity: prefabEntities) {
            bool addToScene = true;
            auto& deserializedEntity = prefab -> getEntity();

            bool status = entitySerializer -> deserialize(entity, deserializedEntity,
                                            addToScene, children, scriptInteractor);

            if(!status) {
                m_error = PrefabError::BadDeSerialization;
                return false;
            }
        }

        return true;
    }

    PrefabError PrefabSerializer::getError() const {
        return m_error;
    }
}