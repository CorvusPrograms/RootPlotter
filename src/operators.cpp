#include "operators.h"
#include <TH1D.h>

namespace rootp {
namespace operators {
    
PlotData add(const PlotData& d1, const PlotData& d2) {
    auto sum_hist = std::shared_ptr<TH1>(static_cast<TH1*>(d1.hist->Clone()));
    sum_hist->Add(d2.hist.get());
    PlotData ret = d1;
    ret.hist = sum_hist;
    return ret;
}

}  // namespace operators

}  // namespace rootp
