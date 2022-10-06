#include "data.h"

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <algorithm>
#include <sol/sol.hpp>

#include "TFile.h"
#include "util.h"
#include "verbosity.h"

std::unordered_set<std::string> SourceSet::getKeys() const {
    return common_keys;
}

void SourceSet::initKeys() {
    std::unordered_set<std::string> ret = sources[0]->keys;
    for (std::size_t i = 1; i < sources.size(); ++i) {
        for (const auto &k : sources[i]->keys) {
            if (ret.count(k) == 0) {
                ret.erase(k);
            }
        }
    }
    common_keys = std::move(ret);
}

TH1 *DataSource::getHist(const std::string &name) {
    assert(file != nullptr);
    TH1 *hist = (file->Get<TH1>(name.c_str()));
    return hist;
};

void DataSource::load() {
    file = TFile::Open((path).c_str());
    if (!file) {
        vRuntimeError("Could not open file {}", path);
    }
}

void DataSource::loadKeys() {
    for (const auto &key : *(file->GetListOfKeys())) {
        keys.insert(key->GetName());
    }
    vPrintHigh("Extracted {} keys from file {}\n", std::size(keys), path);
    if (keys.empty()) {
        vRuntimeError("File {} does not contain any keys", path);
    }
}

// std::string SourceSet::to_string() {
//     std::vector<std::string> temp;
//     for (const auto &ds : sources) {
//         temp.push_back(ds->path);
//     }
//     return fmt::format("SourceSet({})", temp);
// }

std::vector<DataSource *> SourceSet::getSources() { return sources; }
std::vector<DataSource *> DataSource::getSources() { return {this}; }

std::unordered_set<std::string> DataSource::getKeys() const { return keys; }

void bindData(sol::state &lua) {
    lua["finalize_input_data"] =
        sol::overload(finalizeManyInputData, finalizeInputData);
    lua["expand_data"] = expand;

    auto style_type = lua.new_usertype<Style>(
        "Style", BUILD(Style, mode), BUILD(Style, palette_idx),
        BUILD(Style, marker_style), BUILD(Style, marker_size),
        BUILD(Style, line_width), BUILD(Style, line_style),
        BUILD(Style, fill_style), BUILD(Style, color));

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
        BUILD(DataSource, keys),
        //"keys", &DataSource::keys,
        //  sol::meta_function::garbage_collect,
        //  sol::destructor( [](DataSource* ds){
        //      fmt::print("Calling garbage collection on {}\n",
        //      ds->to_string());
        // } ),
        "style", faststyle, sol::base_classes, sol::bases<SourceSet>());

    auto plot_styles_type =
        lua.new_enum<Style::Mode>("plot_mode", {{"none", Style::Mode::None},
                                                {"line", Style::Mode::Line},
                                                {"marker", Style::Mode::Marker},
                                                {"fill", Style::Mode::Fill}});

    auto source_set_type = lua.new_usertype<SourceSet>(
        "SourceSet",
        sol::constructors<SourceSet(const std::vector<DataSource *>)>(),
        BUILD(SourceSet, sources), "get_keys", &SourceSet::getKeys);

    auto input_data = lua.new_usertype<InputData>(
        "InputData", sol::constructors<InputData(), InputData(SourceSet *)>(),
        BUILD(InputData, source_set), BUILD(InputData, normalize),
        BUILD(InputData, sort), BUILD(InputData, norm_to),
        BUILD(InputData, yrange), "xrange",
        [](InputData *c, float x, float y) {
            c->xrange = {x, y};
            return c;
        },
        "yrange",
        [](InputData *c, float x, float y) {
            c->yrange = {x, y};
            return c;
        },
        BUILD(InputData, stack));

    auto matched_type = lua.new_usertype<MatchedKey>(
        "MatchedKey", "inputs", sol::readonly(&MatchedKey::inputs), "captures",
        sol::readonly(&MatchedKey::captures));

    lua["r_get_hist"] = [](DataSource *ds, const std::string &name) {
        TH1 *ret;
        ds->file->GetObject(name.c_str(), ret);
        return ret;
    };

    lua["r_total_hist_entries"] = [](TH1 *hist) { return hist->GetEntries(); };
}

std::pair<std::vector<std::unique_ptr<PlotElement>>, bool> finalizeInputData(
    const PlotterInput &input) {
    assert(input.data.source_set);

    std::vector<std::unique_ptr<PlotElement>> ret;
    std::vector<DataSource *> sources;
    bool all_filled = true;
    static int i = 0;
    auto stack = new THStack();
    auto insources = input.data.source_set->getSources();

    std::vector<std::pair<DataSource *, TH1 *>> hists;

    for (DataSource *source : insources) {
        ++i;
        TH1 *hist = (TH1 *)(source->getHist(input.name)
                                ->Clone(std::to_string(i).c_str()));
        if (input.data.stack) {
            all_filled = all_filled || (hist->GetEntries() > 0);
        } else {
            all_filled = all_filled && (hist->GetEntries() > 0);
        }
        // TH1 *hist = source->getHist(input.name);
        if (!hist) {
            vRuntimeError("Could not get a histogram from file {} with name {}",
                          source->name, input.name);
        }
        hists.push_back({source, hist});
    }
    if (input.data.stack && input.data.sort) {
        std::sort(hists.begin(), hists.end(), [](auto &h1, auto &h2) {
            return h1.second->Integral() < h2.second->Integral();
        });
    }

    for (auto &pair : hists) {
        TH1 *hist = pair.second;
        DataSource *source = pair.first;
        if (input.data.normalize) {
            hist->Scale(input.data.norm_to / hist->Integral());
        }
        if (input.data.stack) {
            stack->Add(hist);
            sources.push_back(source);
        } else {
            auto h = std::make_unique<Histogram>(source, hist);
            if (input.data.xrange) {
                h->setRangeX(input.data.xrange.value());
            }
            if (input.data.yrange) {
                h->setRangeY(input.data.yrange.value());
            }
            ret.push_back(std::move(h));
        }
    }
    if (input.data.stack) {
        auto h = std::make_unique<Stack>(sources, stack);
        if (input.data.xrange) {
            h->setRangeX(input.data.xrange.value());
        }
        if (input.data.yrange) {
            h->setRangeY(input.data.yrange.value());
        }
        ret.push_back(std::move(h));
    }
    return {std::move(ret), all_filled};
}

std::pair<std::vector<std::unique_ptr<PlotElement>>, bool>
finalizeManyInputData(const std::vector<PlotterInput> &input) {
    std::vector<std::unique_ptr<PlotElement>> ret;
    bool all_filled = true;
    for (const auto &d : input) {
        auto one_set = finalizeInputData(d);
        std::move(one_set.first.begin(), one_set.first.end(),
                  std::back_inserter(ret));
        all_filled = all_filled && one_set.second;
    }
    return {std::move(ret), all_filled};
}

std::vector<MatchedKey> expand(std::vector<InputData> in,
                               const std::string &pattern) {
    vPrintHigh("Expanding pattern {}\n", pattern);
    std::unordered_set<std::string> keys = in[0].source_set->getKeys();
    for (std::size_t i = 1; i < in.size(); ++i) {
        for (const auto &k : in[i].source_set->getKeys()) {
            if (keys.count(k) == 0) {
                keys.erase(k);
            }
        }
    }
    if (!glob::isGlob(pattern)) {
        vPrintHigh(
            "Pattern {} was not recognized as a glob, treating as "
            "name\n",
            pattern);
        MatchedKey single;
        for (const auto &input : in) {
            single.inputs.push_back({pattern, input});
            single.captures["HISTNAME"] = pattern;
        }
        vPrintHigh("Pattern expanded to 1 result\n");
        return {single};
    }
    std::vector<MatchedKey> ret;
    for (const auto &k : keys) {
        if (glob::match(k, pattern)) {
            MatchedKey single;
            for (const auto &input : in) {
                single.inputs.push_back({k, input});
                single.captures["HISTNAME"] = k;
            }
            ret.push_back(single);
        }
    }
    vPrintHigh("Pattern expanded to {} results\n", std::size(ret));
    if (ret.empty()) {
        vRuntimeError("Pattern '{}' does not match any key in the data sources",
                      pattern);
    }
    return ret;
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

void bindGraphicalData(sol::state &lua) {
    bindMarkerStyles(lua);
    bindLineStyles(lua);
    bindFillStyles(lua);
    bindPalettes(lua);
}
