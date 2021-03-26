//
// Created by support on 26.03.2021.
//

#pragma once
#include <unordered_map>
#include "IPanel.h"

namespace robot2D {

    // need scan for entity system //

    /*
     * auto list = entity.getComponentList();
     */

    class ObjectComponentPanel{
    public:
        using Ptr = std::shared_ptr<ObjectComponentPanel>;
    public:
        ObjectComponentPanel();
        virtual ~ObjectComponentPanel() = 0;


        virtual void update(void* entity, float dt) = 0;
    public:
        void* m_entity;
    };

    //mini PanelManager
    class ObjectPanel: public IPanel{
    public:
        ObjectPanel();
        ~ObjectPanel() = default;

        void handleEvents(const Event &event) override;

        void update(float dt) override;

    private:
        // scaning entity for components
        void scanEntity();

        // main panel with sub_panels
        // auto list = entity.getComponents();
        // sortListByLogic();
        // imgui create main panel
        // works for default panels
        // for(auto& it: list) {
        //      it->update(dt, entity);
        // }
        // imgui create main panel
        void create_panel();
        void setup_components();
    private:
        std::unordered_map<int32_t,
                            ObjectComponentPanel::Ptr> m_components;
    };
}
