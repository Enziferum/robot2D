//
// Created by support on 26.03.2021.
//

#include <vector>

#include <robot2D/Editor/ObjectPanel.h>

namespace robot2D{

    ObjectPanel::ObjectPanel() {

    }

    void ObjectPanel::scanEntity() {

    }

    void ObjectPanel::create_panel() {

    }

    void ObjectPanel::update(float dt) {
        void* m_entity;
        void* entity;
        //got components
        auto list = std::vector<int32_t>();
        // todo sort
        // object panel imgui begin //
        for(auto& it: list) {
            auto found = m_components.find(it);
            if(found == m_components.end())
                continue;
            //will create panels
            found->second->update(entity, dt);
        }

        // object panel imgui end //
    }

    void ObjectPanel::setup_components() {
        int32_t mask;
        m_components.insert(std::pair<int32_t, ObjectComponentPanel::Ptr>
                                    (mask, nullptr));

    }

    void ObjectPanel::handleEvents(const Event &event) {

    }



}

