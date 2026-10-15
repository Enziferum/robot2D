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
#include <filesystem>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <algorithm>
#include <editor/serializers/SceneSerializer.hpp>
#include <editor/serializers/EntitySerializer.hpp>
#include <editor/serializers/YamlFields.hpp>
#include <editor/serializers/YamlFile.hpp>
#include <editor/Scene.hpp>
#include <editor/Components.hpp>

namespace editor {

    namespace {
        constexpr unsigned sceneVersion = 1;
        using EntityID = std::uint64_t;

        struct EntityRecord {
            EntityID id;
            YAML::Node data;
            std::vector<EntityID> children;
            std::optional<EntityID> legacyParent;
        };
    }

    SceneSerializer::SceneSerializer(Scene::Ptr scene): m_scene(std::move(scene)) {}

    bool SceneSerializer::serialize(const std::string& path, const std::string& sceneName,
                                     IScriptInteractorFrom::Ptr scriptingEngine) {
        m_error = SceneSerializerError::None;
        m_errorMessage.clear();
        if(!m_scene) {
            m_error = SceneSerializerError::InvalidScene;
            return false;
        }
        try {
            std::vector<SceneEntity> entities;
            std::vector<EntityID> roots;
            for(const auto& root: m_scene -> getRootEntities())
                roots.push_back(root.getUUID());
            std::vector<SceneEntity> pending;
            for(auto it = m_scene -> getRootEntities().rbegin(); it != m_scene -> getRootEntities().rend(); ++it)
                pending.push_back(*it);
            std::unordered_set<EntityID> visited;
            while(!pending.empty()) {
                auto entity = pending.back();
                pending.pop_back();
                if(!entity || !visited.insert(entity.getUUID()).second) {
                    m_error = SceneSerializerError::InvalidHierarchy;
                    m_errorMessage = "Repeated or invalid entity while traversing the scene";
                    return false;
                }
                entities.push_back(entity);
                auto children = entity.getChildren();
                for(auto it = children.rbegin(); it != children.rend(); ++it)
                    pending.push_back(*it);
            }

            YAML::Emitter out;
            out << YAML::BeginMap;
            yaml::write(out, "SceneVersion", sceneVersion);
            yaml::write(out, "Scene", sceneName);
            yaml::write(out, "EntityCount", entities.size());
            out << YAML::Key << "RootEntities" << YAML::Value << YAML::Flow << roots;
            out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
            EntityYAMLSerializer serializer;
            for(const auto& entity: entities) {
                if(!serializer.serialize(out, entity, scriptingEngine, false)) {
                    m_error = SceneSerializerError::InvalidData;
                    m_errorMessage = "Entity " + std::to_string(static_cast<EntityID>(entity.getUUID())) +
                                     ": " + serializer.getErrorMessage();
                    return false;
                }
            }
            out << YAML::EndSeq << YAML::EndMap;
            if(!yaml::writeFile(path, out)) {
                m_error = SceneSerializerError::WriteFailed;
                m_errorMessage = "Could not replace scene file: " + path;
                return false;
            }
            return true;
        }
        catch(const std::exception& error) {
            m_error = SceneSerializerError::InvalidData;
            m_errorMessage = error.what();
            return false;
        }
    }

    bool SceneSerializer::deserialize(const std::string& path, IScriptInteractorFrom::Ptr scriptingEngine) {
        m_error = SceneSerializerError::None;
        m_errorMessage.clear();
        try {
            std::ifstream input(std::filesystem::u8path(path), std::ios::binary);
            if(!input.is_open()) {
                m_error = SceneSerializerError::NoFileOpen;
                m_errorMessage = "Could not open scene: " + path;
                return false;
            }
            std::stringstream contents;
            contents << input.rdbuf();
            if(input.bad()) {
                m_error = SceneSerializerError::InvalidData;
                m_errorMessage = "Could not read complete scene file";
                return false;
            }
            const auto data = YAML::Load(contents.str());
            return deserializeDocument(data, scriptingEngine);
        }
        catch(const std::exception& error) {
            m_error = SceneSerializerError::InvalidData;
            m_errorMessage = error.what();
            return false;
        }
    }

    bool SceneSerializer::deserializeDocument(const YAML::Node& data, IScriptInteractorFrom::Ptr scriptingEngine) {
        m_error = SceneSerializerError::None;
        m_errorMessage.clear();
        auto fail = [&](SceneSerializerError error, const std::string& message) {
            m_error = error;
            m_errorMessage = message;
            return false;
        };
        if(!m_scene || m_scene -> isRunning() || !m_scene -> getRootEntities().empty())
            return fail(SceneSerializerError::InvalidScene, "Loading requires a fresh, empty editor scene");
        try {
            const auto version = yaml::optional<unsigned long long>(data, "SceneVersion", 0);
            if(version > sceneVersion)
                return fail(SceneSerializerError::UnsupportedVersion, "Scene version " + std::to_string(version));
            const bool legacy = version == 0;
            yaml::validateKeys(data, legacy);
            if(!data["Scene"])
                return fail(SceneSerializerError::NotSceneTag, "Missing scene name");
            const auto name = yaml::read<std::string>(data, "Scene");
            const auto nodes = yaml::required(data, "Entities");
            if(!nodes.IsSequence())
                return fail(SceneSerializerError::InvalidData, "Entities must be a sequence");
            if(!legacy && yaml::read<std::size_t>(data, "EntityCount") != nodes.size())
                return fail(SceneSerializerError::CountMismatch, "EntityCount does not match Entities.size()");

            std::vector<EntityRecord> records;
            records.reserve(nodes.size());
            std::unordered_map<EntityID, std::size_t> index;
            index.reserve(nodes.size());
            for(const auto& node: nodes) {
                const auto id = yaml::read<EntityID>(node, "Entity");
                if(!index.emplace(id, records.size()).second || m_scene -> getEntity(UUID{id}))
                    return fail(SceneSerializerError::InvalidData, "Duplicate entity UUID " + std::to_string(id));
                if(!yaml::required(node, "TagComponent").IsMap())
                    return fail(SceneSerializerError::InvalidData, "TagComponent must be a map");
                yaml::read<std::string>(node["TagComponent"], "Tag");
                records.push_back({id, node, {}, {}});
                auto& record = records.back();
                if(const auto hierarchy = node["HierarchyComponent"]) {
                    if(!hierarchy.IsMap())
                        return fail(SceneSerializerError::InvalidHierarchy, "HierarchyComponent must be a map");
                    record.children = yaml::optional<std::vector<EntityID>>(hierarchy, "Children", {});
                }
                if(const auto transform = node["TransformComponent"]) {
                    if(!transform.IsMap())
                        return fail(SceneSerializerError::InvalidData, "TransformComponent must be a map");
                    if(!legacy && (transform["ParentID"] || transform["ChildIDs"] ||
                                   transform["isChild"] || transform["HasChildren"]))
                        return fail(SceneSerializerError::InvalidHierarchy, "Legacy hierarchy fields in a versioned scene");
                    if(legacy && !node["HierarchyComponent"]) {
                        if(transform["ChildIDs"])
                            record.children = yaml::read<std::vector<EntityID>>(transform, "ChildIDs");
                        else if(yaml::optional<bool>(transform, "HasChildren", false))
                            return fail(SceneSerializerError::InvalidHierarchy, "HasChildren requires ChildIDs");
                        if(yaml::optional<bool>(transform, "isChild", false) || transform["ParentID"])
                            record.legacyParent = yaml::read<EntityID>(transform, "ParentID");
                    }
                }
            }

            std::unordered_map<EntityID, EntityID> parents;
            parents.reserve(records.size());
            for(const auto& record: records) {
                for(const auto child: record.children) {
                    if(!index.count(child) || child == record.id || !parents.emplace(child, record.id).second)
                        return fail(SceneSerializerError::InvalidHierarchy, "Invalid, repeated or multiply parented child " + std::to_string(child));
                }
            }
            if(legacy) {
                for(const auto& record: records) {
                    if(!record.legacyParent)
                        continue;
                    const auto parent = *record.legacyParent;
                    if(!index.count(parent) || parent == record.id)
                        return fail(SceneSerializerError::InvalidHierarchy, "Invalid legacy parent");
                    const auto found = parents.find(record.id);
                    if(found != parents.end() && found -> second != parent)
                        return fail(SceneSerializerError::InvalidHierarchy, "ParentID contradicts ChildIDs");
                    if(found == parents.end()) {
                        records[index.at(parent)].children.push_back(record.id);
                        parents.emplace(record.id, parent);
                    }
                }
            }
            std::vector<EntityID> roots;
            if(legacy) {
                for(const auto& record: records)
                    if(!parents.count(record.id))
                        roots.push_back(record.id);
            }
            else
                roots = yaml::read<std::vector<EntityID>>(data, "RootEntities");
            std::unordered_set<EntityID> visited;
            visited.reserve(records.size());
            std::vector<EntityID> pending = roots;
            while(!pending.empty()) {
                const auto id = pending.back();
                pending.pop_back();
                if(!index.count(id) || !visited.insert(id).second)
                    return fail(SceneSerializerError::InvalidHierarchy, "Unknown or repeated hierarchy entry");
                const auto& children = records[index.at(id)].children;
                pending.insert(pending.end(), children.begin(), children.end());
            }
            for(const auto root: roots)
                if(parents.count(root))
                    return fail(SceneSerializerError::InvalidHierarchy, "A root entity also has a parent");
            if(visited.size() != records.size())
                return fail(SceneSerializerError::InvalidHierarchy, "Hierarchy contains a cycle or unreachable entity");

            // Register every UUID before loading components which can refer to other entities.
            for(const auto& record: records) {
                auto entity = m_scene -> createEmptyEntity();
                entity.addComponent<IDComponent>().ID = UUID{record.id};
                entity.addComponent<TagComponent>();
                if(!m_scene -> attachEntity(entity))
                    return fail(SceneSerializerError::InvalidData, "Could not register entity");
            }
            EntityYAMLSerializer serializer;
            std::vector<ChildInfo> unusedHierarchy;
            for(const auto& record: records) {
                auto entity = m_scene -> getEntity(UUID{record.id});
                bool addToScene = true;
                if(!serializer.deserialize(record.data, entity, addToScene, unusedHierarchy, scriptingEngine))
                    return fail(SceneSerializerError::InvalidData, "Entity " + std::to_string(record.id) +
                                ": " + serializer.getErrorMessage());
                // Rebind loaded TransformComponent to its owning scene; this does not add an ECS entity twice.
                if(!m_scene -> attachEntity(entity))
                    return fail(SceneSerializerError::InvalidData, "Could not finalize entity");
            }
            for(const auto& record: records)
                for(const auto child: record.children)
                    if(!m_scene -> reparent(m_scene -> getEntity(UUID{child}), m_scene -> getEntity(UUID{record.id})))
                        return fail(SceneSerializerError::InvalidHierarchy, "Could not attach child");
            for(std::size_t i = roots.size(); i > 1; --i)
                if(!m_scene -> setBefore(m_scene -> getEntity(UUID{roots[i - 2]}), m_scene -> getEntity(UUID{roots[i - 1]})))
                    return fail(SceneSerializerError::InvalidHierarchy, "Could not restore root order");
            m_scene -> setName(name);
            m_scene -> markSaved();
            return true;
        }
        catch(const std::exception& error) {
            return fail(SceneSerializerError::InvalidData, error.what());
        }
    }

    SceneSerializerError SceneSerializer::getError() const {
        return m_error;
    }

} // namespace editor
