#pragma once
#include <optional>

#include "TColor.h"
#include "data.h"

struct DataSource;
class TH1;
class TVirtualPad;
class TLegend;

using Pad = TVirtualPad;

struct PlotElement {
    std::optional<std::pair<float, float>> xrange = std::nullopt,
                                           yrange = std::nullopt;
    //  std::string xlabel, ylabel;
    virtual void addToLegend(TLegend *legend) = 0;
    virtual TH1 *getHistogram() = 0;
    virtual TH1 *getTotals() = 0;
    virtual float getIntegral() = 0;
    virtual TAxis *getXAxis() = 0;
    virtual TAxis *getYAxis() = 0;
    virtual void Draw(const std::string &s) = 0;
    virtual std::string to_string() const = 0;

    virtual float getMinDomain() = 0;
    virtual float getMaxDomain() = 0;

    virtual void setMinRange(float v) = 0;
    virtual void setMaxRange(float v) = 0;
    virtual float getMinRange() const = 0;
    virtual float getMaxRange() const = 0;

    virtual void setFillAtt(TAttFill *) {};
    virtual void setMarkAtt(TAttMarker *) {};
    virtual void setLineAtt(TAttLine *) {};
    virtual void setTitle(const std::string &s) = 0;
    virtual void setupRanges() = 0;

    virtual void setRangeX(const std::pair<float, float> &range) {
        xrange = range;
    }
    virtual void setRangeY(const std::pair<float, float> &range) {
        yrange = range;
    }
    virtual std::optional<std::pair<float, float>> getRangeX() const {
        return xrange;
    }
    virtual std::optional<std::pair<float, float>> getRangeY() const {
        return yrange;
    }
    virtual std::string getName() const = 0;
    virtual std::string getSourceID() const = 0;

    //   virtual void setXLabel(const std::string &s) { xlabel = s; }
    //   virtual std::string setXLabel(const std::string &s) const { return
    // xlabel; }
    //   virtual void setYLabel(const std::string &s) { ylabel = s; }
    //   virtual std::string setYLabel(const std::string &s) const { return
    // ylabel; }
    virtual ~PlotElement() = default;
};

struct Histogram : public PlotElement {
    DataSource *source;
    TH1 *hist;
    Histogram(DataSource *s, TH1 *h);
    std::string getSourceID() const;
    virtual void addToLegend(TLegend *legend);
    virtual void setupRanges();
    virtual TH1 *getHistogram();
    virtual TH1 *getTotals();
    virtual float getIntegral();
    virtual TAxis *getXAxis();
    virtual TAxis *getYAxis();
    virtual void setTitle(const std::string &s);
    virtual void Draw(const std::string &s);
    virtual std::string getName() const;
    void setFillStyle();
    void setMarkerStyle();
    void setLineStyle();
    virtual float getMinDomain();
    virtual float getMaxDomain();
    virtual void setMinRange(float v);
    virtual void setMaxRange(float v);

    virtual float getMinRange() const;
    virtual float getMaxRange() const;
    virtual std::string to_string() const;
    virtual void setFillAtt(TAttFill *fill_att);
    virtual void setMarkAtt(TAttMarker *mark_att);
    virtual void setLineAtt(TAttLine *line_att);
    virtual ~Histogram() = default;
};

struct Stack : public PlotElement {
    std::vector<DataSource *> sources;
    THStack *hist;
    Stack(const std::vector<DataSource *> s, THStack *h);

    virtual std::string getSourceID() const;
    virtual void addToLegend(TLegend *legend);
    virtual void setupRanges();
    virtual void setTitle(const std::string &s);
    virtual TH1 *getHistogram();
    virtual TH1 *getTotals();
    virtual std::string getName() const;

    virtual void setMinRange(float v);
    virtual void setMaxRange(float v);
    virtual float getMinRange() const;
    virtual float getMaxRange() const;

    virtual float getIntegral();
    virtual TAxis *getXAxis();
    virtual TAxis *getYAxis();
    virtual void Draw(const std::string &s);
    virtual std::string to_string() const;
    virtual float getMinDomain();
    virtual float getMaxDomain();
    virtual ~Stack() = default;
};

namespace sol {
class state;
}

void bindPlotElements(sol::state &lua);
