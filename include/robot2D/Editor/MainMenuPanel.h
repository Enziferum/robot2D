//
// Created by support on 26.03.2021.
//

#pragma once

#include "IPanel.h"

namespace robot2D {

    class MainMenuPanel: public IPanel{
    public:
        MainMenuPanel();
        ~MainMenuPanel() = default;

        void update(float dt);
    private:
        // callbacks //
        void onCreateScene();
        void onLoadScene();

        void onUndo();
        void onRedo();

        // callbacks //


        void setup();
        void create_panel();
    private:
    };
}
