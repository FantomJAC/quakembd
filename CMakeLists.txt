cmake_minimum_required(VERSION 3.8)
project(QuakEMBD C ASM)

# IMPORTANT: WinQuake uses 32-bit fixed length enum
add_compile_options(-fno-common -fno-short-enums)

add_subdirectory(port)
add_subdirectory(winquake)