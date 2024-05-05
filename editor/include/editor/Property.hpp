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

#include <functional>
#include "Macro.hpp"

namespace editor {

#define PROPERTY_GET(name) auto get_##name(const decltype(name)::Type& value)
#define PROPERTY_SET(name) void set_##name(const decltype(name)::Type& oldValue, \
            const decltype(name)::Type& newValue)

#define PROPERTY_BIND_GETTER(name) name.bind_getter(BIND_CLASS_FN(get_##name));
#define PROPERTY_BIND_SETTER(name) name.bind_setter(BIND_CLASS_FN(set_##name));


    enum class PropertyMode {
        Both,
        Write,
        Read
    };
    template<typename T, PropertyMode = PropertyMode::Both>
    class Property{};

    template<typename T>
    class Property<T, PropertyMode::Both> {
    public:
        static_assert(!std::is_same_v<T, void> && "Property can't hold void");

        using Type = T;
        using Getter = std::function<const T&(const T&)>;
        using Setter = std::function<void(const T&, const T&)>;

        Property():m_data{}{}
        Property(const T& data): m_data{data}{}
        Property(T&& data): m_data{std::move(data)}{}
        ~Property() = default;

        operator T() {
            if(m_getter)
                return m_getter(m_data);
            return m_data;
        }

        operator T&() {
            if(m_getter)
                return m_getter(m_data);
            return m_data;
        }

        operator T&() const {
            if(m_getter)
                return m_getter(m_data);
            return m_data;
        }

        Property& operator=(const T& data) {
            if(m_setter)
                m_setter(m_data, data);
            m_data = data;
            return *this;
        }

        void bind_getter(Getter&& getter) {
            m_getter = std::move(getter);
        }

        void bind_setter(Setter&& setter) {
            m_setter = std::move(setter);
        }
    private:
        T m_data;
        Getter m_getter{nullptr};
        Setter m_setter{nullptr};
    };
}