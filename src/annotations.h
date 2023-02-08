#pragma once
#include <string>

namespace rootp {
struct CanvasText {
    std::string text = "";
    float x = 0.0f, y = 0.0f;
    float size = 0.025f;
    float angle = 0.0f;
    CanvasText() = default;
    CanvasText(const std::string& s, float ix, float iy)
        : text{s}, x{ix}, y{iy} {}
};

struct DrawPad;
void addText(DrawPad& dp, int subpad, const CanvasText& t);

}  // namespace rootp
