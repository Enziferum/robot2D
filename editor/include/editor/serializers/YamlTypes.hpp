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

#include <cmath>
#include <yaml-cpp/yaml.h>
#include <robot2D/Core/Vector2.hpp>
#include <robot2D/Core/Vector3.hpp>
#include <robot2D/Graphics/Color.hpp>
#include <editor/Uuid.hpp>

namespace YAML {
    template<>
    struct convert<editor::UUID>
    {
        static Node encode(const editor::UUID& uuid)
        {
            Node node;
            node = static_cast<uint64_t>(uuid);
            return node;
        }

        static bool decode(const Node& node, editor::UUID& uuid)
        {
            if(!node.IsScalar()) return false;
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
            if(!std::isfinite(rhs.x)) return false;
            rhs.y = node[1].as<float>();
            if(!std::isfinite(rhs.y)) return false;
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
            if(!std::isfinite(rhs.x)) return false;
            rhs.y = node[1].as<float>();
            if(!std::isfinite(rhs.y)) return false;
            rhs.z = node[2].as<float>();
            if(!std::isfinite(rhs.z)) return false;
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
            return std::isfinite(rhs.red) && std::isfinite(rhs.green) && std::isfinite(rhs.blue) && std::isfinite(rhs.alpha);
        }
    };
    inline YAML::Emitter& operator<<(YAML::Emitter& out, const robot2D::vec2f& value) {
        if(!std::isfinite(value.x) || !std::isfinite(value.y))
            throw YAML::RepresentationException(YAML::Mark::null_mark(), "Cannot write a non-finite vec2");
        out << YAML::Flow;
        out << YAML::BeginSeq << value.x << value.y << YAML::EndSeq;
        return out;
    }

    inline YAML::Emitter& operator<<(YAML::Emitter& out, const robot2D::vec3f& value) {
        if(!std::isfinite(value.x) || !std::isfinite(value.y) || !std::isfinite(value.z))
            throw YAML::RepresentationException(YAML::Mark::null_mark(), "Cannot write a non-finite vec3");
        out << YAML::Flow;
        out << YAML::BeginSeq << value.x << value.y << value.z << YAML::EndSeq;
        return out;
    }

    inline YAML::Emitter& operator<<(YAML::Emitter& out, const robot2D::Color& value) {
        if(!std::isfinite(value.red) || !std::isfinite(value.green) ||
           !std::isfinite(value.blue) || !std::isfinite(value.alpha))
            throw YAML::RepresentationException(YAML::Mark::null_mark(), "Cannot write a non-finite color");
        out << YAML::Flow;
        out << YAML::BeginSeq << value.red << value.green
            << value.blue << value.alpha << YAML::EndSeq;
        return out;
    }



}
