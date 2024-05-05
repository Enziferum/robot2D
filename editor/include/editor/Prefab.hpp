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

#pragma once
#include <memory>
#include <string>
#include <editor/SceneEntity.hpp>
#include "Uuid.hpp"

namespace editor {

    class Prefab final {
    public:
        using Ptr = std::shared_ptr<Prefab>;
    public:

        Prefab() = default;
        Prefab(const std::string& path, const SceneEntity& prefabEntity, UUID uuid) { }
        Prefab(const Prefab& other) = delete;
        Prefab& operator=(const Prefab& other) = delete;
        Prefab(Prefab&& other) = delete;
        Prefab& operator=(Prefab&& other) = delete;
        ~Prefab() = default;

        void setEntity(const SceneEntity& entity) { }
        void setPath(const std::string& path) { }
        void addUUID() { }

        const SceneEntity& getEntity() const { return m_entity; }
        const std::string& getPath() const { return m_path; }
        const UUID& getUUID() const { return m_uuid; }
    private:
        std::string m_path;
        SceneEntity m_entity;
        UUID m_uuid;
    };

} // namespace editor