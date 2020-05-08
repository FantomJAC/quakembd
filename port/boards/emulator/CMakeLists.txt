set(USE_METAL_API ON)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib/minifb ${CMAKE_BINARY_DIR}/minifb)

add_executable(quakembd
	main.c
	display.c
	../../fio/fio_posix.c
)

target_link_libraries(quakembd
	winquake
	port
	minifb
)