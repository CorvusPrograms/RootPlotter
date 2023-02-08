#include "annotations.h"
#include <TLatex.h>

#include "plotting.h"

namespace rootp {
void addText(DrawPad& dp, int subpad, const CanvasText& text) {
    auto pad = dp.pad.get();
    pad->cd(subpad);
    TLatex l;
    l.SetTextSize(text.size);
    l.SetTextAngle(text.angle);
    l.DrawLatexNDC(text.x, text.y, text.text.c_str());
}

}  // namespace rootp
