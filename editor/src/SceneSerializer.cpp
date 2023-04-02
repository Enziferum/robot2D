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
#include <editor/scripting/ScriptingEngine.hpp>

#include <robot2D/Util/Logger.hpp>

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
    struct convert<robot2D::vec3f> {
        static Node encode(const robot2D::vec3f& rhs) {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            return node;
        }

        static bool decode(const Node& node, robot2D::vec3f& rhs) {
            if(!node.IsSequence() || node.size() != 3)
                return false;
            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
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

#define WRITE_SCRIPT_FIELD(FieldType, Type)           \
			case ScriptFieldType::FieldType:          \
				out << scriptField.getValue<Type>();  \
				break

#define READ_SCRIPT_FIELD(FieldType, Type)             \
	case ScriptFieldType::FieldType:                   \
	{                                                  \
		Type data = scriptField["Data"].as<Type>();    \
		fieldInstance.setValue(data);                  \
		break;                                         \
	}


    static std::string RigidBody2DBodyTypeToString(Physics2DComponent::BodyType bodyType)
    {
        switch (bodyType)
        {
            case Physics2DComponent::BodyType::Static:    return "Static";
            case Physics2DComponent::BodyType::Dynamic:   return "Dynamic";
            case Physics2DComponent::BodyType::Kinematic: return "Kinematic";
        }

        //HZ_CORE_ASSERT(false, "Unknown body type");
        return {};
    }

    static Physics2DComponent::BodyType RigidBody2DBodyTypeFromString(const std::string& bodyTypeString)
    {
        if (bodyTypeString == "Static")    return Physics2DComponent::BodyType::Static;
        if (bodyTypeString == "Dynamic")   return Physics2DComponent::BodyType::Dynamic;
        if (bodyTypeString == "Kinematic") return Physics2DComponent::BodyType::Kinematic;

       // HZ_CORE_ASSERT(false, "Unknown body type");
        return Physics2DComponent::BodyType::Static;
    }

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




    namespace {
        // tmp value before UUID Manager
        const std::string fakeUUID = "9012510123";
    }

    SceneSerializer::SceneSerializer(Scene::Ptr scene): m_scene(scene) {}

    YAML::Emitter& operator<<(YAML::Emitter& out, const robot2D::vec2f& value) {
        out << YAML::Flow;
        out << YAML::BeginSeq << value.x << value.y << YAML::EndSeq;
        return out;
    }

    YAML::Emitter& operator<<(YAML::Emitter& out, const robot2D::vec3f& value) {
        out << YAML::Flow;
        out << YAML::BeginSeq << value.x << value.y << value.z << YAML::EndSeq;
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
        out << YAML::Key << "Entity" << YAML::Value << fakeUUID;

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
            // TODO: @a.raag add rotation
            out << YAML::Key << "Rotation" << YAML::Value << 0.F;
            out << YAML::EndMap;
        }

        if(entity.hasComponent<Transform3DComponent>()) {
            out << YAML::Key << "TransformComponent";
            out << YAML::BeginMap;
            auto& ts = entity.getComponent<Transform3DComponent>();
            out << YAML::Key << "Position" << YAML::Value << ts.getPosition();
            out << YAML::Key << "Size" << YAML::Value << ts.getScale();
            // TODO: @a.raag add rotation
            out << YAML::Key << "Rotation" << YAML::Value << 0.F;
            out << YAML::EndMap;
        }

        if(entity.hasComponent<SpriteComponent>()) {
            out << YAML::Key << "SpriteComponent";
            out << YAML::BeginMap;
            auto& sp = entity.getComponent<SpriteComponent>();
            out << YAML::Key << "Color" << YAML::Value << sp.getColor();
            out << YAML::EndMap;
        }

        if(entity.hasComponent<ScriptComponent>()) {
            auto& scriptComponent = entity.getComponent<ScriptComponent>();

            out << YAML::Key << "ScriptComponent";
            out << YAML::BeginMap;
            out << YAML::Key << "ClassName" << YAML::Value << scriptComponent.name;

            auto entityClass = ScriptEngine::getEntityClass(scriptComponent.name);
            const auto& fields = entityClass -> getFields();

            if(fields.size() > 0) {
                out << YAML::Key << "ScriptFields" << YAML::Value;
                auto& entityFields = ScriptEngine::getScriptFieldMap(entity);
                out << YAML::BeginSeq;
                for (const auto& [name, field] : fields)
                {
                    if (entityFields.find(name) == entityFields.end())
                        continue;

                    out << YAML::BeginMap; // ScriptField
                    out << YAML::Key << "Name" << YAML::Value << name;
                    out << YAML::Key << "Type" << YAML::Value << util::ScriptFieldTypeToString(field.Type);
                    out << YAML::Key << "Data" << YAML::Value;
                    ScriptFieldInstance& scriptField = entityFields.at(name);

                    switch (field.Type)
                    {
                        WRITE_SCRIPT_FIELD(Float,   float     );
                        WRITE_SCRIPT_FIELD(Double,  double    );
                        WRITE_SCRIPT_FIELD(Bool,    bool      );
                        WRITE_SCRIPT_FIELD(Char,    char      );
                        WRITE_SCRIPT_FIELD(Byte,    int8_t    );
                        WRITE_SCRIPT_FIELD(Short,   int16_t   );
                        WRITE_SCRIPT_FIELD(Int,     int32_t   );
                        WRITE_SCRIPT_FIELD(Long,    int64_t   );
                        WRITE_SCRIPT_FIELD(UByte,   uint8_t   );
                        WRITE_SCRIPT_FIELD(UShort,  uint16_t  );
                        WRITE_SCRIPT_FIELD(UInt,    uint32_t  );
                        WRITE_SCRIPT_FIELD(ULong,   uint64_t  );
                        WRITE_SCRIPT_FIELD(Vector2, robot2D::vec2f );
                        //WRITE_SCRIPT_FIELD(Entity,  UUID      );
                    }
                    out << YAML::EndMap; // ScriptFields
                }
                out << YAML::EndSeq;
            }
            out << YAML::EndMap; // ScriptComponent
        }

        if (entity.hasComponent<Physics2DComponent>())
        {
            out << YAML::Key << "Rigidbody2DComponent";
            out << YAML::BeginMap; // Rigidbody2DComponent

            auto& rb2dComponent = entity.getComponent<Physics2DComponent>();
            out << YAML::Key << "BodyType" << YAML::Value << RigidBody2DBodyTypeToString(rb2dComponent.type);
            out << YAML::Key << "FixedRotation" << YAML::Value << rb2dComponent.fixedRotation;

            out << YAML::EndMap; // Rigidbody2DComponent
        }

        if (entity.hasComponent<Collider2DComponent>())
        {
            out << YAML::Key << "BoxCollider2DComponent";
            out << YAML::BeginMap; // BoxCollider2DComponent

            auto& bc2dComponent = entity.getComponent<Collider2DComponent>();
            out << YAML::Key << "Offset" << YAML::Value << bc2dComponent.offset;
            out << YAML::Key << "Size" << YAML::Value << bc2dComponent.size;
            out << YAML::Key << "Density" << YAML::Value << bc2dComponent.density;
            out << YAML::Key << "Friction" << YAML::Value << bc2dComponent.friction;
            out << YAML::Key << "Restitution" << YAML::Value << bc2dComponent.restitution;
            out << YAML::Key << "RestitutionThreshold" << YAML::Value << bc2dComponent.restitutionThreshold;

            out << YAML::EndMap; // BoxCollider2DComponent
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
        for(auto& it: m_scene -> getEntities()) {
            SerializeEntity(out, it);
        }
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

        if(entites) {
            for(auto entity: entites) {

                uint64_t uuid = entity["Entity"].as<uint64_t>();
                std::string name;

                auto tagComponent = entity["TagComponent"];
                if(tagComponent)
                    name = tagComponent["Tag"].as<std::string>();

                auto deserializedEntity = m_scene -> createEntity();
                auto& tagData = deserializedEntity.addComponent<TagComponent>();
                tagData.setTag(name);

                auto transformComponent = entity["TransformComponent"];
                if(transformComponent) {
                    auto& transform = deserializedEntity.addComponent<TransformComponent>();
                    transform.setPosition(transformComponent["Position"].as<robot2D::vec2f>());
                    transform.setScale(transformComponent["Size"].as<robot2D::vec2f>());
                    // TODO: @a.raag add rotation
                    //transform.setRotate(transformComponent["Rotation"].as<float>());
                }

                auto spriteComponent = entity["SpriteComponent"];
                if(spriteComponent) {
                    auto& sp = deserializedEntity.addComponent<SpriteComponent>();
                    sp.setColor(spriteComponent["Color"].as<robot2D::Color>());
                }

                auto scriptComponent = entity["ScriptComponent"];
                if(scriptComponent) {
                    auto& sc = deserializedEntity.addComponent<ScriptComponent>();
                    sc.name = scriptComponent["ClassName"].as<std::string>();

                    auto scriptFields = scriptComponent["ScriptFields"];
                    if (scriptFields)
                    {
                        auto entityClass = ScriptEngine::getEntityClass(sc.name);
                        if (entityClass)
                        {
                            const auto& fields = entityClass -> getFields();
                            auto& entityFields = ScriptEngine::getScriptFieldMap(deserializedEntity);

                            for (auto scriptField : scriptFields)
                            {
                                std::string name = scriptField["Name"].as<std::string>();
                                std::string typeString = scriptField["Type"].as<std::string>();
                                ScriptFieldType type = util::ScriptFieldTypeFromString(typeString);

                                ScriptFieldInstance& fieldInstance = entityFields[name];

                                // TODO(Yan): turn this assert into Hazelnut log warning
                                // HZ_CORE_ASSERT(fields.find(name) != fields.end());

                                if (fields.find(name) == fields.end())
                                    continue;

                                fieldInstance.Field = fields.at(name);

                                switch (type)
                                {
                                    READ_SCRIPT_FIELD(Float, float);
                                    READ_SCRIPT_FIELD(Double, double);
                                    READ_SCRIPT_FIELD(Bool, bool);
                                    READ_SCRIPT_FIELD(Char, char);
                                    READ_SCRIPT_FIELD(Byte, int8_t);
                                    READ_SCRIPT_FIELD(Short, int16_t);
                                    READ_SCRIPT_FIELD(Int, int32_t);
                                    READ_SCRIPT_FIELD(Long, int64_t);
                                    READ_SCRIPT_FIELD(UByte, uint8_t);
                                    READ_SCRIPT_FIELD(UShort, uint16_t);
                                    READ_SCRIPT_FIELD(UInt, uint32_t);
                                    READ_SCRIPT_FIELD(ULong, uint64_t);
                                    READ_SCRIPT_FIELD(Vector2, robot2D::vec2f);
                                    // READ_SCRIPT_FIELD(Vector3, glm::vec3);
                                    // READ_SCRIPT_FIELD(Vector4, glm::vec4);
                                    // READ_SCRIPT_FIELD(Entity, UUID);
                                }

                            }
                        }
                    }
                }

                auto rigidbody2DComponent = entity["Rigidbody2DComponent"];
                if (rigidbody2DComponent)
                {
                    auto& rb2d = deserializedEntity.addComponent<Physics2DComponent>();
                    rb2d.type = RigidBody2DBodyTypeFromString(rigidbody2DComponent["BodyType"].as<std::string>());
                    rb2d.fixedRotation = rigidbody2DComponent["FixedRotation"].as<bool>();
                }

                auto boxCollider2DComponent = entity["BoxCollider2DComponent"];
                if (boxCollider2DComponent)
                {
                    auto& bc2d = deserializedEntity.addComponent<Collider2DComponent>();
                    bc2d.offset = boxCollider2DComponent["Offset"].as<robot2D::vec2f>();
                    bc2d.size = boxCollider2DComponent["Size"].as<robot2D::vec2f>();
                    bc2d.density = boxCollider2DComponent["Density"].as<float>();
                    bc2d.friction = boxCollider2DComponent["Friction"].as<float>();
                    bc2d.restitution = boxCollider2DComponent["Restitution"].as<float>();
                    bc2d.restitutionThreshold = boxCollider2DComponent["RestitutionThreshold"].as<float>();
                }
            }
        }
        return true;
    }

    SceneSerializerError SceneSerializer::getError() const {
        return SceneSerializerError::None;
    }

}