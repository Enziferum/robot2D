//
// Created by support on 26.03.2021.
//

#pragma once

#include <string>

namespace robot2D{
    class EditorScene final {
    public:
        EditorScene();
        ~EditorScene();


        bool load(const std::string& path);
        bool save(const std::string& path);
    private:
        int32_t m_id;
    };
}