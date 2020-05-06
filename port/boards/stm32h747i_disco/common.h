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

#ifndef __COMMON_H
#define __COMMON_H

/* CMSIS */
#include <stm32h747xx.h>
/* HAL */
#include <stm32h7xx_hal.h>
/* BSP */
#include <stm32h747i_discovery.h>
#include <stm32h747i_discovery_sdram.h>
/* Common clibs */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define bail_if_error(X, COND, msg) { if ((X) != (COND)) { printf("[STM32_ERROR] " msg ": %d", X); goto bail; } }
#define bail(msg) { printf("[STM32_ERROR] " msg); goto bail; }

void error_loop();

#endif /* __COMMON_H */