cmake_minimum_required(VERSION 4.2.1)
project(ams2_haptics LANGUAGES C CXX)

set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-Wall -Wextra -Werror)

# stdout telemetry logger
add_executable(ams2_telemetry_logger src/telemetry_logger.c src/ams2_telemetry.c)

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
target_link_libraries(ams2_haptics PRIVATE ftxui::screen ftxui::dom ftxui::component)
