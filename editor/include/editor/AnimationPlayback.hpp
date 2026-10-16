#pragma once

#include <string>
#include <editor/SceneEntity.hpp>

namespace editor {

    // Shared entry points for editor preview and managed calls. Failure preserves playback.
    bool playAnimation(SceneEntity entity, const std::string& name);
    bool stopAnimation(SceneEntity entity, const std::string& name = {});

}
