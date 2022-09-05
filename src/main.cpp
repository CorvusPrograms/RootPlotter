#include <TError.h>

#include <sol/sol.hpp>
#include <fmt/format.h>

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
    lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::table);
    bindPlotters(lua);
    bindData(lua);
    bindPalettes(lua);
    CLI::App app{fmt::format("Root plotter interface\nRootPlotter version {}.{}",
                             PLOTTER_VERSION_MAJOR, PLOTTER_VERSION_MINOR)};
    std::string config_file_name;
    CLI::Option* file_opt = app.add_option("file", config_file_name,
                                           "Path to the configuration file")
                                ->required();
    CLI11_PARSE(app, argc, argv);
    lua.script_file(APP_INSTALL_DATAROOTDIR "/base.lua");
    {
        CLI::AutoTimer timer{"Creating Graphs", CLI::Timer::Big};
        lua.script_file("example.lua");
    }
    return 0;
}
