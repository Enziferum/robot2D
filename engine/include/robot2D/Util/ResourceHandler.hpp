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

#include <robot2D/Config.hpp>

namespace robot2D{
    template<typename T, typename ID = std::string>
    class ROBOT2D_EXPORT_API ResourceHandler{
    public:
        using Ptr = std::unique_ptr<T>;
    public:
        ResourceHandler();
        ~ResourceHandler() = default;

        template<typename ... Args>
        bool loadFromFile(const ID& idx, Args&&... args);

        bool append(const ID& idx, T resource) {
            auto Resource = std::make_unique<T>(std::move(resource));
            m_resources.insert(std::pair<ID, Ptr>(idx,
                                                  std::move(Resource)));
        }

        T& get(const ID& idx);
        const T& get(const ID& idx) const;
    private:
        std::unordered_map<ID, Ptr> m_resources;
    };

    #include "ResourceHandler.inl"
}
