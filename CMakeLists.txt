cmake_minimum_required(VERSION 3.8)
project(QuakEMBD C ASM)

add_compile_options(-fno-common)
add_definitions(-DWINQUAKE_ENABLE_LOGGING -DWINQUAKE_LOGGING_EXTERNAL)

add_subdirectory(port)
add_subdirectory(winquake)