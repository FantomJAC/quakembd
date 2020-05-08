add_library(port OBJECT
	in_null.c
	cd_null.c
	snd_null.c
	sys_port.c
	vid_port.c
)

target_include_directories(port PUBLIC
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/winquake
)

add_subdirectory(boards/${BOARD_NAME})