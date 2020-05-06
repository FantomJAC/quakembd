/*
 * Copyright (C) 2020 Shotaro Uchida <fantom@xmaker.mx>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <quakedef.h>
#include <quakembd.h>

#define DEFAULT_MEM_SIZE (8 * 1024 * 1024)

qboolean isDedicated;

int nostdout = 0;

char *basedir = "quakembd";
char *cachedir = "/tmp";

cvar_t  sys_linerefresh = {"sys_linerefresh","0"};// set for entity display

// =======================================================================
// Logging
// =======================================================================

void Sys_DebugLog(char *file, char *fmt, ...)
{
	va_list argptr; 
	char s[1024];

	va_start(argptr, fmt);
	vsprintf(s, fmt, argptr);
	va_end(argptr);

	printf("[DEBUG] %s\n", s);
}

void Sys_Error(char *fmt, ...)
{ 
	va_list argptr; 
	char s[1024];

	va_start(argptr, fmt);
	vsprintf(s, fmt, argptr);
	va_end(argptr);

	printf("[ERROR] %s\n", s);

	Host_Shutdown();
	exit(1);
} 

void Sys_Printf(char *fmt, ...)
{
	va_list argptr;
	char text[1024];
	unsigned char *p;

	va_start(argptr, fmt);
	vsprintf(text, fmt, argptr);
	va_end(argptr);

	if (strlen(text) > sizeof(text))
		Sys_Error("memory overwrite in Sys_Printf");

	if (nostdout)
		return;

	for (p = (unsigned char *) text; *p; p++) {
		*p &= 0x7f;
		if ((*p > 128 || *p < 32) && *p != 10 && *p != 13 && *p != 9)
			printf("[%02x]", *p);
		else
			putc(*p, stdout);
	}
}

// =======================================================================
// General routines
// =======================================================================

void Sys_Quit(void)
{
	Host_Shutdown();
	exit(0);
}

double Sys_FloatTime(void)
{
	return qembd_get_us_time() / 1000000.0;
}

char *Sys_ConsoleInput(void)
{
	// TODO
	return NULL;
}

void Sys_Sleep(void)
{
}

void Sys_SendKeyEvents(void)
{
}

void Sys_HighFPPrecision(void)
{
}

void Sys_LowFPPrecision(void)
{
}

void Sys_MakeCodeWriteable(unsigned long startaddr, unsigned long length)
{
}

int qembd_main(int c, char **v)
{
	float time, oldtime, newtime;
	quakeparms_t parms = {0};
	extern int vcrFile;
	extern int recording;
	int j;

	COM_InitArgv(c, v);
	parms.argc = com_argc;
	parms.argv = com_argv;

	parms.memsize = DEFAULT_MEM_SIZE;
	j = COM_CheckParm("-mem");
	if (j)
		parms.memsize = (int) (Q_atof(com_argv[j+1]) * 1024 * 1024);
	parms.membase = qembd_allocmain(parms.memsize);
	if (!parms.membase)
		Sys_Error("Memory cannot be allocated");

	parms.basedir = basedir;
// caching is disabled by default, use -cachedir to enable
//	parms.cachedir = cachedir;

	Host_Init(&parms);

	if (COM_CheckParm("-nostdout")) {
		nostdout = 1;
	} else {
		printf("QuakEMBD - Based on WinQuake %0.3f\n", VERSION);
	}

	oldtime = Sys_FloatTime() - 0.1;
	while (1) {
		// find time spent rendering last frame
		newtime = Sys_FloatTime();
		time = newtime - oldtime;

		if (cls.state == ca_dedicated) {
			// play vcrfiles at max speed
			if (time < sys_ticrate.value && (vcrFile == -1 || recording)) {
				qembd_udelay(1);
				continue;	// not time to run a server only tic yet
			}
			time = sys_ticrate.value;
		}

		if (time > sys_ticrate.value*2)
			oldtime = newtime;
		else
			oldtime += time;

		Host_Frame(time);

#if 0
		// graphic debugging aids
		if (sys_linerefresh.value)
			Sys_LineRefresh ();
#endif
	}
}
