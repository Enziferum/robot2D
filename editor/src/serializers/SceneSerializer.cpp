/*********************************************************************
(c) Alex Raag 2026
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
#include <yaml-cpp/yaml.h>
#include <robot2D/Util/Logger.hpp>


#include <editor/serializers/SceneSerializer.hpp>
#include <editor/serializers/EntitySerializer.hpp>

#include <editor/Scene.hpp>
#include <editor/Components.hpp>
#include <editor/scripting/ScriptingEngine.hpp>


namespace editor {


    class ISceneDeserializer {
    public:
        virtual ~ISceneDeserializer() = 0;
        virtual bool deserialize(const std::string& path) = 0;
    };

    ISceneDeserializer::~ISceneDeserializer() = default;


    class YamlSceneDeserializer: public ISceneDeserializer {
    public:
        ~YamlSceneDeserializer() override = default;
        bool deserialize(const std::string& path) override;
    };

    bool YamlSceneDeserializer::deserialize(const std::string& path) {
        return true;
    }



    SceneSerializer::SceneSerializer(Scene::Ptr scene): m_scene(scene) {}


    bool SceneSerializer::serialize(const std::string& path, const std::string& sceneName,
                                    IScriptInteractorFrom::Ptr scriptingEngine) {
        if(m_scene == nullptr)
            return false;



        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Scene" << YAML::Value << sceneName;
        out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

        auto entitySerializer = getSerializer<EntityYAMLSerializer>();
        for(auto& entity: m_scene -> getEntities())
            entitySerializer -> serialize(out, entity, scriptingEngine);
        out << YAML::EndSeq;
        out << YAML::EndMap;

        std::ofstream ofstream(path);
        if(!ofstream.is_open()) {
            m_error = SceneSerializerError::NoFileOpen;
            return false;
        }

        ofstream << out.c_str();
        ofstream.close();

        return true;
    }
    /// 1
    //// 2 15 3
    ////  6  4 9

    struct SceneSerializeData {
        enum class ResourceType {
            Texture,
            Font
        };

        std::vector<ResourceType> resourceTypes;
    };

    bool SceneSerializer::deserialize(const std::string& path, IScriptInteractorFrom::Ptr scriptingEngine) {
        YAML::Node data;
        try {
            std::ifstream ifstream(path);
            if(!ifstream.is_open()) {
                m_error = SceneSerializerError::NoFileOpen;
                return false;
            }

            std::stringstream sstr;
            sstr << ifstream.rdbuf();
            ifstream.close();
            data = YAML::Load(sstr.str());
        }
        catch (const std::exception& e) {
            RB_EDITOR_CRITICAL("[SceneSerializer]: Deserialize YAML Exception {0}", e.what());
            exit(2);
        }

        if(!data["Scene"]) {
            m_error = SceneSerializerError::NotSceneTag;
            return false;
        }

        std::string sceneName = data["Scene"].as<std::string>();
        auto entites = data["Entities"];
        if(!entites)
            return false;


        auto entitySerializer = getSerializer<EntityYAMLSerializer>();
        std::vector<ChildInfo> children;

        for(const auto& entity: entites) {
            bool addToScene = true;
            auto deserializedEntity = m_scene -> createEmptyEntity();
            if(!entitySerializer -> deserialize(entity, deserializedEntity, addToScene,
                                                 children, scriptingEngine))
                return false;
            if(m_scene -> getEntity(deserializedEntity.getUUID()))
                return false;
            if(!m_scene -> attachEntity(deserializedEntity))
                return false;
        }

        for(auto& info: children) {
            if(!info.hasChildren())
                continue;
            auto parent = m_scene -> getEntity(info.childPair.first);
            if(!parent)
                return false;
            for(auto uuid: info.childPair.second) {
                auto child = m_scene -> getEntity(uuid);
                if(!child || !m_scene -> reparent(child, parent))
                    return false;
            }
        }
        for(auto& info: children) {
            if(!info.isChild)
                continue;
            auto parent = m_scene -> getEntity(info.parentUUID);
            auto currentParent = info.self.getParent();
            if(currentParent && currentParent != parent)
                return false;
            if(!parent || !m_scene -> reparent(info.self, parent))
                return false;
        }

        return true;
    }

    SceneSerializerError SceneSerializer::getError() const {
        return SceneSerializerError::None;
    }

}