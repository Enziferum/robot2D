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
#include <utility>
#include <cstddef>

#include <robot2D/Config.hpp>

namespace robot2D {
    /**
     * @brief Owns resources indexed by unique identifiers.
     * @details Loading is synchronous. Duplicate IDs never replace resources.
     * Insertion preserves pointers and references; clear() or destruction invalidates them.
     * Concurrent access involving a mutation requires external synchronization.
     * Resource operations may impose additional requirements, such as a current GL context.
     * @tparam T Owned resource type.
     * @tparam ID Hashable identifier compatible with std::unordered_map.
     */
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

        /**
         * @brief Default-constructs and loads a resource under an unused ID.
         * @param idx Unique resource identifier.
         * @param args Arguments forwarded to T::loadFromFile.
         * @return False for an occupied ID or a loading failure; true on insertion.
         * @details An occupied ID is checked before constructing or loading T.
         * A failed load leaves the handler unchanged. Exceptions propagate to the caller.
         */
        template<typename ... Args>
        bool loadFromFile(const ID& idx, Args&&... args);

        /// @brief Returns whether the identifier is registered, without inserting it.
        bool has(const ID& key) const {
            return m_resources.find(key) != m_resources.end();
        }

        /**
         * @brief Move-constructs a resource under an unused ID.
         * @return False for an occupied ID, leaving resource untouched.
         * @details Requires move construction, not move assignment. Exceptions propagate.
         */
        bool add(const ID& idx, T&& resource) {
            static_assert(std::is_move_constructible_v<T>, "Resource must be move constructible");
            if(has(idx))
                return false;

            return m_resources.emplace(idx, std::make_unique<T>(std::move(resource))).second;
        }

        /**
         * @brief Default-constructs a resource under an unused ID.
         * @return Owned resource pointer, or nullptr for an occupied ID.
         * @details Duplicate IDs do not construct T. Exceptions propagate.
         */
        T* add(const ID& id) {
            static_assert(std::is_default_constructible_v<T>, "Resource must be default constructible");
            if(has(id))
                return nullptr;

            auto result = m_resources.emplace(id, std::make_unique<T>());
            return result.first -> second.get();
        }

        /// @brief Returns a resource without insertion; throws std::out_of_range if absent.
        T& operator[](const ID& id) {
            return get(id);
        }
        /// @brief Const access without insertion; throws std::out_of_range if absent.
        const T& operator[](const ID& id) const {
            return get(id);
        }

        /// @brief Returns an owned reference; throws std::out_of_range if idx is absent.
        T& get(const ID& idx) {
            return *m_resources.at(idx);
        }
        /// @brief Returns an owned const reference; throws std::out_of_range if idx is absent.
        const T& get(const ID& idx) const;

        /// @brief Returns an owned pointer, or nullptr if idx is absent; never inserts.
        T* find(const ID& idx) {
            auto it = m_resources.find(idx);
            return it == m_resources.end() ? nullptr : it -> second.get();
        }
        /// @brief Returns an owned const pointer, or nullptr if idx is absent; never inserts.
        const T* find(const ID& idx) const {
            auto it = m_resources.find(idx);
            return it == m_resources.end() ? nullptr : it -> second.get();
        }

        /// @brief Returns the number of registered resources.
        std::size_t size() const noexcept {
            return m_resources.size();
        }
        /// @brief Returns whether no resources are registered.
        bool empty() const noexcept {
            return m_resources.empty();
        }
        /// @brief Releases all owned resources. Resource-specific context rules still apply.
        void clear() {
            m_resources.clear();
        }
    private:
        std::unordered_map<ID, Ptr> m_resources{};
    };

    #include "ResourceHandler.inl"
}
