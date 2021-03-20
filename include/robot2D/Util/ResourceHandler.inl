
#include "ResourceHandler.h"

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


template<typename T, typename ID>
ResourceHandler<T, ID>::ResourceHandler() {

}


template<typename T, typename ID>
template<typename ... Args>
bool ResourceHandler<T, ID>::loadFromFile(const ID &idx, Args&& ... args) {
    Ptr resource = std::make_unique<T>();
    if(!resource->loadFromFile(std::forward<Args>(args)...))
        return false;

    if(m_resources.find(idx) != m_resources.end())
        return false;

    //todo change to emplace back
    m_resources.insert(std::pair<ID, Ptr>(idx,
                                          std::move(resource)));

    return true;
}

template<typename T, typename ID>
const T& ResourceHandler<T, ID>::get(const ID &idx) const {
    return *m_resources.at(idx);
}
