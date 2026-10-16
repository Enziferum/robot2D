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
#include <unordered_set>
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
#include <editor/physics/Layers2D.hpp>

#include <editor/serializers/YamlTypes.hpp>
#include <editor/serializers/YamlFields.hpp>

namespace editor {


#define WRITE_SCRIPT_FIELD(FieldType, Type)           \
        case ScriptFieldType::FieldType:          \
            yaml::value(out, field.getValue<Type>());  \
            break

#define READ_SCRIPT_FIELD(FieldType, Type)             \
        case ScriptFieldType::FieldType:                   \
            {                                                  \
                Type data = yaml::read<Type>(scriptField, "Data");    \
                field.setValue(data);                  \
                break;                                         \
            }




    static std::string RigidBody2DBodyTypeToString(Physics2DComponent::BodyType bodyType) {
        switch(bodyType)
        {
        case Physics2DComponent::BodyType::Static:    return "Static";
        case Physics2DComponent::BodyType::Dynamic:   return "Dynamic";
        case Physics2DComponent::BodyType::Kinematic: return "Kinematic";
        }

        return {};
    }

    static Physics2DComponent::BodyType RigidBody2DBodyTypeFromString(const std::string& bodyTypeString) {
        if(bodyTypeString == "Static")     return Physics2DComponent::BodyType::Static;
        if(bodyTypeString == "Dynamic")    return Physics2DComponent::BodyType::Dynamic;
        if(bodyTypeString == "Kinematic")  return Physics2DComponent::BodyType::Kinematic;
        throw YAML::RepresentationException(YAML::Mark::null_mark(),
                  "Unknown Rigidbody2D BodyType '" + bodyTypeString + "'");
    }

    static bool SerializeEntity(YAML::Emitter& out, SceneEntity entity,
        IScriptInteractorFrom::Ptr scriptInteractor, bool recursive,
        const std::string& projectPath) {
        if(!entity || !entity.hasComponent<IDComponent>() || !entity.hasComponent<TagComponent>())
            return false;
        out << YAML::BeginMap;
        yaml::write(out, "Entity", entity.getComponent<IDComponent>().ID);


        const auto childEntities = entity.getChildren();
        if(!childEntities.empty()) {
            std::vector<uint64_t> ids;
            ids.reserve(childEntities.size());
            for(const auto& child: childEntities)
                ids.push_back(child.getUUID());
            yaml::write(out, "HierarchyComponent", YAML::BeginMap);
            yaml::write(out, "Children", ids);
            out << YAML::EndMap;
        }

        if(entity.hasComponent<TagComponent>()) {
            out << YAML::Key << "TagComponent";
            out << YAML::BeginMap;
            auto& tag = entity.getComponent<TagComponent>().getTag();
            yaml::write(out, "Tag", tag);
            out << YAML::EndMap;
        }

        if(entity.hasComponent<CameraComponent>()) {
            out << YAML::Key << "CameraComponent";
            out << YAML::BeginMap;
            auto isPrimary = entity.getComponent<CameraComponent>().isPrimary;
            yaml::write(out, "isPrimary", isPrimary);
            yaml::write(out, "OrthoSize", entity.getComponent<CameraComponent>().orthoSize);
            yaml::write(out, "Size", entity.getComponent<CameraComponent>().size);
            yaml::write(out, "Position", entity.getComponent<CameraComponent>().position);
            out << YAML::EndMap;
        }

        if(entity.hasComponent<TransformComponent>()) {
            out << YAML::Key << "TransformComponent";
            out << YAML::BeginMap;
            auto& ts = entity.getComponent<TransformComponent>();
            yaml::write(out, "Position", ts.getPosition());
            yaml::write(out, "Size", ts.getSize());
            yaml::write(out, "Rotation", ts.getRotate());
            yaml::write(out, "Origin", ts.getOrigin());

            out << YAML::EndMap;
        }

        if(entity.hasComponent<Transform3DComponent>()) {
            out << YAML::Key << "Transform3DComponent";
            out << YAML::BeginMap;
            auto& ts = entity.getComponent<Transform3DComponent>();
            yaml::write(out, "Position", ts.getPosition());
            yaml::write(out, "Scale", ts.getScale());
            yaml::write(out, "Rotation", ts.getRotation());
            out << YAML::EndMap;
        }

        if(entity.hasComponent<DrawableComponent>()) {
            out << YAML::Key << "SpriteComponent";
            out << YAML::BeginMap;
            auto& sp = entity.getComponent<DrawableComponent>();
            yaml::write(out, "Color", sp.getColor());
            yaml::write(out, "TexturePath", projectPath.empty() || sp.getTexturePath().empty()
                        ? sp.getTexturePath() : projectAssetReference(projectPath, sp.getTexturePath()));
            yaml::write(out, "zDepth", sp.getDepth());
            out << YAML::EndMap;
        }

        if(entity.hasComponent<ScriptComponent>()) {
            if(!scriptInteractor) return false;
            auto& scriptComponent = entity.getComponent<ScriptComponent>();

            out << YAML::Key << "ScriptComponent";
            out << YAML::BeginMap;
            yaml::write(out, "ClassName", scriptComponent.name);

            const auto& fields = scriptInteractor -> getFields(entity.getUUID());
            if(!fields.empty()) {
                out << YAML::Key << "ScriptFields" << YAML::Value;
                out << YAML::BeginSeq;
                for(const auto& [name, field] : fields) {
                    auto scriptType = util::convert2Script(field.getType());
                    out << YAML::BeginMap; // ScriptField
                    yaml::write(out, "Name", name);
                    yaml::write(out, "Type", util::ScriptFieldTypeToString(scriptType));
                    out << YAML::Key << "Data" << YAML::Value;

                    switch(scriptType)
                    {
                    WRITE_SCRIPT_FIELD(Float, float     );
                    WRITE_SCRIPT_FIELD(Double, double    );
                    WRITE_SCRIPT_FIELD(Bool, bool      );
                    WRITE_SCRIPT_FIELD(Char, char      );
                    WRITE_SCRIPT_FIELD(Byte, int8_t    );
                    WRITE_SCRIPT_FIELD(Short, int16_t   );
                    WRITE_SCRIPT_FIELD(Int, int32_t   );
                    WRITE_SCRIPT_FIELD(Long, int64_t   );
                    WRITE_SCRIPT_FIELD(UByte, uint8_t   );
                    WRITE_SCRIPT_FIELD(UShort, uint16_t  );
                    WRITE_SCRIPT_FIELD(UInt, uint32_t  );
                    WRITE_SCRIPT_FIELD(ULong, uint64_t  );
                    WRITE_SCRIPT_FIELD(Vector2, robot2D::vec2f );
                    WRITE_SCRIPT_FIELD(Transform, UUID );
                    WRITE_SCRIPT_FIELD(Entity, UUID );
                    }
                    out << YAML::EndMap; // ScriptFields
                }
                out << YAML::EndSeq;

            }
            out << YAML::EndMap; // ScriptComponent
        }

        if(entity.hasComponent<Physics2DComponent>()) {
            out << YAML::Key << "Rigidbody2DComponent";
            out << YAML::BeginMap; // Rigidbody2DComponent

            auto& rb2dComponent = entity.getComponent<Physics2DComponent>();
            yaml::write(out, "BodyType", RigidBody2DBodyTypeToString(rb2dComponent.type));
            yaml::write(out, "FixedRotation", rb2dComponent.fixedRotation);
            yaml::write(out, "Bullet", rb2dComponent.bullet);

            out << YAML::EndMap; // Rigidbody2DComponent
        }

        if(entity.hasComponent<Collider2DComponent>()) {
            out << YAML::Key << "BoxCollider2DComponent";
            out << YAML::BeginMap; // BoxCollider2DComponent

            auto& bc2dComponent = entity.getComponent<Collider2DComponent>();
            yaml::write(out, "Offset", bc2dComponent.offset);
            yaml::write(out, "Size", bc2dComponent.size);
            yaml::write(out, "Density", bc2dComponent.density);
            yaml::write(out, "Friction", bc2dComponent.friction);
            yaml::write(out, "Restitution", bc2dComponent.restitution);
            yaml::write(out, "RestitutionThreshold", bc2dComponent.restitutionThreshold);
            yaml::write(out, "UseDefaultMask", bc2dComponent.useDefaultMask);
            yaml::write(out, "FilterLayer", bc2dComponent.filter.name);

            out << YAML::EndMap; // BoxCollider2DComponent
        }

        if(entity.hasComponent<TextComponent>()) {
            out << YAML::Key << "TextComponent";
            out << YAML::BeginMap; // TextComponent

            auto& text = entity.getComponent<TextComponent>();
            yaml::write(out, "Text", text.getText());
            if(!text.getFontPath().empty())
                yaml::write(out, "FontPath", projectPath.empty() ? text.getFontPath() :
                    projectAssetReference(projectPath, text.getFontPath()));
            out << YAML::EndMap; // TextComponent
        }

        if(entity.hasComponent<PrefabComponent>()) {
            out << YAML::Key << "PrefabComponent";
            out << YAML::BeginMap;
            auto& uuid = entity.getComponent<PrefabComponent>().prefabUUID;
            yaml::write(out, "UUID", uuid);
            out << YAML::EndMap;
        }

        if(entity.hasComponent<AnimationComponent>()) {
            auto* animationManager = LocalResourceManager::getManager();
            const std::deque<Animation> empty;
            const auto* found = animationManager -> findAnimations(entity.getUUID());
            const auto& animations = found ? *found : empty;

            out << YAML::Key << "AnimationComponent";
            out << YAML::BeginMap;
            std::vector<std::string> animationPaths;
            animationPaths.reserve(animations.size());
            AnimationParser animationParser;

            for(auto animation: animations) {
                animationPaths.emplace_back(projectAssetReference(projectPath, animation.filePath));
                animation.texturePath = projectAssetReference(projectPath, animation.texturePath);
                if(!animationParser.saveToFile(animation.filePath, &animation))
                    throw std::runtime_error("Could not save animation: " + animation.filePath);
            }
            if(!found)
                animationPaths = ResourceManager::getManager() -> getAnimationPaths(entity.getUUID());

            yaml::write(out, "Animations", animationPaths);
            out << YAML::EndMap;
        }

        if(entity.hasComponent<ButtonComponent>()) {
            auto& btnComp = entity.getComponent<ButtonComponent>();
            if(btnComp.hasEntity()) {
                out << YAML::Key << "ButtonComponent";
                out << YAML::BeginMap;
                yaml::write(out, "ScriptUUID", btnComp.scriptEntity);
                if(!btnComp.clickMethodName.empty())
                    yaml::write(out, "MethodName", btnComp.clickMethodName);
                out << YAML::EndMap;
            }
        }

        out << YAML::EndMap;

        if(recursive) {
            for(const auto& child: childEntities)
                if(!SerializeEntity(out, child, scriptInteractor, true, projectPath))
                    return false;
        }
        return out.good();
    }

    IEntitySerializer::~IEntitySerializer() noexcept = default;

    bool EntityYAMLSerializer::serialize(YAML::Emitter& out, const SceneEntity& entity,
        IScriptInteractorFrom::Ptr scriptInteractor, bool recursive) {
        m_errorMessage.clear();
        try {
            if(SerializeEntity(out, entity, scriptInteractor, recursive, m_projectPath) && out.good())
                return true;
            m_errorMessage = "Invalid entity, missing script interactor or incomplete YAML output";
        }
        catch(const std::exception& error) {
            m_errorMessage = error.what();
        }
        return false;
    }

    bool EntityYAMLSerializer::deserialize(const YAML::Node& iterator,
        SceneEntity& deserializedEntity,
        bool& addToScene,
        std::vector<ChildInfo>& children,
        IScriptInteractorFrom::Ptr scriptInteractor) {

        m_errorMessage.clear();
        try {
            const auto& entity = iterator;
            if(!entity.IsMap())
                throw YAML::RepresentationException(entity.Mark(), "Expected an entity map");
            static const std::unordered_set<std::string> componentNames {
                "Entity", "TagComponent", "CameraComponent", "TransformComponent", "Transform3DComponent",
                "HierarchyComponent", "SpriteComponent", "ScriptComponent", "Rigidbody2DComponent",
                "BoxCollider2DComponent", "TextComponent", "PrefabComponent", "AnimationComponent", "ButtonComponent"
            };
            for(const auto& item: entity) {
                const auto key = item.first.as<std::string>();
                if(!componentNames.count(key))
                    throw YAML::RepresentationException(item.first.Mark(), "Unsupported entity field '" + key + "'");
                if(key != "Entity" && !item.second.IsMap())
                    throw YAML::RepresentationException(item.second.Mark(), "Component '" + key + "' must be a map");
            }

            /// BASE
            if(!entity["Entity"] || !entity["TagComponent"])
                throw YAML::RepresentationException(entity.Mark(), "Entity and TagComponent are required");

            auto uuid = yaml::read<UUID>(entity, "Entity");
            if(!deserializedEntity.hasComponent<IDComponent>())
                deserializedEntity.addComponent<IDComponent>();
            if(!deserializedEntity.hasComponent<TagComponent>())
                deserializedEntity.addComponent<TagComponent>();
            deserializedEntity.getComponent<IDComponent>().ID = uuid;
            auto& tagData = deserializedEntity.getComponent<TagComponent>();
            tagData.setTag(yaml::read<std::string>(yaml::required(entity, "TagComponent"), "Tag"));

            auto cameraComponent = entity["CameraComponent"];
            if(cameraComponent) {
                auto& isPrimary = deserializedEntity.addComponent<CameraComponent>().isPrimary;
                isPrimary = yaml::read<bool>(cameraComponent, "isPrimary");
                deserializedEntity.getComponent<CameraComponent>().orthoSize = yaml::read<float>(cameraComponent,
                        "OrthoSize");
                deserializedEntity.getComponent<CameraComponent>().size = yaml::read<robot2D::vec2f>(cameraComponent,
                        "Size");
                deserializedEntity.getComponent<CameraComponent>().position =
                    yaml::read<robot2D::vec2f>(cameraComponent, "Position");
            }

            auto transformComponent = entity["TransformComponent"];
            if(transformComponent) {
                auto& transform = deserializedEntity.addComponent<TransformComponent>();
                transform.setPosition(yaml::read<robot2D::vec2f>(transformComponent, "Position"));
                transform.setSize(yaml::read<robot2D::vec2f>(transformComponent, "Size"));
                transform.setRotate(yaml::read<float>(transformComponent, "Rotation"));
                if(transformComponent["Origin"])
                    transform.setOrigin(yaml::read<robot2D::vec2f>(transformComponent, "Origin"));
                ChildInfo childInfo;

                if(!entity["HierarchyComponent"] && yaml::optional<bool>(transformComponent, "HasChildren", false)) {
                    auto childIDS = yaml::read<std::vector<UUID> >(transformComponent, "ChildIDs");
                    ChildPair pair = std::make_pair(uuid, childIDS);
                    childInfo.childPair = std::move(pair);
                }
                if(!entity["HierarchyComponent"] && yaml::optional<bool>(transformComponent, "isChild", false)) {
                    auto parentID = yaml::read<UUID>(transformComponent, "ParentID");
                    childInfo.isChild = true;
                    childInfo.parentUUID = parentID;
                    childInfo.self = deserializedEntity;
                    addToScene = false;
                }

                if(!childInfo.isEmpty())
                    children.emplace_back(childInfo);

            }

            if(const auto hierarchy = entity["HierarchyComponent"]) {
                ChildInfo info;
                info.childPair.first = uuid;
                if(hierarchy["Children"])
                    info.childPair.second = yaml::read<std::vector<UUID> >(hierarchy, "Children");
                if(info.hasChildren())
                    children.push_back(std::move(info));
            }

            if(const auto transform3D = entity["Transform3DComponent"]) {
                auto& transform = deserializedEntity.addComponent<Transform3DComponent>();
                transform.setPosition(yaml::read<robot2D::vec3f>(transform3D, "Position"));
                transform.setScale(yaml::read<robot2D::vec3f>(transform3D, "Scale"));
                transform.setRotation(yaml::read<robot2D::vec3f>(transform3D, "Rotation"));
            }

            auto spriteComponent = entity["SpriteComponent"];
            if(spriteComponent) {
                auto& drawable = deserializedEntity.addComponent<DrawableComponent>();
                drawable.setColor(yaml::read<robot2D::Color>(spriteComponent, "Color"));
                if(spriteComponent["TexturePath"])
                    drawable.setTexturePath(yaml::read<std::string>(spriteComponent, "TexturePath"));
                if(spriteComponent["zDepth"])
                    drawable.setDepth(yaml::read<int>(spriteComponent, "zDepth"));
                drawable.setReorderZBuffer(true);
            }

            auto textComponent = entity["TextComponent"];
            if(textComponent) {
                auto& text = deserializedEntity.addComponent<TextComponent>();
                text.setText(yaml::read<std::string>(textComponent, "Text"));
                if(textComponent["FontPath"])
                    text.setFontPath(yaml::read<std::string>(textComponent, "FontPath"));
            }

            auto scriptComponent = entity["ScriptComponent"];
            if(scriptComponent) {
                if(!scriptInteractor)
                    throw YAML::RepresentationException(scriptComponent.Mark(),
                              "ScriptComponent requires a script interactor");
                auto& sc = deserializedEntity.addComponent<ScriptComponent>();
                sc.name = yaml::read<std::string>(scriptComponent, "ClassName");

                auto scriptFields = scriptComponent["ScriptFields"];
                if(scriptFields) {
                    if(!scriptFields.IsSequence())
                        throw YAML::RepresentationException(scriptFields.Mark(), "ScriptFields must be a sequence");
                    auto& fields = scriptInteractor -> getFields(uuid);
                    for(const auto& scriptField: scriptFields) {
                        std::string name = yaml::read<std::string>(scriptField, "Name");
                        std::string typeString = yaml::read<std::string>(scriptField, "Type");
                        static const std::unordered_set<std::string> supportedTypes {
                            "Float", "Int", "Transform", "Entity"
                        };
                        if(!supportedTypes.count(typeString))
                            throw YAML::RepresentationException(scriptField.Mark(),
                                      "Unsupported script field type '" + typeString + "'");
                        yaml::required(scriptField, "Data");
                        ScriptFieldType type = util::ScriptFieldTypeFromString(typeString);

                        auto& field = fields[name];
                        field.setName(name);
                        field.setType(util::convertFromScript(type));

                        switch(type)
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
                            READ_SCRIPT_FIELD(Entity, UUID);
                        }
                    }
                }
            }

            auto rigidbody2DComponent = entity["Rigidbody2DComponent"];
            if(rigidbody2DComponent) {
                auto& rb2d = deserializedEntity.addComponent<Physics2DComponent>();
                rb2d.type = RigidBody2DBodyTypeFromString(yaml::read<std::string>(rigidbody2DComponent, "BodyType"));
                rb2d.fixedRotation = yaml::read<bool>(rigidbody2DComponent, "FixedRotation");
                if(rigidbody2DComponent["Bullet"])
                    rb2d.bullet = yaml::read<bool>(rigidbody2DComponent, "Bullet");
            }

            auto boxCollider2DComponent = entity["BoxCollider2DComponent"];
            if(boxCollider2DComponent) {
                auto& bc2d = deserializedEntity.addComponent<Collider2DComponent>();
                bc2d.offset = yaml::read<robot2D::vec2f>(boxCollider2DComponent, "Offset");
                bc2d.size = yaml::read<robot2D::vec2f>(boxCollider2DComponent, "Size");
                bc2d.density = yaml::read<float>(boxCollider2DComponent, "Density");
                bc2d.friction = yaml::read<float>(boxCollider2DComponent, "Friction");
                bc2d.restitution = yaml::read<float>(boxCollider2DComponent, "Restitution");
                bc2d.restitutionThreshold = yaml::read<float>(boxCollider2DComponent, "RestitutionThreshold");
                if(boxCollider2DComponent["UseDefaultMask"])
                    bc2d.useDefaultMask = yaml::read<bool>(boxCollider2DComponent, "UseDefaultMask");
                if(boxCollider2DComponent["FilterLayer"]) {
                    auto filterLayer = yaml::read<std::string>(boxCollider2DComponent, "FilterLayer");
                    auto& registry = phys2d::LayerRegistry::I();
                    if(std::find_if(registry.names().begin(), registry.names().end(), [&](const auto& name) {
                        return name == filterLayer;
                    }) != registry.names().end())
                        bc2d.filter = phys2d::makeFilterDefault(filterLayer);
                }
            }

            auto prefabComponent = entity["PrefabComponent"];
            if(prefabComponent) {
                deserializedEntity.addComponent<PrefabComponent>().prefabUUID = yaml::read<uint64_t>(prefabComponent,
                        "UUID");
            }

            auto animationComponent = entity["AnimationComponent"];
            if(animationComponent) {
                auto resourceManager = ResourceManager::getManager();
                auto animationPaths = yaml::read<std::vector<std::string> >(animationComponent, "Animations");
                resourceManager -> setAnimationPathsToLoad(uuid, std::move(animationPaths));
                deserializedEntity.addComponent<AnimationComponent>();
                deserializedEntity.addComponent<AnimatorComponent>();
            }

            auto buttonComponent = entity["ButtonComponent"];
            if(buttonComponent) {
                auto& btnComponent = deserializedEntity.addComponent<ButtonComponent>();
                deserializedEntity.addComponent<UIHitbox>();
                btnComponent.scriptEntity = yaml::read<UUID>(buttonComponent, "ScriptUUID");
                if(buttonComponent["MethodName"])
                    btnComponent.clickMethodName = yaml::read<std::string>(buttonComponent, "MethodName");
            }

            return true;
        }
        catch(const std::exception& error) {
            m_errorMessage = error.what();
            return false;
        }
    }

} // namespace editor
