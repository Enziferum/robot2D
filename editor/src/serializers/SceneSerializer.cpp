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

    ISceneDeserializer::~ISceneDeserializer() {}


    class YamlSceneDeserializer: public ISceneDeserializer {
    public:
        ~YamlSceneDeserializer() override = default;
        bool deserialize(const std::string& path) override;
    };

    bool YamlSceneDeserializer::deserialize(const std::string& path) {
        return true;
    }



    SceneSerializer::SceneSerializer(Scene::Ptr scene): m_scene(scene) {}



    bool SceneSerializer::serialize(const std::string& path) {
        if(m_scene == nullptr)
            return false;

        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Scene" << YAML::Value << "Unnamed Scene";
        out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
        EntitySerializer entitySerializer;
        for(auto& entity: m_scene -> getEntities())
            entitySerializer.serialize(out, entity);
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

    bool SceneSerializer::deserialize(const std::string& path) {
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
        catch (...) {
            RB_EDITOR_CRITICAL("YAML Exception");
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


        EntitySerializer entitySerializer;
        std::vector<ChildInfo> children;

        for(auto entity: entites) {
            bool addToScene = true;

            auto deserializedEntity = m_scene -> createEntity();
            entitySerializer.deserialize(&entity, deserializedEntity, addToScene, children);

            if(addToScene)
                m_scene -> addAssociatedEntity(deserializedEntity);
        }

        /// TODO(a.raag): Save and Read Scene as Tree
        for(auto& child: children) {
            if(child.isChild) {
                auto selfUUID = child.self.getComponent<IDComponent>().ID;
                if(child.hasChildren()) {
                    for(auto& item: children) {
                        if(item.isChild && item.parentUUID == selfUUID)
                            child.self.getComponent<TransformComponent>().addChild(child.self, item.self);
                    }
                }

                auto parent = child.self.getComponent<TransformComponent>().getParent();
                if(!parent) {
                    auto found = m_scene -> getByUUID(child.parentUUID);
                    if(found && !found.destroyed())
                        found.getComponent<TransformComponent>().addChild(found, child.self);
                    else {
                        for(auto& item: children) {
                            if(item.isChild && item.self.getComponent<IDComponent>().ID == child.parentUUID)
                                item.self.getComponent<TransformComponent>().addChild(item.self, child.self);
                        }
                    }
                }
            }
            else {
                if(child.hasChildren()) {
                    auto& parentUUID = child.childPair.first;
                    auto self = m_scene -> getByUUID(parentUUID);

                    for(auto& item: children) {
                        if(item.isChild && item.parentUUID == parentUUID)
                            self.getComponent<TransformComponent>().addChild(self, item.self);
                    }

                }
            }

        }


        return true;
    }

    SceneSerializerError SceneSerializer::getError() const {
        return SceneSerializerError::None;
    }

}