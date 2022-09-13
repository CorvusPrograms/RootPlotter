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
                       sol::lib::package, sol::lib::math);
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
    std::string output_base_path;
    std::string cli_script;
    bool just_palettes = false;
    bool extract_keys = false;
    bool extract_totals = false;
    int v_flag = 0;

    CLI::Option *pal_opt =
        app.add_flag("-P,--print-palettes", just_palettes,
                     "Print the possible palettes, then exit.");

    CLI::Option *ext_opt = app.add_flag(
        "-E,--extract-keys", extract_keys,
        "Extract the keys for a given configuration file, then exit.");
    CLI::Option *tot_opt = app.add_flag(
        "-T,--extract-totals", extract_totals,
        "Extract the totals for a given configuration file, then exit.");

    app.add_flag(
        "-v", v_flag,
        "Verbosity. Set flag between 0 and 3 times to indicate level.");

    CLI::Option *script_opt =
        app.add_option("-e,--execute", cli_script,
                       "Lua script to execute, after loading the the base "
                       "files but before loading any user config files.");

    CLI::Option *f_opt = app.add_option("file", config_file_name,
                                        "Path to the configuration file");
    CLI::Option *out_opt = app.add_option(
        "-o,--output", output_base_path,
        "Base output directory. Overwrites value in config value.");
    pal_opt->excludes(f_opt);
    ext_opt->needs(f_opt);
    tot_opt->needs(f_opt);
    CLI11_PARSE(app, argc, argv);

    verbosity = v_flag;
    lua["VERBOSITY"] = verbosity;
    if (verbosity > 3) {
        fmt::print(
            "Verbosity level must be between 0 and 3, you specified {}\n",
            verbosity);
        std::exit(1);
    }

    vPrintLow("Running at verbosity level {}\n", verbosity);

    if (output_base_path.empty()) {
        output_base_path = "output";
    }
    lua["OUTPUT_BASE_PATH"] = output_base_path;
    vPrintLow("Ouput base path is {}\n", output_base_path);

#ifdef SOL_ALL_SAFETIES_ON
    if (verbosity > VerbosityLevel::Low) {
        fmt::print("All lua safeties are on\n");
    }
#endif

    if (just_palettes) {
        lua.script_file(APP_INSTALL_DATAROOTDIR "/list_pals.lua");
        std::exit(0);
    }
    if (!cli_script.empty()) {
        lua.script(cli_script);
    }

    if (extract_keys) {
        vPrintLow("Running key extraction\n");
        vPrintHigh("Setting plot and execute to empty functions\n");
        lua.script("function plot(...) end");
        lua.script("function execute_deferred_plots(...) end");
    }
    if (extract_totals) {
        vPrintLow("Running total extraction\n");
        vPrintHigh("Setting execute to empty function\n");
        lua.script("function execute_deferred_plots(...) end");
        lua.script_file(APP_INSTALL_DATAROOTDIR "/get_totals.lua");
    }

    auto result = lua.safe_script_file(config_file_name);
    if (!result.valid()) {
        sol::error err = result;
        fmt::print(
            "Caught exception during user script execution, please check the "
            "validity of your script:\nException:\n{}\n",
            err.what());
        std::exit(1);
    } else {
        vPrintMedium("Successfully established configuration\n");
    }

    vPrintMedium("Executing deferred plots\n");

    result = lua.safe_script("execute_deferred_plots()");

    if (!result.valid()) {
        sol::error err = result;
        fmt::print(
            "Caught exception during user plot execution. This likely means "
            "that you passed invalid parameters to one of your 'plot' "
            "calls\n:\nException:\n{}\n",
            err.what());
        std::exit(1);
    } else {
        vPrintMedium("Successfully executed deferred plots\n");
    }
    if (extract_keys) {
        lua.script_file(APP_INSTALL_DATAROOTDIR "/extract_keys.lua");
        std::exit(0);
    }
    return 0;
}
