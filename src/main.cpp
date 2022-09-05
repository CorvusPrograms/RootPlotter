#include <TError.h>
#include <fmt/format.h>

#include <sol/sol.hpp>

#include "CLI/App.hpp"
#include "CLI/Config.hpp"
#include "CLI/Formatter.hpp"
#include "CLI/Timer.hpp"
#include "data.h"
#include "install_info.h"
#include "plot_element.h"
#include "plotters.h"

int main(int argc, char* argv[]) {
    // TH1::AddDirectory(kFALSE);
    sol::state lua;
    gErrorIgnoreLevel = kFatal;
    lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::table,
                       sol::lib::io);
    bindPlotters(lua);
    bindData(lua);
    bindPalettes(lua);
    lua.script_file(APP_INSTALL_DATAROOTDIR "/base.lua");

    CLI::App app{
        fmt::format("Root plotter interface\nRootPlotter version {}.{}",
                    PLOTTER_VERSION_MAJOR, PLOTTER_VERSION_MINOR)};
    std::string config_file_name;
    bool just_palettes = false;
    bool extract_keys = false;

    CLI::Option* pal_opt = app.add_flag(
        "-P", just_palettes, "Print the possible palettes, then exit.");

    CLI::Option* ext_opt = app.add_flag(
        "-E,--extract-keys", extract_keys,
        "Extract the keys for a given configuration file, then exit.");

    CLI::Option* f_opt = app.add_option("file", config_file_name,
                                        "Path to the configuration file");
    pal_opt->excludes(f_opt);
    CLI11_PARSE(app, argc, argv);

    if (just_palettes) {
        lua.script_file(APP_INSTALL_DATAROOTDIR "/list_pals.lua");
        std::exit(0);
    }

    if (extract_keys) {
        lua.script("function plot(...) end");
    }
    try {
        lua.script_file(config_file_name);
    } catch (std::exception& e) {
        fmt::print("ENCOUNTERED EXCEPTION\n{}", e.what());
    }
    if (extract_keys) {
        lua.script_file(APP_INSTALL_DATAROOTDIR "/extract_keys.lua");
        std::exit(0);
    }

    return 0;
}
