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

#include <fstream>
#include <yaml-cpp/yaml.h>

#include <editor/SceneSerializer.hpp>
#include <editor/Scene.hpp>
#include <editor/Components.hpp>

namespace YAML {
    template<>
    struct convert<robot2D::vec2f> {
        static Node encode(const robot2D::vec2f& rhs) {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            return node;
        }

        static bool decode(const Node& node, robot2D::vec2f& rhs) {
            if(!node.IsSequence() || node.size() != 2)
                return false;
            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            return true;
        }
    };

    template<>
    struct convert<robot2D::Color> {
        static Node encode(const robot2D::Color& rhs) {
            Node node;
            node.push_back(rhs.red);
            node.push_back(rhs.green);
            node.push_back(rhs.blue);
            node.push_back(rhs.alpha);
            return node;
        }

        static bool decode(const Node& node, robot2D::Color& rhs) {
            if(!node.IsSequence() || node.size() != 4)
                return false;
            rhs.red = node[0].as<float>();
            rhs.green = node[1].as<float>();
            rhs.blue = node[2].as<float>();
            rhs.alpha = node[3].as<float>();
            return true;
        }
    };
}

namespace editor {
    enum class SerializeID {
        Scene,
        Entities,
        Tag,
        Transform,
        Sprite
    };

    std::unordered_map<SerializeID, std::string> serializeKeys = {
            {SerializeID::Scene, "Scene"},
            {SerializeID::Entities, "Entities"},
            {SerializeID::Tag, "TagComponent"},
            {SerializeID::Transform, "TransformComponent"},
            {SerializeID::Sprite, "SpriteComponent"},
            {SerializeID::Tag, "TransformComponent"},
    };

    SceneSerializer::SceneSerializer(Scene::Ptr scene): m_scene(scene) {}

    YAML::Emitter& operator<<(YAML::Emitter& out, const robot2D::vec2f& value) {
        out << YAML::Flow;
        out << YAML::BeginSeq << value.x << value.y << YAML::EndSeq;
        return out;
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const robot2D::Color& value) {
        out << YAML::Flow;
        out << YAML::BeginSeq << value.red << value.green
        << value.blue << value.alpha << YAML::EndSeq;
        return out;
    }

    void SerializeEntity(YAML::Emitter& out, robot2D::ecs::Entity entity) {
        out << YAML::BeginMap;
        out << YAML::Key << "Entity" << YAML::Value << "9012510123";
        if(entity.hasComponent<TagComponent>()) {
            out << YAML::Key << "TagComponent";
            out << YAML::BeginMap;
            auto& tag = entity.getComponent<TagComponent>().getTag();
            out << YAML::Key << "Tag" << YAML::Value << tag;
            out << YAML::EndMap;
        }

        if(entity.hasComponent<TransformComponent>()) {
            out << YAML::Key << "TransformComponent";
            out << YAML::BeginMap;
            auto& ts = entity.getComponent<TransformComponent>();
            out << YAML::Key << "Position" << YAML::Value << ts.getPosition();
            out << YAML::Key << "Size" << YAML::Value << ts.getScale();
            out << YAML::Key << "Rotation" << YAML::Value << ts.getRotate();
            out << YAML::EndMap;
        }

        if(entity.hasComponent<SpriteComponent>()) {
            out << YAML::Key << "SpriteComponent";
            out << YAML::BeginMap;
            auto& sp = entity.getComponent<SpriteComponent>();
            out << YAML::Key << "Color" << YAML::Value << sp.getColor();
            out << YAML::EndMap;
        }

        out << YAML::EndMap;
    }

    bool SceneSerializer::serialize(const std::string& path) {
        if(m_scene == nullptr)
            return false;

        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Scene" << YAML::Value << "Unnamed Scene";
        out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
        for(auto& it: m_scene->getEntities()) {
            SerializeEntity(out, it);
        }
        out << YAML::EndSeq;
        out << YAML::EndMap;

        std::ofstream ofstream(path);
        if(!ofstream.is_open())
            return false;

        ofstream << out.c_str();
        ofstream.close();

        return true;
    }

    bool SceneSerializer::deserialize(const std::string& path) {
        std::ifstream ifstream(path);
        if(!ifstream.is_open())
            return false;
        std::stringstream sstr;
        sstr << ifstream.rdbuf();

        YAML::Node data = YAML::Load(sstr.str());
        if(!data["Scene"])
            return false;

        std::string sceneName = data["Scene"].as<std::string>();
        auto entites = data["Entities"];
        if(entites) {
            for(auto entity: entites) {
                uint64_t uuid = entity["Entity"].as<uint64_t>();
                std::string name;
                auto tagComponent = entity["TagComponent"];
                if(tagComponent)
                    name = tagComponent["Tag"].as<std::string>();

                auto deserializedEntity = m_scene->createEntity();
                auto& tagData = deserializedEntity.addComponent<TagComponent>();
                tagData.setTag(name);

                auto transformComponent = entity["TransformComponent"];
                if(transformComponent) {
                    auto& transform = deserializedEntity.addComponent<TransformComponent>();
                    transform.setPosition(transformComponent["Position"].as<robot2D::vec2f>());
                    transform.setScale(transformComponent["Size"].as<robot2D::vec2f>());
                    transform.setRotate(transformComponent["Rotation"].as<float>());
                }

                auto spriteComponent = entity["SpriteComponent"];
                if(spriteComponent) {
                    auto& sp = deserializedEntity.addComponent<SpriteComponent>();
                    sp.setColor(spriteComponent["Color"].as<robot2D::Color>());
                }
            }
        }
        return true;
    }
}