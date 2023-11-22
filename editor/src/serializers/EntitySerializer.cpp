#include <yaml-cpp/yaml.h>
#include <robot2D/Ecs/EntityManager.hpp>

#include <editor/serializers/EntitySerializer.hpp>
#include <editor/Components.hpp>
#include <editor/scripting/ScriptingEngine.hpp>



namespace YAML {
    template<>
    struct convert<editor::UUID>
    {
        static Node encode(const editor::UUID& uuid)
        {
            Node node;
            node.push_back((uint64_t)uuid);
            return node;
        }

        static bool decode(const Node& node, editor::UUID& uuid)
        {
            uuid = node.as<uint64_t>();
            return true;
        }
    };

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

        return {};
    }

    static Physics2DComponent::BodyType RigidBody2DBodyTypeFromString(const std::string& bodyTypeString)
    {
        if (bodyTypeString == "Static")    return Physics2DComponent::BodyType::Static;
        if (bodyTypeString == "Dynamic")   return Physics2DComponent::BodyType::Dynamic;
        if (bodyTypeString == "Kinematic") return Physics2DComponent::BodyType::Kinematic;

        return Physics2DComponent::BodyType::Static;
    }

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
        out << YAML::Key << "Entity" << YAML::Value << entity.getComponent<IDComponent>().ID;

        bool needSerializeChildren = false;

        if(entity.hasComponent<TagComponent>()) {
            out << YAML::Key << "TagComponent";
            out << YAML::BeginMap;
            auto& tag = entity.getComponent<TagComponent>().getTag();
            out << YAML::Key << "Tag" << YAML::Value << tag;
            out << YAML::EndMap;
        }

        if(entity.hasComponent<CameraComponent>()) {
            out << YAML::Key << "CameraComponent";
            out << YAML::BeginMap;
            auto isPrimary = entity.getComponent<CameraComponent>().isPrimary;
            out << YAML::Key << "isPrimary" << YAML::Value << isPrimary;
            out << YAML::Key << "OrthoSize" << YAML::Value << entity.getComponent<CameraComponent>().orthoSize;
            out << YAML::Key << "Size" << YAML::Value << entity.getComponent<CameraComponent>().size;
            out << YAML::Key << "Position" << YAML::Value << entity.getComponent<CameraComponent>().position;
            out << YAML::EndMap;
        }

        if(entity.hasComponent<TransformComponent>()) {
            out << YAML::Key << "TransformComponent";
            out << YAML::BeginMap;
            auto& ts = entity.getComponent<TransformComponent>();
            out << YAML::Key << "Position" << YAML::Value << ts.getPosition();
            out << YAML::Key << "Size" << YAML::Value << ts.getScale();
            out << YAML::Key << "Rotation" << YAML::Value << ts.getRotate();

            if(ts.hasChildren()) {
                out << YAML::Key << "HasChildren" << YAML::Value << true;
                std::vector<UUID> childIds;
                for(auto& child: ts.getChildren()) {
                    childIds.emplace_back(child.getComponent<IDComponent>().ID);
                }
                out << YAML::Key << "ChildIDs" << YAML::Value << childIds;
                needSerializeChildren = true;
            }

            if(ts.isChild()) {
                out << YAML::Key << "isChild" << YAML::Value << true;
            }

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

        if(entity.hasComponent<DrawableComponent>()) {
            out << YAML::Key << "SpriteComponent";
            out << YAML::BeginMap;
            auto& sp = entity.getComponent<DrawableComponent>();
            out << YAML::Key << "Color" << YAML::Value << sp.getColor();
            out << YAML::Key << "TexturePath" << YAML::Value << sp.getTexturePath();
            out << YAML::Key << "zDepth" << YAML::Value << sp.getDepth();
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

        if (entity.hasComponent<TextComponent>())
        {
            out << YAML::Key << "TextComponent";
            out << YAML::BeginMap; // TextComponent

            auto& text = entity.getComponent<TextComponent>();
            out << YAML::Key << "Text" << YAML::Value << text.getText();
            if(text.getFont())
                out << YAML::Key << "FontPath" << YAML::Value << text.getFont() -> getPath();
            out << YAML::EndMap; // TextComponent
        }

        if(entity.hasComponent<PrefabComponent>()) {
            out << YAML::Key << "PrefabComponent";
            out << YAML::BeginMap;
            auto& uuid = entity.getComponent<PrefabComponent>().prefabUUID;
            out << YAML::Key << "UUID" << YAML::Value << uuid;
            out << YAML::EndMap;
        }

        out << YAML::EndMap;

        if(needSerializeChildren) {
            auto& ts = entity.getComponent<TransformComponent>();
            for(auto& child: ts.getChildren())
                SerializeEntity(out, child);
        }

    }

    bool EntitySerializer::serialize(YAML::Emitter& out, robot2D::ecs::Entity &entity) {
        SerializeEntity(out, entity);
        return true;
    }

    bool EntitySerializer::deserialize(void* inputData, robot2D::ecs::Entity& deserializedEntity,
                                       bool& addToScene, std::vector<ChildPair>& children) {
        if(!inputData)
            return false;

        auto entity = *static_cast<YAML::detail::iterator_value*>(inputData);

        uint64_t uuid = entity["Entity"].as<uint64_t>();
        std::string name;

        auto tagComponent = entity["TagComponent"];
        if(tagComponent)
            name = tagComponent["Tag"].as<std::string>();

        deserializedEntity.addComponent<IDComponent>().ID = uuid;
        auto& tagData = deserializedEntity.addComponent<TagComponent>();
        tagData.setTag(name);

        auto cameraComponent = entity["CameraComponent"];
        if(cameraComponent) {
            auto& isPrimary = deserializedEntity.addComponent<CameraComponent>().isPrimary;
            isPrimary = cameraComponent["isPrimary"].as<bool>();
            deserializedEntity.getComponent<CameraComponent>().orthoSize = cameraComponent["OrthoSize"].as<float>();
            deserializedEntity.getComponent<CameraComponent>().size = cameraComponent["Size"].as<robot2D::vec2f>();
            deserializedEntity.getComponent<CameraComponent>().position = cameraComponent["Position"].as<robot2D::vec2f>();
        }

        auto transformComponent = entity["TransformComponent"];
        if(transformComponent) {
            auto& transform = deserializedEntity.addComponent<TransformComponent>();
            transform.setPosition(transformComponent["Position"].as<robot2D::vec2f>());
            transform.setScale(transformComponent["Size"].as<robot2D::vec2f>());
            transform.setRotate(transformComponent["Rotation"].as<float>());
            if(transformComponent["HasChildren"]) {
                auto childIDS = transformComponent["ChildIDs"].as<std::vector<UUID>>();
                ChildPair pair = std::make_pair(UUID(uuid), childIDS);
                children.emplace_back(pair);
            }
        }

        auto spriteComponent = entity["SpriteComponent"];
        if(spriteComponent) {
            auto& drawable = deserializedEntity.addComponent<DrawableComponent>();
            drawable.setColor(spriteComponent["Color"].as<robot2D::Color>());
            if(spriteComponent["TexturePath"])
                drawable.setTexturePath(spriteComponent["TexturePath"].as<std::string>());
            if(spriteComponent["zDepth"])
                drawable.setDepth(spriteComponent["zDepth"].as<int>());
            drawable.setReorderZBuffer(true);
        }

        auto textComponent = entity["TextComponent"];
        if(textComponent) {
            auto& text = deserializedEntity.addComponent<TextComponent>();
            text.setText(textComponent["Text"].as<std::string>());
            if(textComponent["FontPath"])
                text.setFontPath(textComponent["FontPath"].as<std::string>());
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

        auto prefabComponent = entity["PrefabComponent"];
        if(prefabComponent) {
            deserializedEntity.addComponent<PrefabComponent>().prefabUUID = prefabComponent["UUID"].as<uint64_t>();
        }

        return true;
    }


} // namespace editor