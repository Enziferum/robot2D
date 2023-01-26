/*********************************************************************
(c) Alex Raag 2021
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

#include <unordered_map>
#include <string>
#include <memory>
#include <type_traits>

#include <robot2D/Config.hpp>

namespace robot2D{
    template<typename T, typename ID = std::string>
    class ROBOT2D_EXPORT_API ResourceHandler {
    public:
        using Ptr = std::unique_ptr<T>;
    public:
        ResourceHandler() = default;
        ResourceHandler(const ResourceHandler& other) = delete;
        ResourceHandler& operator=(const ResourceHandler& other) = delete;
        ResourceHandler(ResourceHandler&& other) = delete;
        ResourceHandler& operator=(ResourceHandler&& other) = delete;
        ~ResourceHandler() = default;

        template<typename ... Args>
        bool loadFromFile(const ID& idx, Args&&... args);

        template<typename = std::enable_if_t<std::is_move_assignable_v<T>>>
        bool add(const ID& idx, T&& resource) {
            auto Resource = std::make_unique<T>(std::move(resource));
            return m_resources.insert(std::pair<ID, Ptr>(idx,
                                                  std::move(Resource))).second;
        }

        T& operator[](ID id) { return m_resources[id]; }
        T& get(const ID& idx);
        const T& get(const ID& idx) const;
    private:
        std::unordered_map<ID, Ptr> m_resources{};
    };

    #include "ResourceHandler.inl"
}
