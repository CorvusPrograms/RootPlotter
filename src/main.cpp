#include <TCanvas.h>
#include <TError.h>
#include <TROOT.h>
#include <TStyle.h>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include <optional>
#include <sol/sol.hpp>

#include "CLI/App.hpp"
#include "CLI/Config.hpp"
#include "CLI/Formatter.hpp"
#include "CLI/Timer.hpp"
#include "bindings.h"
#include "data.h"
#include "install_info.h"
#include "plotting.h"
#include "pool.h"
#include "verbosity.h"

struct ApplicationOptions {
    std::string config_file_name;
    std::string output_base_path;
    std::string cli_script;
    bool just_palettes = false;
    bool extract_keys = false;
    bool extract_totals = false;
    int v_flag = 0;
};

auto makeCLI(ApplicationOptions &options) {
    auto ret = std::make_unique<CLI::App>(
        fmt::format("Root plotter interface\nRootPlotter version {}.{}",
                    PLOTTER_VERSION_MAJOR, PLOTTER_VERSION_MINOR));
    auto &app = *ret;

    CLI::Option *pal_opt =
        app.add_flag("-P,--print-palettes", options.just_palettes,
                     "Print the possible palettes, then exit.");

    CLI::Option *ext_opt = app.add_flag(
        "-E,--extract-keys", options.extract_keys,
        "Extract the keys for a given configuration file, then exit.");
    CLI::Option *tot_opt = app.add_flag(
        "-T,--extract-totals", options.extract_totals,
        "Extract the totals for a given configuration file, then exit.");

    app.add_flag(
        "-v", options.v_flag,
        "Verbosity. Set flag between 0 and 3 times to indicate level.");

    CLI::Option *script_opt =
        app.add_option("-e,--execute", options.cli_script,
                       "Lua script to execute, after loading the the base "
                       "files but before loading any user config files.");

    CLI::Option *f_opt = app.add_option("file", options.config_file_name,
                                        "Path to the configuration file");
    CLI::Option *out_opt = app.add_option(
        "-o,--output", options.output_base_path,
        "Base output directory. Overwrites value in config value.");
    pal_opt->excludes(f_opt);
    ext_opt->needs(f_opt);
    tot_opt->needs(f_opt);
    return ret;
}

int main(int argc, char *argv[]) {
    TH1::AddDirectory(kFALSE);
    ROOT::EnableThreadSafety();
    gErrorIgnoreLevel = kFatal;
    gStyle->SetOptStat(0);
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::table,
                       sol::lib::io, sol::lib::debug, sol::lib::os,
                       sol::lib::package, sol::lib::math, sol::lib::coroutine);
    sol::optional<std::string> lua_path = lua["package"]["path"];
    std::string new_path;
    if (lua_path) {
        new_path = lua_path.value() + ";" APP_INSTALL_DATAROOTDIR "/?.lua";
    } else {
        new_path = APP_INSTALL_DATAROOTDIR "/?.lua";
    }
    lua["package"]["path"] = new_path;

    rootp::bindMarkerStyles(lua);
    rootp::bindLineStyles(lua);
    rootp::bindFillStyles(lua);
    rootp::bindPalettes(lua);
    rootp::bindDataOps(lua);
    rootp::bindPlotting(lua);

    lua.script_file(APP_INSTALL_DATAROOTDIR "/base.lua");

    ApplicationOptions opts;

    auto app = makeCLI(opts);
    CLI11_PARSE(*app, argc, argv);
    verbosity = opts.v_flag;
    lua["VERBOSITY"] = verbosity;
    if (verbosity > 3) {
        fmt::print(
            "Verbosity level must be between 0 and 3, you specified {}\n",
            verbosity);
        std::exit(1);
    }

    vPrint(VerbosityLevel::Low, "Running at verbosity level {}\n", verbosity);

    if (opts.output_base_path.empty()) {
        opts.output_base_path = "output";
    }
    lua["OUTPUT_BASE_PATH"] = opts.output_base_path;
    vPrint(VerbosityLevel::Low, "Ouput base path is {}\n",
           opts.output_base_path);

    if (!opts.cli_script.empty()) {
        lua.script(opts.cli_script);
    }
    if (opts.extract_keys) {
        lua.script("function plot(...) end");
        lua.script("function execute_deferred_plots(...) end");
    }

    auto result = lua.safe_script_file(opts.config_file_name);
    if (!result.valid()) {
        sol::error err = result;
        fmt::print(
            "Caught exception during user script execution, please check the "
            "validity of your script:\nException:\n{}\n",
            err.what());
        return 1;
    }
    result = lua.safe_script("execute_deferred_plots()");
    if (!result.valid()) {
        sol::error err = result;
        fmt::print(
            "Caught exception during user plot execution. This likely means "
            "that you passed invalid parameters to one of your 'plot' "
            "calls\n:\nException:\n{}\n",
            err.what());
        return 2;
    }

    if (opts.extract_keys) {
        lua.script_file(APP_INSTALL_DATAROOTDIR "/extract_keys.lua");
    }

    return 0;
}
