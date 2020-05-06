cmake_minimum_required(VERSION 3.8)
project(QuakEMBD C ASM)

set(USE_METAL_API ON)
add_subdirectory(lib/minifb)

set(WQ_SRCS
	winquake/chase.c
	winquake/cmd.c
	winquake/common.c
	winquake/console.c
	winquake/crc.c
	winquake/cvar.c
	winquake/draw.c
	winquake/host.c
	winquake/host_cmd.c
	winquake/keys.c
	winquake/mathlib.c
	winquake/menu.c
	winquake/model.c
	winquake/nonintel.c
	winquake/screen.c
	winquake/sbar.c
	winquake/zone.c
	winquake/view.c
	winquake/wad.c
	winquake/world.c
	winquake/cl_demo.c
	winquake/cl_input.c
	winquake/cl_main.c
	winquake/cl_parse.c
	winquake/cl_tent.c
	winquake/d_edge.c
	winquake/d_fill.c
	winquake/d_init.c
	winquake/d_modech.c
	winquake/d_part.c
	winquake/d_polyse.c
	winquake/d_scan.c
	winquake/d_sky.c
	winquake/d_sprite.c
	winquake/d_surf.c
	winquake/d_vars.c
	winquake/d_zpoint.c
	winquake/net_dgrm.c
	winquake/net_loop.c
	winquake/net_main.c
	winquake/net_vcr.c
	winquake/net_udp.c
	winquake/net_bsd.c
	winquake/pr_cmds.c
	winquake/pr_edict.c
	winquake/pr_exec.c
	winquake/r_aclip.c
	winquake/r_alias.c
	winquake/r_bsp.c
	winquake/r_light.c
	winquake/r_draw.c
	winquake/r_efrag.c
	winquake/r_edge.c
	winquake/r_misc.c
	winquake/r_main.c
	winquake/r_sky.c
	winquake/r_sprite.c
	winquake/r_surf.c
	winquake/r_part.c
	winquake/r_vars.c
	winquake/sv_main.c
	winquake/sv_phys.c
	winquake/sv_move.c
	winquake/sv_user.c
)

set(WQ_I386_GAS
	winquake/x86/d_draw.s
	winquake/x86/d_draw16.s
	winquake/x86/d_parta.s
	winquake/x86/d_polysa.s
	winquake/x86/d_scana.s
	winquake/x86/d_spr8.s
	winquake/x86/d_varsa.s
	winquake/x86/math.s
	winquake/x86/r_aliasa.s
	winquake/x86/r_drawa.s
	winquake/x86/r_edgea.s
	winquake/x86/r_varsa.s
	winquake/x86/surf16.s
	winquake/x86/surf8.s
	winquake/x86/worlda.s
	winquake/x86/r_aclipa.s
)
set_source_files_properties(${WQ_I386_GAS} PROPERTIES COMPILE_FLAGS "${CFLAGS} -DELF -x assembler-with-cpp")

add_executable(quake
	${WQ_SRCS}
	${WQ_I386_GAS}
	port/in_null.c
	port/cd_null.c
	port/snd_null.c
	port/sys_port.c
	port/vid_port.c
	port/fio/fio_posix.c
	port/boards/emulator/main.c
	port/boards/emulator/display.c
)
target_include_directories(quake PRIVATE winquake include)
target_link_libraries(quake minifb)