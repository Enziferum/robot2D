#pragma once

#include <imgui/imgui.h>

namespace editor {
    enum class EditorIcon { Move, Scale, Rotate, Grid, Snap, Frame, Folder, Image, Scene, Script,
                            Font, Prefab, Animation, File };
    // Original geometry in a 24-unit design grid; no font glyphs or external bitmap assets.
    void drawEditorIcon(ImDrawList& draw, EditorIcon icon, ImVec2 position, float size, ImU32 color);
}
