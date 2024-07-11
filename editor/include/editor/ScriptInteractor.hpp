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
#include <algorithm>

#include <editor/scripting/ScriptingEngineService.hpp>
#include "SceneEntity.hpp"
#include "EditorInteractor.hpp"

namespace editor {

    enum class FieldType {
        None,
        Float,
        Int,
        Transform,
        Entity
    };

    namespace util {
        FieldType convertFromScript(const ScriptFieldType& type) {
            switch(type) {
                default:
                    return FieldType::None;
                case ScriptFieldType::Float:
                    return FieldType::Float;
                case ScriptFieldType::Int:
                    return FieldType::Int;
                case ScriptFieldType::Transform:
                    return FieldType::Transform;
                case ScriptFieldType::Entity:
                    return FieldType::Entity;
            }
        }

        ScriptFieldType convert2Script(const FieldType& type) {
            switch(type) {
                default:
                    return ScriptFieldType::None;
                case FieldType::Float:
                    return ScriptFieldType::Float;
                case FieldType::Int:
                    return ScriptFieldType::Int;
                case FieldType::Transform:
                    return ScriptFieldType::Transform;
                case FieldType::Entity:
                    return ScriptFieldType::Entity;
            }
        }
    }

    class Field {
    public:
        Field() = default;
        Field(const Field& other) = default;
        Field& operator=(const Field& other) = default;
        Field(Field&& other) = default;
        Field& operator=(Field&& other) = default;
        ~Field() = default;

        template<typename T>
        void setValue(const T& value) {
            assert(sizeof(value) <= 64);
            std::memcpy(&m_buffer, &value, sizeof(T));
        }

        template<typename T>
        T* getValue() {
            return (T*)m_buffer;
        }

        template<typename T>
        const T getValue() const {
            return *(T*)m_buffer;
        }

        void setType(const FieldType& fieldType);
        FieldType getType() const;

        void setName(const std::string& name);
        const std::string& getName() const;
    private:
        std::string name;
        FieldType m_type;
        std::uint8_t m_buffer[64];
    };

    //// \brief Using for getting info from ScriptingEngine
    class IScriptInteractorFrom {
    public:
        using FieldMap = std::unordered_map<std::string, Field>;
        using Ptr = std::shared_ptr<IScriptInteractorFrom>;
        using WeakPtr = std::weak_ptr<IScriptInteractorFrom>;

        virtual bool hasEntityClass(const std::string& name) const = 0;
        virtual void setScriptClass(std::string name, UUID uuid) = 0;
        virtual FieldMap& getFields(UUID uuid) = 0;
        virtual std::vector<std::string> getClassesNames() const = 0;
    };

    //// \brief Using for getting info to ScriptingEngine
    class IScriptInteractor {
    public:
        using Ptr = std::shared_ptr<IScriptInteractor>;

        virtual ~IScriptInteractor() = 0;
        virtual SceneEntity getEntity(std::uint64_t entityID) = 0;

        virtual bool loadSceneRuntime(std::string&& name) = 0;
        virtual void loadSceneAsyncRuntime(std::string&& name) = 0;
        virtual void exitEngineRuntime() = 0;
        virtual SceneEntity duplicateRuntime(SceneEntity entity,
                                                      robot2D::vec2f position = robot2D::vec2f{}) = 0;
    };


    class ScriptInteractor: public IScriptInteractor, public IScriptInteractorFrom {
    public:
        ScriptInteractor(EditorInteractor::WeakPtr editorInteractor, ScriptingEngineService::Ptr scriptingEngine);
        ScriptInteractor(const ScriptInteractor& other) = delete;
        ScriptInteractor& operator=(const ScriptInteractor& other) = delete;
        ScriptInteractor(ScriptInteractor&& other) = delete;
        ScriptInteractor& operator=(ScriptInteractor&& other) = delete;
        ~ScriptInteractor() override = default;

        SceneEntity getEntity(std::uint64_t entityID) override;
        void loadSceneAsyncRuntime(std::string &&name) override;
        bool loadSceneRuntime(std::string &&name) override;
        void exitEngineRuntime() override;
        SceneEntity duplicateRuntime(editor::SceneEntity entity, robot2D::vec2f position = robot2D::vec2f{}) override;


        void setScriptClass(std::string name, UUID uuid) override;

        FieldMap& getFields(UUID uuid) override;

        bool hasEntityClass(const std::string& name) const override;

        std::vector<std::string> getClassesNames() const override;
    private:
        EditorInteractor::WeakPtr m_editorInteractor;
        ScriptingEngineService::Ptr m_scriptingEngine;

        std::unordered_map<UUID, FieldMap> m_fields;
    };

} // namespace editor