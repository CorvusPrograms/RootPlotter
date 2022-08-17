cmake_minimum_required(VERSION 3.14)

project(RootPlotter)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE DEBUG)
endif()

if (MSVC)
  add_compile_options(/W4 /WX)
  add_compile_options(
    $<$<CONFIG:RELEASE>:/O3>
    $<$<CONFIG:DEBUG>:/O0>
    $<$<CONFIG:DEBUG>:/g>
    )
else()
  add_compile_options(-Wno-unused-variable)
  add_compile_options(-Wall -Wextra -pedantic -Wno-unused-variable -ftemplate-backtrace-limit=0)
  add_compile_options(
    $<$<CONFIG:RELEASE>:-O3>
    $<$<CONFIG:DEBUG>:-O0>
    $<$<CONFIG:DEBUG>:-g>
    #$<$<CONFIG:DEBUG>:-fsanitize=address>
    )

  add_link_options(
    #$<$<CONFIG:DEBUG>:-fsanitize=address>
    )
endif()

include(FetchContent)
FetchContent_Declare(
fmt
GIT_REPOSITORY https://github.com/fmtlib/fmt
GIT_TAG b6f4ceaed0a0a24ccf575fab6c56dd50ccf6f1a9
)

FetchContent_Declare(
json
GIT_REPOSITORY https://github.com/nlohmann/json/
GIT_TAG 4f8fba14066156b73f1189a2b8bd568bde5284c5
)

#FetchContent_Declare(
#sol2
#GIT_REPOSITORY https://github.com/ThePhD/sol2/
#GIT_TAG eba86625b707e3c8c99bbfc4624e51f42dc9e561
#)

FetchContent_MakeAvailable(fmt json)

find_package(ROOT REQUIRED COMPONENTS RIO Core Hist Tree Thread)

add_executable(plotter plotter.cpp)

target_link_libraries(plotter PUBLIC fmt::fmt ROOT::Core ROOT::Hist ROOT::RIO nlohmann_json::nlohmann_json ROOT::Graf ROOT::Gpad)


unset(CMAKE_BUILD_TYPE CACHE)

install(TARGETS plotter DESTINATION "~/.local/bin")
