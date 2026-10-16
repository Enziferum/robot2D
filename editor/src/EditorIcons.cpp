#include <editor/EditorIcons.hpp>
#include <algorithm>

namespace editor {
    void drawEditorIcon(ImDrawList& draw, EditorIcon icon, ImVec2 position, float size, ImU32 color) {
        if(size <= 0.f)
            return;
        const float scale = size / 24.f;
        const float stroke = std::max(1.f, 1.7f * scale);
        const auto point = [&](float x, float y) { return ImVec2{position.x + x * scale, position.y + y * scale}; };
        const auto line = [&](float x, float y, float a, float b) {
            draw.AddLine(point(x, y), point(a, b), color, stroke);
        };
        const auto rect = [&](float x, float y, float a, float b, float rounding = 1.f) {
            draw.AddRect(point(x, y), point(a, b), color, rounding * scale, 0, stroke);
        };
        const auto arrow = [&](float x, float y, float dx, float dy) {
            line(x, y, x - dx - dy, y - dy + dx);
            line(x, y, x - dx + dy, y - dy - dx);
        };
        switch(icon) {
        case EditorIcon::Move:
            line(3, 12, 21, 12); line(12, 3, 12, 21);
            arrow(3, 12, -3, 0); arrow(21, 12, 3, 0); arrow(12, 3, 0, -3); arrow(12, 21, 0, 3); break;
        case EditorIcon::Scale:
            rect(4, 10, 14, 20); line(9, 15, 21, 3); line(14, 3, 21, 3); line(21, 3, 21, 10); break;
        case EditorIcon::Rotate:
            draw.PathArcTo(point(12, 12), 8.f * scale, -2.7f, 1.6f, 28);
            draw.PathStroke(color, 0, stroke); line(4, 4, 4, 10); line(4, 10, 10, 10); break;
        case EditorIcon::Grid:
            rect(3, 3, 21, 21); line(9, 3, 9, 21); line(15, 3, 15, 21); line(3, 9, 21, 9); line(3, 15, 21, 15); break;
        case EditorIcon::Snap:
            draw.PathArcTo(point(12, 12), 7.f * scale, 0.f, 3.14159265f, 20);
            draw.PathStroke(color, 0, stroke * 2.f); line(5, 12, 5, 4); line(19, 12, 19, 4);
            line(3, 4, 7, 4); line(17, 4, 21, 4); break;
        case EditorIcon::Frame:
            line(3, 9, 3, 3); line(3, 3, 9, 3); line(15, 3, 21, 3); line(21, 3, 21, 9);
            line(3, 15, 3, 21); line(3, 21, 9, 21); line(15, 21, 21, 21); line(21, 21, 21, 15);
            draw.AddCircle(point(12, 12), 2.f * scale, color, 12, stroke); break;
        case EditorIcon::Folder:
            line(3, 7, 3, 20); line(3, 20, 21, 20); line(21, 20, 21, 7); line(21, 7, 11, 7);
            line(11, 7, 9, 4); line(9, 4, 3, 4); line(3, 4, 3, 7); line(3, 10, 21, 10); break;
        case EditorIcon::Image:
            rect(3, 3, 21, 21); draw.AddCircle(point(8, 8), 2.f * scale, color, 12, stroke);
            line(4, 19, 10, 13); line(10, 13, 14, 17); line(14, 17, 17, 12); line(17, 12, 21, 17); break;
        case EditorIcon::Scene:
            rect(9, 3, 15, 9); rect(3, 15, 9, 21); rect(15, 15, 21, 21);
            line(12, 9, 12, 12); line(6, 12, 18, 12); line(6, 12, 6, 15); line(18, 12, 18, 15); break;
        case EditorIcon::Script:
            line(8, 6, 2, 12); line(2, 12, 8, 18); line(16, 6, 22, 12); line(22, 12, 16, 18); line(14, 4, 10, 20);
            break;
        case EditorIcon::Font:
            line(4, 20, 12, 4); line(12, 4, 20, 20); line(7, 14, 17, 14); break;
        case EditorIcon::Prefab:
            line(12, 2, 22, 7); line(22, 7, 22, 17); line(22, 17, 12, 22); line(12, 22, 2, 17);
            line(2, 17, 2, 7); line(2, 7, 12, 2); line(2, 7, 12, 12); line(22, 7, 12, 12); line(12, 12, 12, 22); break;
        case EditorIcon::Animation:
            rect(3, 3, 21, 21); line(7, 3, 7, 21); line(17, 3, 17, 21);
            for(float y: {7.f, 12.f, 17.f}) {
                line(3, y, 7, y); line(17, y, 21, y);
            }
            draw.AddTriangleFilled(point(10, 8), point(15, 12), point(10, 16), color); break;
        case EditorIcon::File:
            line(4, 2, 15, 2); line(15, 2, 20, 7); line(20, 7, 20, 22); line(20, 22, 4, 22); line(4, 22, 4, 2);
            line(15, 2, 15, 7); line(15, 7, 20, 7); line(8, 12, 16, 12); line(8, 16, 16, 16); break;
        }
    }
}
