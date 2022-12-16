#include "bindings.h"

#include <TColor.h>
#include <TFile.h>

#include <sol/sol.hpp>

#include "data.h"
#include "plotting.h"
#include "util.h"

namespace rootp {

void bindDataOps(sol::state &lua) {
    auto faststyle = [&lua](DataSource &ds, sol::table s) {
        for (auto pair : s) {
            lua["Style"][pair.first](ds.style, pair.second);
        }
        return ds;
    };

    auto data_source_type = lua.new_usertype<DataSource>(
        "DataSource", sol::constructors<DataSource(const std::string &)>(),
        BUILD(DataSource, name), BUILD(DataSource, path),
        BUILD(DataSource, tags), BUILD(DataSource, file),
        BUILD(DataSource, keys), "style", faststyle);

    auto plot_data_type = lua.new_usertype<PlotData>(
        "PlotData", "name", sol::readonly(&PlotData::name));

    auto source_set_type = lua.new_usertype<SourceSet>(
        "SourceSet",
        sol::constructors<SourceSet(const std::vector<DataSource *>)>(),
        BUILD(SourceSet, sources), "get_keys", &SourceSet::getKeys);

    auto style_type = lua.new_usertype<Style>(
        "Style", BUILD(Style, mode), BUILD(Style, palette_idx),
        BUILD(Style, marker_style), BUILD(Style, marker_size),
        BUILD(Style, line_width), BUILD(Style, line_style),
        BUILD(Style, fill_style), BUILD(Style, color));

    lua["get_histos"] = extractMatchingHistos;
}
void bindPlotting(sol::state &lua) {
    auto draw_pad = lua.new_usertype<DrawPad>("DrawPad");
    auto draw_opts = lua.new_usertype<CommonOptions>(
        "Options", BUILD(CommonOptions, logx), BUILD(CommonOptions, logy),
        BUILD(CommonOptions, normalize), BUILD(CommonOptions, x_label),
        BUILD(CommonOptions, y_label), BUILD(CommonOptions, plot_title),
        BUILD(CommonOptions, xrange), BUILD(CommonOptions, yrange));
    lua["new_color"] = sol::overload(
        [](int r, int g, int b) { return TColor::GetColor(r, g, b); },
        [](float r, float g, float b) { return TColor::GetColor(r, g, b); },
        [](const std::string &hex) { return TColor::GetColor(hex.c_str()); });

    lua["plotting"] = lua.create_table();
    lua["plotting"]["simple"] = plotStandard;
    lua["plotting"]["stack"] = plotStack;
    lua["plotting"]["new_legend"] = newLegend;
    lua["plotting"]["add_to_legend"] = addToLegend;
    lua["plotting"]["add_legend_to_pad"] = addLegendToPad;
    lua["plotting"]["save_pad"] =
        sol::resolve<void(const DrawPad &, const std::string &)>(saveDrawPad);
    lua["plotting"]["execute_plot"] = executePlot;

    lua["transforms"] = lua.create_table();
    lua["transforms"]["sort_integral"] = [](std::vector<PlotData> &d) {
        transforms::sortIntegral(d.begin(), d.end());
    };
    lua["transforms"]["remove_empty"] = [](std::vector<PlotData> &d) {
        d.erase(transforms::removeEmpty(d.begin(), d.end()), d.end());
    };
    lua["transforms"]["norm_to"] = [](std::vector<PlotData> &d, float val) {
        std::vector<PlotData> ret;
        ret.reserve(d.size());
        transforms::createNormed(d.begin(), d.end(), std::back_inserter(ret),
                                 val);
        return ret;
    };
}

void bindMarkerStyles(sol::state &lua) {
    lua["mark_style"] = lua.create_table();
    lua["mark_style"]["Dot"] = 1;
    lua["mark_style"]["Plus"] = 2;
    lua["mark_style"]["Star"] = 3;
    lua["mark_style"]["Circle"] = 4;
    lua["mark_style"]["Multiply"] = 5;
    lua["mark_style"]["FullDotSmall"] = 6;
    lua["mark_style"]["FullDotMedium"] = 7;
    lua["mark_style"]["FullDotLarge"] = 8;
    lua["mark_style"]["ScalableDot"] = 9;
    lua["mark_style"]["FullCircle"] = 20;
    lua["mark_style"]["FullSquare"] = 21;
    lua["mark_style"]["FullTriangleUp"] = 22;
    lua["mark_style"]["FullTriangleDown"] = 23;
    lua["mark_style"]["OpenCircle"] = 24;
    lua["mark_style"]["OpenSquare"] = 25;
    lua["mark_style"]["OpenTriangleUp"] = 26;
    lua["mark_style"]["OpenDiamond"] = 27;
    lua["mark_style"]["OpenCross"] = 28;
    lua["mark_style"]["FullStar"] = 29;
    lua["mark_style"]["OpenStar"] = 30;
    lua["mark_style"]["OpenTriangleDown"] = 32;
    lua["mark_style"]["FullDiamond"] = 33;
    lua["mark_style"]["FullCross"] = 34;
    lua["mark_style"]["OpenDiamondCross"] = 35;
    lua["mark_style"]["OpenSquareDiagonal"] = 36;
    lua["mark_style"]["OpenThreeTriangles"] = 37;
    lua["mark_style"]["OctagonCross"] = 38;
    lua["mark_style"]["FullThreeTriangles"] = 39;
    lua["mark_style"]["OpenFourTrianglesX"] = 40;
    lua["mark_style"]["FullFourTrianglesX"] = 41;
    lua["mark_style"]["OpenDoubleDiamond"] = 42;
    lua["mark_style"]["FullDoubleDiamond"] = 43;
    lua["mark_style"]["OpenFourTrianglesPlus"] = 44;
    lua["mark_style"]["FullFourTrianglesPlus"] = 45;
    lua["mark_style"]["OpenCrossX"] = 46;
    lua["mark_style"]["FullCrossX"] = 47;
    lua["mark_style"]["FourSquaresX"] = 48;
    lua["mark_style"]["FourSquaresPlus"] = 49;
}
void bindPalettes(sol::state &lua) {
    lua["palettes"] = lua.create_table();
    lua["palettes"]["DeepSea"] = 51;
    lua["palettes"]["GreyScale"] = 52;
    lua["palettes"]["DarkBodyRadiator"] = 53;
    lua["palettes"]["BlueYellow"] = 54;
    lua["palettes"]["RainBow"] = 55;
    lua["palettes"]["InvertedDarkBodyRadiator"] = 56;
    lua["palettes"]["Bird"] = 57;
    lua["palettes"]["Cubehelix"] = 58;
    lua["palettes"]["GreenRedViolet"] = 59;
    lua["palettes"]["BlueRedYellow"] = 60;
    lua["palettes"]["Ocean"] = 61;
    lua["palettes"]["ColorPrintableOnGrey"] = 62;
    lua["palettes"]["Alpine"] = 63;
    lua["palettes"]["Aquamarine"] = 64;
    lua["palettes"]["Army"] = 65;
    lua["palettes"]["Atlantic"] = 66;
    lua["palettes"]["Aurora"] = 67;
    lua["palettes"]["Avocado"] = 68;
    lua["palettes"]["Beach"] = 69;
    lua["palettes"]["BlackBody"] = 70;
    lua["palettes"]["BlueGreenYellow"] = 71;
    lua["palettes"]["BrownCyan"] = 72;
    lua["palettes"]["CMYK"] = 73;
    lua["palettes"]["Candy"] = 74;
    lua["palettes"]["Cherry"] = 75;
    lua["palettes"]["Coffee"] = 76;
    lua["palettes"]["DarkRainBow"] = 77;
    lua["palettes"]["DarkTerrain"] = 78;
    lua["palettes"]["Fall"] = 79;
    lua["palettes"]["FruitPunch"] = 80;
    lua["palettes"]["Fuchsia"] = 81;
    lua["palettes"]["GreyYellow"] = 82;
    lua["palettes"]["GreenBrownTerrain"] = 83;
    lua["palettes"]["GreenPink"] = 84;
    lua["palettes"]["Island"] = 85;
    lua["palettes"]["Lake"] = 86;
    lua["palettes"]["LightTemperature"] = 87;
    lua["palettes"]["LightTerrain"] = 88;
    lua["palettes"]["Mint"] = 89;
    lua["palettes"]["Neon"] = 90;
    lua["palettes"]["Pastel"] = 91;
    lua["palettes"]["Pearl"] = 92;
    lua["palettes"]["Pigeon"] = 93;
    lua["palettes"]["Plum"] = 94;
    lua["palettes"]["RedBlue"] = 95;
    lua["palettes"]["Rose"] = 96;
    lua["palettes"]["Rust"] = 97;
    lua["palettes"]["SandyTerrain"] = 98;
    lua["palettes"]["Sienna"] = 99;
    lua["palettes"]["Solar"] = 100;
    lua["palettes"]["SouthWest"] = 101;
    lua["palettes"]["StarryNight"] = 102;
    lua["palettes"]["Sunset"] = 103;
    lua["palettes"]["TemperatureMap"] = 104;
    lua["palettes"]["Thermometer"] = 105;
    lua["palettes"]["Valentine"] = 106;
    lua["palettes"]["VisibleSpectrum"] = 107;
    lua["palettes"]["WaterMelon"] = 108;
    lua["palettes"]["Cool"] = 109;
    lua["palettes"]["Copper"] = 110;
    lua["palettes"]["GistEarth"] = 111;
    lua["palettes"]["Viridis"] = 112;
    lua["palettes"]["Cividis"] = 113;
}

void bindLineStyles(sol::state &lua) {
    lua["line_style"] = lua.create_table();
    lua["line_style"]["solid"] = 1;
    lua["line_style"]["d1"] = 2;
    lua["line_style"]["faint"] = 3;
    lua["line_style"]["d2"] = 4;
    lua["line_style"]["d3"] = 5;
    lua["line_style"]["d3"] = 6;
    lua["line_style"]["d5"] = 7;
    lua["line_style"]["d6"] = 8;
    lua["line_style"]["dashed"] = 9;
    lua["line_style"]["dotdashed"] = 10;
}
void bindFillStyles(sol::state &lua) {
    lua["fill_style"] = lua.create_table();
    lua["fill_style"]["hollow"] = 0;
    lua["fill_style"]["solid"] = 1001;
}

}  // namespace rootp
