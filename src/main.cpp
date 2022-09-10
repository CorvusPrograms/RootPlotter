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
#include "verbosity.h"

int main(int argc, char *argv[]) {
    // TH1::AddDirectory(kFALSE);
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::table,
                       sol::lib::io, sol::lib::debug, sol::lib::os,
                       sol::lib::package);
    sol::optional<std::string> lua_path = lua["package"]["path"];
    std::string new_path;
    if (lua_path) {
        new_path = lua_path.value() + ";" APP_INSTALL_DATAROOTDIR "/?.lua";
    } else {
        new_path = APP_INSTALL_DATAROOTDIR "/?.lua";
    }
    lua["package"]["path"] = new_path;

    gErrorIgnoreLevel = kFatal;
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

    int v_flag = 0;
    app.add_flag(
        "-v", v_flag,
        "Verbosity. Set flag between 0 and 3 times to indicate level.");

    std::string cli_script;

    CLI::Option *script_opt =
        app.add_option("-e,--execute", cli_script,
                       "Lua script to execute, after loading the the base "
                       "files but before loading any user config files.");

    CLI::Option *f_opt = app.add_option("file", config_file_name,
                                        "Path to the configuration file");
    pal_opt->excludes(f_opt);
    ext_opt->needs(f_opt);
    tot_opt->needs(f_opt);
    CLI11_PARSE(app, argc, argv);

    verbosity = v_flag;
    lua["VERBOSITY"] = verbosity;

#ifdef SOL_ALL_SAFETIES_ON
    fmt::print("All lua safeties are on\n");
#endif

    // try {
    if (just_palettes) {
        lua.script_file(APP_INSTALL_DATAROOTDIR "/list_pals.lua");
        std::exit(0);
    }
    if (!cli_script.empty()) {
        lua.script(cli_script);
    }

    if (extract_keys) {
        lua.script("function plot(...) end");
        lua.script("function execute_deferred_plots(...) end");
    }
    if (extract_totals) {
        lua.script("function execute_deferred_plots(...) end");
        lua.script_file(APP_INSTALL_DATAROOTDIR "/get_totals.lua");
    }

    lua.script_file(config_file_name);
    lua.script("execute_deferred_plots()");

    if (extract_keys) {
        lua.script_file(APP_INSTALL_DATAROOTDIR "/extract_keys.lua");
        std::exit(0);
    }
    // }

    // catch (std::exception &e) {
    //     fmt::print("ENCOUNTERED EXCEPTION\n{}", e.what());
    // }
    return 0;
}
