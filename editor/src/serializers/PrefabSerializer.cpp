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
#include <editor/serializers/PrefabSerializer.hpp>
#include <editor/serializers/EntitySerializer.hpp>
#include <editor/serializers/SceneSerializer.hpp>
#include <editor/serializers/YamlFields.hpp>
#include <editor/serializers/YamlFile.hpp>
#include <editor/Scene.hpp>
#include <editor/Components.hpp>

namespace editor {

    namespace {
        constexpr unsigned prefabVersion = 1;
        using EntityID = std::uint64_t;

        struct PrefabStorage {
            PrefabStorage() : scene(messages) {
            }
            PrefabStorage(const PrefabStorage&) = delete;
            PrefabStorage& operator=(const PrefabStorage&) = delete;
            PrefabStorage(PrefabStorage&&) = delete;
            PrefabStorage& operator=(PrefabStorage&&) = delete;
            ~PrefabStorage() = default;

            robot2D::MessageBus messages;
            Scene scene;
        };

        std::unordered_map<EntityID, EntityID> remapTemplateIDs(YAML::Node& nodes, bool legacy) {
            std::unordered_set<EntityID> used;
            for(const auto& node: nodes) {
                const auto id = yaml::read<EntityID>(node, "Entity");
                if(!used.insert(id).second)
                    throw YAML::RepresentationException(node.Mark(), "Duplicate prefab entity UUID");
            }
            std::unordered_map<EntityID, EntityID> ids;
            YAML::Node normalized(YAML::NodeType::Sequence);
            for(const auto& node: nodes) {
                // Normalize the identical duplicate Entity keys emitted by older writers.
                YAML::Node copy(YAML::NodeType::Map);
                for(const auto& field: node)
                    copy[field.first.as<std::string>()] = YAML::Clone(field.second);
                const auto oldID = yaml::read<EntityID>(node, "Entity");
                EntityID newID = UUID();
                while(newID == 0 || !used.insert(newID).second)
                    newID = UUID();
                ids.emplace(oldID, newID);
                copy["Entity"] = newID;
                normalized.push_back(copy);
            }
            nodes = normalized;
            const auto remap = [&](EntityID id) {
                const auto found = ids.find(id);
                return found == ids.end() ? id : found -> second;
            };
            for(auto node: nodes) {
                if(const auto hierarchy = node["HierarchyComponent"]) {
                    if(hierarchy["Children"]) {
                        auto children = yaml::read<std::vector<EntityID> >(hierarchy, "Children");
                        for(auto& child: children) {
                            if(!ids.count(child))
                                throw YAML::RepresentationException(hierarchy.Mark(), "Unknown prefab child UUID");
                            child = remap(child);
                        }
                        node["HierarchyComponent"]["Children"] = children;
                    }
                }
                if(const auto transform = node["TransformComponent"]) {
                    if(transform["ParentID"]) {
                        const auto parent = yaml::read<EntityID>(transform, "ParentID");
                        if(legacy && !ids.count(parent) && node == nodes[0]) {
                            // Old prefabs exported from a child retained its external scene parent.
                            node["TransformComponent"].remove("ParentID");
                            node["TransformComponent"].remove("isChild");
                        }
                        else if(ids.count(parent))
                            node["TransformComponent"]["ParentID"] = remap(parent);
                        else
                            throw YAML::RepresentationException(transform.Mark(), "Unknown prefab parent UUID");
                    }
                    if(transform["ChildIDs"]) {
                        auto children = yaml::read<std::vector<EntityID> >(transform, "ChildIDs");
                        for(auto& child: children) {
                            if(!ids.count(child))
                                throw YAML::RepresentationException(transform.Mark(), "Unknown prefab child UUID");
                            child = remap(child);
                        }
                        node["TransformComponent"]["ChildIDs"] = children;
                    }
                }
                if(const auto button = node["ButtonComponent"]) {
                    const auto id = yaml::read<EntityID>(button, "ScriptUUID");
                    if(id != 0) node["ButtonComponent"]["ScriptUUID"] = remap(id);
                }
                if(const auto script = node["ScriptComponent"]) {
                    const auto fields = script["ScriptFields"];
                    if(fields) {
                        if(!fields.IsSequence())
                            throw YAML::RepresentationException(fields.Mark(), "ScriptFields must be a sequence");
                        for(auto field: fields) {
                            const auto type = yaml::read<std::string>(field, "Type");
                            if(type == "Entity" || type == "Transform") {
                                const auto id = yaml::read<EntityID>(field, "Data");
                                if(id != 0) field["Data"] = remap(id);
                            }
                        }
                    }
                }
            }
            return ids;
        }
    }

    PrefabSerializer::PrefabSerializer() = default;

    bool PrefabSerializer::serialize(Prefab::Ptr prefab, IScriptInteractorFrom::Ptr scriptInteractor) {
        m_error = PrefabError::BadSerialization;
        m_errorMessage.clear();
        if(!prefab || !prefab -> getEntity())
            return false;
        try {
            const auto entity = prefab -> getEntity();
            if(!entity.hasComponent<TagComponent>())
                return false;
            YAML::Emitter entities;
            entities << YAML::BeginSeq;
            EntityYAMLSerializer serializer(prefab -> m_projectPath);
            if(!serializer.serialize(entities, entity, scriptInteractor)) {
                m_errorMessage = serializer.getErrorMessage();
                return false;
            }
            entities << YAML::EndSeq;
            if(!entities.good())
                return false;
            auto nodes = YAML::Load(entities.c_str());
            nodes[0]["PrefabComponent"]["UUID"] = static_cast<EntityID>(prefab -> getUUID());
            YAML::Node data(YAML::NodeType::Map);
            data["PrefabVersion"] = prefabVersion;
            data["Prefab"] = entity.getName();
            data["PrefabUUID"] = static_cast<EntityID>(prefab -> getUUID());
            data["EntityCount"] = nodes.size();
            data["RootEntity"] = static_cast<EntityID>(entity.getUUID());
            data["Entities"] = nodes;
            YAML::Emitter out;
            out << data;
            if(!yaml::writeFile(prefab -> getPath(), out)) {
                m_errorMessage = "Could not replace prefab file: " + prefab -> getPath();
                return false;
            }
            m_error = PrefabError::None;
            if(prefab -> getTemplateScene())
                prefab -> getTemplateScene() -> markSaved();
            return true;
        }
        catch(const std::exception& error) {
            m_errorMessage = error.what();
            return false;
        }
    }

    bool PrefabSerializer::deserialize(Prefab::Ptr prefab, IScriptInteractorFrom::Ptr scriptInteractor) {
        m_error = PrefabError::BadDeSerialization;
        m_errorMessage.clear();
        if(!prefab)
            return false;
        try {
            std::ifstream input(std::filesystem::u8path(prefab -> getPath()), std::ios::binary);
            if(!input.is_open()) {
                m_error = PrefabError::NoFileOpen;
                m_errorMessage = "Could not open prefab: " + prefab -> getPath();
                return false;
            }
            std::stringstream contents;
            contents << input.rdbuf();
            if(input.bad()) {
                m_errorMessage = "Could not read complete prefab file";
                return false;
            }
            auto data = YAML::Load(contents.str());
            const auto version = yaml::optional<unsigned long long>(data, "PrefabVersion", 0);
            if(version > prefabVersion) {
                m_errorMessage = "Unsupported prefab version " + std::to_string(version);
                return false;
            }
            yaml::validateKeys(data, version == 0);
            const auto name = yaml::read<std::string>(data, "Prefab");
            const auto uuid = yaml::read<EntityID>(data, "PrefabUUID");
            auto nodes = yaml::required(data, "Entities");
            if(!nodes.IsSequence() || nodes.size() == 0) {
                m_errorMessage = "Prefab requires at least one entity";
                return false;
            }
            const auto ids = remapTemplateIDs(nodes, version == 0);
            YAML::Node sceneDocument(YAML::NodeType::Map);
            sceneDocument["Scene"] = name;
            sceneDocument["Entities"] = nodes;
            if(version != 0) {
                const auto root = yaml::read<EntityID>(data, "RootEntity");
                const auto found = ids.find(root);
                if(found == ids.end()) {
                    m_errorMessage = "RootEntity is missing from Entities";
                    return false;
                }
                sceneDocument["SceneVersion"] = 1;
                sceneDocument["EntityCount"] = yaml::read<std::size_t>(data, "EntityCount");
                sceneDocument["RootEntities"].push_back(found -> second);
            }
            auto storage = std::make_shared<PrefabStorage>();
            auto scene = std::shared_ptr<Scene>(storage, &storage -> scene);
            scene -> setAssociatedProjectPath(prefab -> m_projectPath);
            SceneSerializer serializer(scene);
            if(!serializer.deserializeDocument(sceneDocument, scriptInteractor)) {
                m_errorMessage = serializer.getErrorMessage();
                return false;
            }
            if(scene -> getRootEntities().size() != 1) {
                m_errorMessage = "Prefab must have exactly one root entity";
                return false;
            }
            auto root = scene -> getRootEntities().front();
            if(!root.hasComponent<PrefabComponent>())
                root.addComponent<PrefabComponent>();
            root.getComponent<PrefabComponent>().prefabUUID = UUID{uuid};
            scene -> markSaved();
            // Commit ownership only after the complete subtree has passed validation.
            prefab -> m_entity = root;
            prefab -> m_templateScene = std::move(scene);
            prefab -> m_uuid = UUID{uuid};
            m_error = PrefabError::None;
            return true;
        }
        catch(const std::exception& error) {
            m_errorMessage = error.what();
            return false;
        }
    }

    PrefabError PrefabSerializer::getError() const {
        return m_error;
    }
}
