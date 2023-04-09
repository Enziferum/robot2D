#pragma once

#include <vector>
#include "PopupConfiguration.hpp"

namespace editor {

    class PopupDelegate {
    public:
        using Ptr = PopupDelegate*;
        virtual ~PopupDelegate() = 0;
        virtual void onRender() = 0;
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
                m_activeDelegate -> onRender();
        }

        void addObserver(PopupObserver::Ptr ptr) {
            m_observers.emplace_back(ptr);
        }
    private:
        std::vector<PopupObserver::Ptr> m_observers;
        PopupDelegate::Ptr m_activeDelegate{nullptr};
    };
}