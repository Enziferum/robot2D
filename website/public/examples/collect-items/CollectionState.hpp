// Core lesson 07. Zlib license; see LICENSE.txt.
#pragma once

#include "../sprite-collisions/Collision.hpp"
#include <array>
#include <string>

class CollectionState {
public:
    static constexpr unsigned itemCount = 5;
    static constexpr unsigned pointsPerItem = 10;

    CollectionState() { reset(); }
    void reset() { m_active.fill(true); }
    bool active(unsigned index) const { return m_active.at(index); }

    bool collect(unsigned index) {
        if(!m_active.at(index)) return false;
        m_active[index] = false; // Mark consumed before reporting the pickup.
        return true;
    }

    unsigned collected() const {
        return static_cast<unsigned>(std::count(m_active.begin(), m_active.end(), false));
    }
    unsigned score() const { return collected() * pointsPerItem; }
    bool complete() const { return collected() == itemCount; }
    std::string label() const {
        return "Score: " + std::to_string(score()) + "    Items: " +
            std::to_string(collected()) + "/" + std::to_string(itemCount);
    }

private:
    std::array<bool, itemCount> m_active{};
};

inline unsigned collectOverlapping(robot2D::Sprite& player,
    std::array<robot2D::Sprite, CollectionState::itemCount>& items, CollectionState& state) {
    const auto playerBox = quadBounds(player);
    unsigned picked = 0;
    for(unsigned i = 0; i < items.size(); ++i) {
        if(state.active(i) && overlaps(playerBox, quadBounds(items[i]))) {
            if(state.collect(i)) ++picked;
        }
    }
    return picked;
}
