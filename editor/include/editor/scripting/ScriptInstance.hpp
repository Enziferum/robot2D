/*********************************************************************
(c) Alex Raag 2023
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
#include <editor/SceneEntity.hpp>
#include "MonoClassWrapper.hpp"

extern "C" {
    typedef struct _MonoClass MonoClass;
    typedef struct _MonoObject MonoObject;
    typedef struct _MonoMethod MonoMethod;
    typedef struct _MonoAssembly MonoAssembly;
    typedef struct _MonoImage MonoImage;
    typedef struct _MonoClassField MonoClassField;
}

namespace editor {

    class ScriptEngineData;
    class ScriptInstance {
    public:
        using Ptr = std::shared_ptr<ScriptInstance>;
    public:
        ScriptInstance(ScriptEngineData* data, MonoClassWrapper::Ptr, SceneEntity sceneEntity);
        ~ScriptInstance() = default;

        MonoClassWrapper::Ptr getClassWrapper() { return m_class; }

        template<typename T>
        T getFieldValue(const std::string& name)
        {
            static_assert(sizeof(T) <= 64, "Type too large!");

            bool success = getFieldValueInternal(name, s_FieldValueBuffer);
            if (!success)
                return T();

            return *(T*)s_FieldValueBuffer;
        }

        template<typename T>
        void setFieldValue(const std::string& name, T value)
        {
            static_assert(sizeof(T) <= 64, "Type too large!");

            setFieldValueInternal(name, &value);
        }

        bool getFieldValueInternal(const std::string& name, void* buffer);
        bool setFieldValueInternal(const std::string& name, const void* value);

        void onCreateInvoke();
        void onUpdateInvoke(float dt);
    private:
        MonoClassWrapper::Ptr m_class;
        SceneEntity m_entity;

        inline static char s_FieldValueBuffer[64];
    };
} // namespace editor