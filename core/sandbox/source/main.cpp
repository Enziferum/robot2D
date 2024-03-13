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

template <class BidirectionalIterator, class T> inline
BidirectionalIterator find_first_if(const BidirectionalIterator first, const BidirectionalIterator last, T&& predicate)
{
    for (BidirectionalIterator it = first; it != last;)
        //reverse iteration: 1. check 2. decrement 3. evaluate
    {
        if (predicate(*it))
            return it;
        ++it; //
    }
    return first;
}

template <class BidirectionalIterator, class T> inline
BidirectionalIterator find_last_if(const BidirectionalIterator first, const BidirectionalIterator last, T&& predicate)
{
    for (BidirectionalIterator it = last; it != first;)
        //reverse iteration: 1. check 2. decrement 3. evaluate
    {
        --it; //

        if (predicate(*it))
            return it;
    }
    return last;
}

struct Item {
    int index;
    friend std::ostream& operator <<(std::ostream&, const Item& item);
};

std::ostream& operator <<(std::ostream& os, const Item& item) {
    os << item.index;
    return os;
}

int run() {
    std::vector<Item> vec{Item{1}, Item{2}, Item{3}, Item{4}};

    auto sourceItem = vec[0];
    auto targetItem = vec[3];


    auto sourceIter = std::find_if(vec.begin(), vec.end(), [&sourceItem](const Item& item) {
        return item.index == sourceItem.index;
    });

    auto targetIter = std::find_if(vec.begin(), vec.end(), [&targetItem](const Item& item) {
        return item.index == targetItem.index;
    });

    auto sourceOldDistance = std::distance(vec.begin(), sourceIter);
    auto targetOldDistance = std::distance(vec.begin(), targetIter);

    if(sourceOldDistance > targetOldDistance) {
        /// insert before remove last
        vec.insert(targetIter, sourceItem);
        vec.erase(find_last_if(vec.begin(), vec.end(), [&sourceItem](const Item& item) {
            return item.index == sourceItem.index;
        }), vec.end());
    }
    else if (sourceOldDistance < targetOldDistance) {
        vec.insert(targetIter, sourceItem);
        vec.erase(find_first_if(vec.begin(), vec.end(), [&sourceItem](const Item& item) {
            return item.index == sourceItem.index;
        }));
    }


    for(auto i: vec)
        std::cout << i << " ";


    return 0;
}

int main() {
    robot2D::EngineConfiguration engineConfiguration{};
    engineConfiguration.windowTitle = "Sandbox";
    engineConfiguration.windowSize = { 1920, 1080 };
    //return run();
    ROBOT2D_RUN_ENGINE(Sandbox, engineConfiguration);
}