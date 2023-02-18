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

#include <robot2D/Engine.hpp>
#include <sandbox/Sandbox.hpp>

template<typename Collection, typename StartIter, typename EndIter, typename Pred>
Collection filter(StartIter startIter, EndIter endIter, Pred pred) {
    Collection coll{};

    while(startIter != endIter) {
        if(pred(*startIter))
            coll.push_back(*startIter);
        ++startIter;
    }

    return coll;
}

template<typename Collection, typename Pred,
        typename = std::enable_if_t<std::is_default_constructible_v<Collection>>>
Collection filter(const Collection& collection, Pred pred) {
    Collection coll{};

    auto startIter = std::begin(collection);
    auto endIter = std::end(collection);

    while(startIter != endIter) {
        if(pred(*startIter))
            coll.push_back(*startIter);
        ++startIter;
    }

    return coll;
}

#define DECAY(value) decltype(std::decay_t<typeof(value)>())

int run() {
    std::vector<int> vec{1, 2, 3, 4};

    auto filteredCollection = filter(vec, [](int value) {
        return value % 2 == 0;
    });

    for(const auto& i: filteredCollection)
        std::cout << i << "\n";

    return 0;
}

ROBOT2D_MAIN(Sandbox)