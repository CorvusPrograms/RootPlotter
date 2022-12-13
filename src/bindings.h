#pragma once

namespace sol {
class state;
}

namespace rootp {
void bindMarkerStyles(sol::state &lua);
void bindLineStyles(sol::state &lua);
void bindFillStyles(sol::state &lua);
void bindPalettes(sol::state &lua);

void bindDataOps(sol::state &lua);
void bindPlotting(sol::state &lua);

}  // namespace rootp
