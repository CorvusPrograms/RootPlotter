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

int main(int argc, char *argv[]) {
    // TH1::AddDirectory(kFALSE);
    sol::state lua;
    gErrorIgnoreLevel = kFatal;
    lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::table,
                       sol::lib::io, sol::lib::debug, sol::lib::os);
    bindPlotters(lua);
    bindData(lua);
    bindGraphicalData(lua);
    lua.script_file(APP_INSTALL_DATAROOTDIR "/base.lua");

    CLI::App app{
        fmt::format("Root plotter interface\nRootPlotter version {}.{}",
                    PLOTTER_VERSION_MAJOR, PLOTTER_VERSION_MINOR)};
    std::string config_file_name;
    bool just_palettes = false;
    bool extract_keys = false;
    bool extract_totals = false;

    CLI::Option *pal_opt =
        app.add_flag("-P,--print-palettes", just_palettes,
                     "Print the possible palettes, then exit.");

    CLI::Option *ext_opt = app.add_flag(
        "-E,--extract-keys", extract_keys,
        "Extract the keys for a given configuration file, then exit.");
    CLI::Option *tot_opt = app.add_flag(
        "-T,--extract-totals", extract_totals,
        "Extract the totals for a given configuration file, then exit.");

    CLI::Option *f_opt = app.add_option("file", config_file_name,
                                        "Path to the configuration file");
    pal_opt->excludes(f_opt);
    ext_opt->needs(f_opt);
    tot_opt->needs(f_opt);
    CLI11_PARSE(app, argc, argv);

#ifdef SOL_ALL_SAFETIES_ON
    fmt::print("All lua safeties are on\n");
#endif

    if (just_palettes) {
        lua.script_file(APP_INSTALL_DATAROOTDIR "/list_pals.lua");
        std::exit(0);
    }

    if (extract_keys) {
        lua.script("function plot(...) end");
        lua.script("function execute_deferred_plots(...) end");
    }
    if (extract_totals) {
        lua.script("function execute_deferred_plots(...) end");
        lua.script_file(APP_INSTALL_DATAROOTDIR "/get_totals.lua");
    }

    try {
        lua.script_file(config_file_name);
        lua.script("execute_deferred_plots()");
    }
    catch (std::exception &e) {
        fmt::print("ENCOUNTERED EXCEPTION\n{}", e.what());
    }

    if (extract_keys) {
        lua.script_file(APP_INSTALL_DATAROOTDIR "/extract_keys.lua");
        std::exit(0);
    }
    return 0;
}
