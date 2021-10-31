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
#include <ff.h>

#define MAX_FILES 32
static FIL file_rsrc[MAX_FILES];
static uint32_t file_flags = 0;
#define INVALID_HANDLE -1
#define HANDLE_TO_FILE(h) (((0x01 << h) & file_flags) ? &(file_rsrc[(h)]) : NULL)
#define FREE_HANDLE(h) (file_flags &= ~(0x01 << h))

#define Sys_Warning(msg, ...) Sys_DebugLog(NULL, "[WARN]: " msg "\r\n", ##__VA_ARGS__)

static int nextFreeFile()
{
	int handle = 0;
	uint32_t mask = 0x1;

	while (1) {
		if ((mask & file_flags) == 0) {
			file_flags |= mask;
			break;
		}
		mask <<= 1;
		handle++;
		if (handle == MAX_FILES)
			return -1;
	}

	return handle;
}

int Sys_FileOpenRead(char *path, int *handle)
{
	int	h = INVALID_HANDLE;
	FIL *fp;
	FILINFO info;
	FRESULT r;

	r = f_stat(path, &info);
	if (r != FR_OK) {
		Sys_Warning("Cannot f_stat %s", path);
		goto bail;
	}

	h = nextFreeFile();
	if (h < 0) {
		Sys_Warning("Max file limit reached, cannot open %s", path);
		goto bail;
	}

	fp = HANDLE_TO_FILE(h);
	r = f_open(fp, path, FA_READ);
	if (r != FR_OK) {
		Sys_Warning("FatFs error %d (FA_READ), cannot open %s", r, path);
		goto bail;
	}

	*handle = h;

	return info.fsize;

bail:
	if (h != INVALID_HANDLE)
		FREE_HANDLE(h);

	return INVALID_HANDLE;
}

int Sys_FileOpenWrite(char *path)
{
	int h;
	FIL *fp;
	FRESULT r;

	h = nextFreeFile();
	if (h < 0) {
		Sys_Warning("Max file limit reached, cannot open %s", path);
		goto bail;
	}

	fp = HANDLE_TO_FILE(h);
	r = f_open(fp, path, FA_READ | FA_WRITE | FA_CREATE_ALWAYS);
	if (r != FR_OK) {
		Sys_Warning("FatFs error %d (FA_WRITE), cannot open %s", r, path);
		goto bail;
	}

	return h;

bail:
	if (h != INVALID_HANDLE)
		FREE_HANDLE(h);

	return INVALID_HANDLE;
}

void Sys_FileClose(int handle)
{
	FIL *fp = HANDLE_TO_FILE(handle);

	if (!fp)
		return;

	f_close(fp);
	FREE_HANDLE(handle);
}

void Sys_FileSeek(int handle, int position)
{
	FIL *fp = HANDLE_TO_FILE(handle);

	if (!fp)
		return;

	f_lseek(fp, position);
}

int Sys_FileRead(int handle, void *dest, int count)
{
	FIL *fp = HANDLE_TO_FILE(handle);
	int r;

	if (!fp)
		return;

	f_read(fp, dest, count, &r);

	return r;
}

int Sys_FileWrite(int handle, void *src, int count)
{
	FIL *fp = HANDLE_TO_FILE(handle);
	int w;

	if (!fp)
		return;

	f_write(fp, src, count, &w);

	return w;
}

int	Sys_FileTime(char *path)
{
	FILINFO info;
	FRESULT r;

	r = f_stat(path, &info);
	if (r != FR_OK) {
		Sys_Warning("Cannot f_stat %s", path);
		goto bail;
	}

	return info.ftime;

bail:
	return -1;
}

void Sys_mkdir(char *path)
{
	FRESULT r;

	r = f_mkdir(path);
	if (r != FR_OK)
		Sys_Error("Cannot f_mkdir %s", path);
}

void Sys_FileSync(int handle)
{
	FIL *fp = HANDLE_TO_FILE(handle);
	FRESULT r;

	if (!fp)
		return;

	r = f_sync(fp);
	if (r != FR_OK)
		Sys_Error("Cannot f_sync");
}

void Sys_File_gets(int handle, char *buf, int len)
{
	FIL *fp = HANDLE_TO_FILE(handle);
	f_gets(buf, len, fp);
}