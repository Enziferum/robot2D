#include <editor/ResourceHolder.hpp>

namespace editor {

    ResourceHolder& ResourceHolder::getInstance() {
        static ResourceHolder holder;
        return holder;
    }

    ResourceHolder::ResourceHolder() {

    }

    ResourceHolder::~ResourceHolder() {

    }

    bool ResourceHolder::load(const std::string& path) {
        return true;
    }

    bool ResourceHolder::loadThread(const std::string &path) {
        return true;
    }
}