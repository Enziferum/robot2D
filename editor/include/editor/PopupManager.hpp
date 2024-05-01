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

#include <vector>
#include "PopupConfiguration.hpp"

namespace editor {

    class PopupDelegate {
    public:
        using Ptr = PopupDelegate*;
        virtual ~PopupDelegate() = 0;
        virtual void onPopupRender() = 0;
    };

    class PopupObserver {
    public:
        using Ptr = PopupObserver*;
        virtual ~PopupObserver() = 0;

        virtual void onBeginPopup() = 0;
        virtual void onEndPopup() = 0;
    };

    class PopupManager {
    public:
        static PopupManager* getManager() {
            static PopupManager manager;
            return &manager;
        }

        void beginPopup(PopupDelegate::Ptr delegate, PopupConfiguration* configuration = nullptr) {
            m_activeDelegate = delegate;
            for(auto& observer: m_observers)
                if(observer)
                    observer -> onBeginPopup();
        }

        void endPopup() {
            m_activeDelegate = nullptr;
            for(auto& observer: m_observers)
                if(observer)
                    observer -> onEndPopup();
        }

        void onRender() {
            if(m_activeDelegate)
                m_activeDelegate -> onPopupRender();
        }

        void addObserver(PopupObserver::Ptr ptr) {
            m_observers.emplace_back(ptr);
        }
    private:
        std::vector<PopupObserver::Ptr> m_observers;
        PopupDelegate::Ptr m_activeDelegate{nullptr};
    };

} // namespace editor