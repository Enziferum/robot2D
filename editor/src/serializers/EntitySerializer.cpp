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

#include <yaml-cpp/yaml.h>
#include <robot2D/Ecs/EntityManager.hpp>

#include <editor/serializers/EntitySerializer.hpp>
#include <editor/scripting/ScriptingEngine.hpp>

#include <editor/Components.hpp>
#include <editor/components/ButtonComponent.hpp>
#include <editor/components/UIHitBox.hpp>

#include <editor/AnimationManager.hpp>
#include <editor/ResouceManager.hpp>
#include <editor/LocalResourceManager.hpp>
#include <editor/FileApi.hpp>

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
				out << field.getValue<Type>();  \
				break

#define READ_SCRIPT_FIELD(FieldType, Type)             \
	case ScriptFieldType::FieldType:                   \
	{                                                  \
		Type data = scriptField["Data"].as<Type>();    \
		field.setValue(data);                  \
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


    void SerializeEntity(YAML::Emitter& out, SceneEntity entity, IScriptInteractorFrom::Ptr scriptInteractor) {
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
            out << YAML::Key << "Size" << YAML::Value << ts.getSize();
            out << YAML::Key << "Rotation" << YAML::Value << ts.getRotate();
            out << YAML::Key << "Origin" << YAML::Value << ts.getOrigin();

            if(entity.hasChildren()) {
                std::vector<UUID> childIds;

                for(auto& child: ts.getChildren()) {
                    if(child)
                        childIds.emplace_back(child.getUUID());
                }

                if(!childIds.empty()) {
                    out << YAML::Key << "HasChildren" << YAML::Value << true;
                    out << YAML::Key << "ChildIDs" << YAML::Value << childIds;
                    needSerializeChildren = true;
                }
            }

            if(ts.isChild()) {
                out << YAML::Key << "isChild" << YAML::Value << true;
                out << YAML::Key << "ParentID" << YAML::Value << ts.getParent().getComponent<IDComponent>().ID;
            }

            out << YAML::EndMap;
        }

        if(entity.hasComponent<Transform3DComponent>()) {
            out << YAML::Key << "TransformComponent";
            out << YAML::BeginMap;
            auto& ts = entity.getComponent<Transform3DComponent>();
            out << YAML::Key << "Position" << YAML::Value << ts.getPosition();
            //out << YAML::Key << "Size" << YAML::Value << ts.getSize();
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

            const auto& fields = scriptInteractor -> getFields(entity.getUUID());
            if(!fields.empty()) {
                out << YAML::Key << "ScriptFields" << YAML::Value;
                out << YAML::BeginSeq;
                for (const auto& [name, field] : fields)
                {
                    auto scriptType = util::convert2Script(field.getType());
                    out << YAML::BeginMap; // ScriptField
                    out << YAML::Key << "Name" << YAML::Value << name;
                    out << YAML::Key << "Type" << YAML::Value << util::ScriptFieldTypeToString(scriptType);
                    out << YAML::Key << "Data" << YAML::Value;

                    switch (scriptType)
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
                        WRITE_SCRIPT_FIELD(Transform, UUID );
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

        if(entity.hasComponent<AnimationComponent>()) {
            auto* animationManager = LocalResourceManager::getManager();
            auto animations= animationManager -> getAnimations(entity.getComponent<IDComponent>().ID);

            out << YAML::Key << "AnimationComponent";
            out << YAML::BeginMap;
            std::vector<std::string> animationPaths;
            animationPaths.reserve(animations.size());
            AnimationParser animationParser;

            for(auto& animation: animations) {
                animationPaths.emplace_back(cutPath(animation.filePath, "assets"));
                animationParser.saveToFile(animation.filePath, &animation);
            }

            out << YAML::Key << "Animations" << animationPaths;
            out << YAML::EndMap;
        }
        if(entity.hasComponent<ButtonComponent>()) {
            auto& btnComp = entity.getComponent<ButtonComponent>();
            if(btnComp.hasEntity()) {
                out << YAML::Key << "ButtonComponent";
                out << YAML::BeginMap;
                out << YAML::Key << "ScriptUUID" << YAML::Value << btnComp.scriptEntity;
                if(!btnComp.clickMethodName.empty())
                    out << YAML::Key << "MethodName" << YAML::Value << btnComp.clickMethodName;
                out << YAML::EndMap;
            }
        }

        out << YAML::EndMap;

        if(needSerializeChildren) {
            for(auto child: entity.getChildren())
                SerializeEntity(out, SceneEntity(std::move(child)), scriptInteractor);
        }

    }

    IEntitySerializer::~IEntitySerializer() noexcept = default;

    bool EntityYAMLSerializer::serialize(YAML::Emitter& out, const SceneEntity& entity,
                                         IScriptInteractorFrom::Ptr scriptInteractor) {
        SerializeEntity(out, entity, scriptInteractor);
        return true;
    }

    /// TODO(a.raag) Stringify Enum
    enum class YamlNode {

    };

    bool EntityYAMLSerializer::deserialize(const YAML::detail::iterator_value& iterator,
                                           SceneEntity& deserializedEntity,
                                           bool& addToScene,
                                           std::vector<ChildInfo>& children,
                                           IScriptInteractorFrom::Ptr scriptInteractor) {

        const auto& entity = iterator;

        /// BASE
        if(!entity["Entity"] || !entity["TagComponent"]) {
            return false;
        }

        auto uuid = entity["Entity"].as<UUID>();
        deserializedEntity.addComponent<IDComponent>().ID = uuid;
        auto& tagData = deserializedEntity.addComponent<TagComponent>();
        tagData.setTag(entity["TagComponent"]["Tag"].as<std::string>());

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
            transform.setSize(transformComponent["Size"].as<robot2D::vec2f>());
            transform.setRotate(transformComponent["Rotation"].as<float>());
            if(transformComponent["Origin"])
                transform.setOrigin(transformComponent["Origin"].as<robot2D::vec2f>());
            ChildInfo childInfo;

            if(transformComponent["HasChildren"]) {
                auto childIDS = transformComponent["ChildIDs"].as<std::vector<UUID>>();
                ChildPair pair = std::make_pair(uuid, childIDS);
                childInfo.childPair = std::move(pair);
            }
            if(transformComponent["isChild"]) {
                auto parentID = transformComponent["ParentID"].as<UUID>();
                childInfo.isChild = true;
                childInfo.parentUUID = parentID;
                childInfo.self = deserializedEntity;
                addToScene = false;
            }

            if(!childInfo.isEmpty())
                children.emplace_back(childInfo);

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
            if(!scriptComponent["ClassName"]) {
                /// TODO(a.raag) skip
            }
            sc.name = scriptComponent["ClassName"].as<std::string>();

            auto scriptFields = scriptComponent["ScriptFields"];
            if (scriptFields)
            {
                auto& fields = scriptInteractor -> getFields(uuid);
                for(const auto& scriptField: scriptFields) {
                    std::string name = scriptField["Name"].as<std::string>();
                    std::string typeString = scriptField["Type"].as<std::string>();
                    ScriptFieldType type = util::ScriptFieldTypeFromString(typeString);

                    Field field;
                    field.setName(name);
                    field.setType(util::convertFromScript(type));
                    fields[name] = field;

                    if(!scriptField["Data"])
                        continue;
                    switch (type)
                    {
                        default:
                            break;
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
                        READ_SCRIPT_FIELD(Transform, UUID);
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

        auto animationComponent = entity["AnimationComponent"];
        if(animationComponent) {
            auto resourceManager = ResourceManager::getManager();
            auto animationPaths = animationComponent["Animations"].as<std::vector<std::string>>();
            resourceManager -> setAnimationPathsToLoad(uuid, std::move(animationPaths));
            deserializedEntity.addComponent<AnimationComponent>();
            deserializedEntity.addComponent<AnimatorComponent>();
        }

        auto buttonComponent = entity["ButtonComponent"];
        if(buttonComponent) {
            auto& btnComponent = deserializedEntity.addComponent<ButtonComponent>();
            deserializedEntity.addComponent<UIHitbox>();
            btnComponent.scriptEntity = buttonComponent["ScriptUUID"].as<UUID>();
            if(buttonComponent["MethodName"])
                btnComponent.clickMethodName = buttonComponent["MethodName"].as<std::string>();
        }

        return true;
    }


} // namespace editor