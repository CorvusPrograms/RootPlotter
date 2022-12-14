#pragma once

#include <optional>

namespace rootp {
struct Style {
    enum Mode {
        None = 0,
        Line = 1 << 0,
        Marker = 1 << 1,
        Fill = 1 << 2,
    };

    Mode mode = Mode::Marker;

    using StyleId_t = int;
    std::optional<int> palette_idx = std::nullopt;
    std::optional<int> color = std::nullopt;
    std::optional<StyleId_t> marker_style = std::nullopt;
    std::optional<float> marker_size = std::nullopt;
    std::optional<StyleId_t> line_style = std::nullopt;
    std::optional<int> line_width = std::nullopt;
    std::optional<StyleId_t> fill_style = std::nullopt;
};

}  // namespace rootp
