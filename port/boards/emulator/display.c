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

#include <quakembd.h>
#include <MiniFB.h>

#define	DISPLAY_WIDTH 800
#define	DISPLAY_HEIGHT 480

static struct Window *window;
static uint32_t buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT];

int qembd_get_width()
{
	return DISPLAY_WIDTH;
}

int qembd_get_height()
{
	return DISPLAY_HEIGHT;
}

void qembd_vidinit()
{
	window = mfb_open_ex("Quake", DISPLAY_WIDTH, DISPLAY_HEIGHT, WF_BORDERLESS);
	if (!window)
		qembd_error("Can't create Window");
}

void qembd_fillrect(uint8_t *src, uint32_t *clut, uint16_t x, uint16_t y, uint16_t xsize, uint16_t ysize)
{
	int offset;
	int px;
	int py;

	for (py = 0; py < ysize; py++) {
		offset = (y + py) * DISPLAY_WIDTH + x;
		for (px = 0; px < xsize; px++) {
			buffer[offset + px] = clut[src[offset + px]];
		}
	}
}

void qembd_refresh()
{
	if (window)
		mfb_update(window, buffer);
}