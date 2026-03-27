cmake_minimum_required(VERSION 4.2.1)
project(ams2_haptics C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-Wall -Wextra -Werror)
add_executable(ams2_haptics src/main.c src/ams2_telemetry.c src/telemetry_debug_log.c)
