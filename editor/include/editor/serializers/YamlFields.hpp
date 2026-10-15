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

#pragma once

#include <cmath>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <yaml-cpp/yaml.h>

namespace editor::yaml {

    inline YAML::Node required(const YAML::Node& node, const char* name) {
        if(!node.IsMap())
            throw YAML::RepresentationException(node.Mark(), "Expected a map for field '" + std::string{name} + "'");
        const auto value = node[name];
        if(!value || value.IsNull())
            throw YAML::RepresentationException(node.Mark(), "Missing field '" + std::string{name} + "'");
        return value;
    }

    template<typename T>
    T read(const YAML::Node& node, const char* name) {
        const auto value = required(node, name);
        try {
            auto result = value.as<T>();
            if constexpr(std::is_floating_point_v<T>) {
                if(!std::isfinite(result))
                    throw YAML::RepresentationException(value.Mark(), "Expected a finite number");
            }
            return result;
        }
        catch(const YAML::Exception& error) {
            throw YAML::RepresentationException(value.Mark(), "Field '" + std::string{name} + "': " + error.what());
        }
    }

    template<typename T>
    T optional(const YAML::Node& node, const char* name, const T& fallback) {
        return node[name] ? read<T>(node, name) : fallback;
    }

    template<typename T>
    void value(YAML::Emitter& out, const T& data) {
        if constexpr(std::is_floating_point_v<T>) {
            if(!std::isfinite(data))
                throw YAML::RepresentationException(YAML::Mark::null_mark(), "Cannot write a non-finite number");
        }
        out << data;
    }

    template<typename T>
    void write(YAML::Emitter& out, const char* name, const T& data) {
        out << YAML::Key << name << YAML::Value;
        value(out, data);
    }

    inline void validateKeys(const YAML::Node& node, bool legacyEntityKey = false, unsigned depth = 0) {
        if(depth > 128)
            throw YAML::RepresentationException(node.Mark(), "YAML nesting is too deep or contains a recursive alias");
        if(node.IsMap()) {
            std::unordered_map<std::string, YAML::Node> keys;
            for(const auto& item: node) {
                if(!item.first.IsScalar())
                    throw YAML::RepresentationException(item.first.Mark(), "Map keys must be scalar values");
                const auto key = item.first.as<std::string>();
                const auto inserted = keys.emplace(key, item.second);
                if(!inserted.second) {
                    // Older entity writers emitted this identical UUID field twice.
                    if(!(legacyEntityKey && key == "Entity" && item.second.IsScalar() &&
                         inserted.first -> second.IsScalar() &&
                         item.second.Scalar() == inserted.first -> second.Scalar()))
                        throw YAML::RepresentationException(item.first.Mark(), "Duplicate field '" + key + "'");
                }
                validateKeys(item.second, legacyEntityKey, depth + 1);
            }
        }
        else if(node.IsSequence()) {
            for(const auto& item: node)
                validateKeys(item, legacyEntityKey, depth + 1);
        }
    }
}
