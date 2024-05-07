#include <editor/Prefab.hpp>
#include <editor/Components.hpp>

namespace editor {
    Prefab::Prefab(const std::string& path, const SceneEntity& prefabEntity, UUID uuid):
    m_path(path), m_entity(prefabEntity), m_uuid(uuid) { }

    void Prefab::setEntity(const SceneEntity& entity) {
        m_entity = entity;
    }

    void Prefab::setPath(const std::string& path) {
        m_path = path;
    }

    void Prefab::addUUID() {
        m_entity.addComponent<PrefabComponent>().prefabUUID = m_uuid;
    }

    void Prefab::setUUID(UUID uuid) {
        m_uuid = uuid;
    }

} // namespace editor