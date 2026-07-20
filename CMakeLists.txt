cmake_minimum_required(VERSION 4.2.1)
project(ams2_haptics LANGUAGES C CXX)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-Wall -Wextra -Werror)

# carquet parquet library (submodule)
set(CARQUET_BUILD_CLI OFF CACHE BOOL "" FORCE)
add_subdirectory(carquet EXCLUDE_FROM_ALL)
target_compile_options(carquet PRIVATE -Wno-error)

# stdout telemetry logger
add_executable(ams2_telemetry_logger src/telemetry_logger.c src/ams2_telemetry.c)

# parquet telemetry logger
add_executable(ams2_parquet_logger src/telemetry_parquet.c src/ams2_telemetry.c)
target_link_libraries(ams2_parquet_logger PRIVATE carquet)

# Sample apps
include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/arthursonzogni/ftxui.git
  GIT_TAG v6.1.9
  GIT_SHALLOW TRUE
  EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(ftxui)

add_executable(ams2_haptics src/main.cpp src/ams2_telemetry.c src/telemetry_debug_log.c)
target_link_libraries(ams2_haptics PRIVATE ftxui::screen ftxui::component)

add_executable(ams2_wheel_slip src/wheel_slip.cpp src/ams2_telemetry.c)
target_link_libraries(ams2_wheel_slip PRIVATE ftxui::screen ftxui::component)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PIPEWIRE IMPORTED_TARGET libpipewire-0.3)
if(PIPEWIRE_FOUND)
  add_executable(pipewire-tone src/pipewire_tone.c)
  target_link_libraries(pipewire-tone PRIVATE PkgConfig::PIPEWIRE m)
endif()
