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

#ifndef __QUAKEMBD_H
#define __QUAKEMBD_H

#include <stdint.h>
#include <stdlib.h>

typedef struct {
	uint8_t code;
	uint8_t down;
} key_event_t;

typedef struct {
	int x;
	int y;
} mouse_position_t;

int qembd_get_width();
int qembd_get_height();
void qembd_vidinit();
void qembd_fillrect(uint8_t *src, uint32_t *clut, uint16_t x, uint16_t y, uint16_t xsize, uint16_t ysize);
void qembd_refresh();
uint64_t qembd_get_us_time();
void qembd_udelay(uint32_t us);
void *qembd_allocmain(size_t size);
int qembd_main(int c, char **v);
int qembd_dequeue_key_event(key_event_t *e);
int qembd_get_current_position(mouse_position_t *position);

#endif /* __QUAKEMBD_H */