#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include <stdexcept>

namespace editor {

    namespace phys2d {

        constexpr int kMaxLayers = 16;

        struct LayerInfo {
            std::string name{};
            uint16_t    bit{};   // 1<<index
            int         index{}; // 0..15
        };

        class LayerRegistry {
        public:
            static LayerRegistry& I() { static LayerRegistry inst; return inst; }

            void defineLayer(int index, std::string name) {
                if (index < 0 || index >= kMaxLayers)
                    throw std::out_of_range("layer index");
                if (m_indexToName[index] != "" || m_nameToIndex.count(name))
                    throw std::runtime_error("layer exists");
                m_indexToName[index] = name;
                m_nameToIndex[name]  = index;
                m_layers[index]      = LayerInfo{name, (uint16_t)(1u << index), index};
            }

            uint16_t bit(const std::string& name) const { return (uint16_t)(1u << index(name)); }
            uint16_t bit(int index) const { return (uint16_t)(1u << index); }

            int index(const std::string& name) const {
                auto it = m_nameToIndex.find(name);
                if (it == m_nameToIndex.end()) throw std::runtime_error("unknown layer: " + name);
                return it->second;
            }

            const std::array<std::string, kMaxLayers>& names() const { return m_indexToName; }


            void setDefaultCollides(const std::string& a, const std::string& b, bool on) {
                int ia = index(a), ib = index(b);
                m_matrix[ia][ib] = m_matrix[ib][ia] = on;
            }

            bool defaultCollides(const std::string& a, const std::string& b) const {
                int ia = index(a), ib = index(b);
                return m_matrix[ia][ib];
            }

            uint16_t defaultMaskFor(const std::string& layer) const {
                int i = index(layer);
                uint16_t m = 0;
                for (int j = 0; j < kMaxLayers; ++j)
                    if (m_matrix[i][j]) m |= (uint16_t)(1u << j);
                return m;
            }

            uint16_t maskFrom(const std::vector<std::string>& layers) const {
                uint16_t m = 0;
                for (auto& n : layers) m |= bit(n);
                return m;
            }


            bool has(const std::string& name) const {
                return m_nameToIndex.count(name) != 0;
            }

            const LayerInfo& layerInfo(int idx) const {
                return m_layers[idx];
            }

        private:
            LayerRegistry() {
                m_indexToName.fill("");
                for (auto& row : m_matrix)
                    row.fill(false);
            }

            std::array<std::string, kMaxLayers>         m_indexToName;
            std::unordered_map<std::string,int>         m_nameToIndex;
            std::array<LayerInfo, kMaxLayers>           m_layers{};
            std::array<std::array<bool, kMaxLayers>, kMaxLayers> m_matrix;
        };

        struct FilterBits {
            uint16_t categoryBits = 1;
            uint16_t maskBits     = 0xFFFF;
            int16_t  groupIndex   = 0;
            std::string name;
        };

        inline FilterBits makeFilterDefault(const std::string& layerName, int16_t groupIndex = 0) {
            auto& reg = LayerRegistry::I();
            return FilterBits{
                    /*categoryBits*/ reg.bit(layerName),
                    /*maskBits*/     reg.defaultMaskFor(layerName),
                    /*groupIndex*/   groupIndex,
                    /*name*/         layerName
            };
        }

        inline FilterBits makeFilterCustomMask(const std::string& layerName,
                                               const std::vector<std::string>& collidesWith,
                                               int16_t groupIndex = 0)
        {
            auto& reg = LayerRegistry::I();
            return FilterBits{
                    reg.bit(layerName),
                    reg.maskFrom(collidesWith),
                    groupIndex
            };
        }

        inline int layerIndexFromCategoryBits(uint16_t categoryBits, bool assertSingleBit = true)
        {
            if (categoryBits == 0)
                return -1;

            if (assertSingleBit && (categoryBits & (categoryBits - 1)) != 0) {
            }

            for (int i = 0; i < kMaxLayers; ++i)
                if (categoryBits & (uint16_t(1u) << i))
                    return i;

            return -1;
        }

        inline std::string layerNameFromCategoryBit(uint16_t categoryBits)
        {
            int idx = layerIndexFromCategoryBits(categoryBits, /*assertSingleBit*/false);
            if (idx < 0) return {};

            const auto& names = LayerRegistry::I().names();
            if (idx >= 0 && idx < kMaxLayers && !names[idx].empty())
                return names[idx];

            return {};
        }

    } // namespace phys2d

    inline void InitPhysicsLayers()
    {
        auto& LR = phys2d::LayerRegistry::I();

        LR.defineLayer(0, "Default");
        LR.defineLayer(1, "Player");
        LR.defineLayer(2, "Ground");
        LR.defineLayer(3, "Wall");
        LR.defineLayer(4, "OneWay");
        LR.defineLayer(5, "Enemy");
        LR.defineLayer(6, "Projectile");
        LR.defineLayer(7, "Trigger");

        auto on  = [&](const char* a, const char* b){ LR.setDefaultCollides(a,b,true);  };
        auto off = [&](const char* a, const char* b){ LR.setDefaultCollides(a,b,false); };

        // Базово: всё с Default
        for (auto& n : LR.names())
            if (!n.empty())
                on("Default", n.c_str());

        // Player ↔ Ground/Wall/OneWay/Enemy/Collectible
        on("Player","Ground"); on("Player","Wall"); on("Player","OneWay"); on("Player","Enemy");

        // Projectile ↔ Ground/Wall/Enemy (без Player по умолчанию)
        on("Projectile","Ground"); on("Projectile","Wall"); on("Projectile","Enemy");

        // OneWay ↔ Player/Enemy
        on("OneWay","Player"); on("OneWay","Enemy");

        for (auto& n : LR.names())
            if (!n.empty())
                off("Trigger", n.c_str());
    }

} // namespace editor