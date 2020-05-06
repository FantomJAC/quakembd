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
#include "timer.h"

/* Note: TARGET_CLOCK <= APBx timer clock */
#define TARGET_CLOCK				(1 * 1000000)

/* Using TIM7(APB1) */
#define TIMx						TIM7
#define TIMx_CLK_ENABLE()			__HAL_RCC_TIM7_CLK_ENABLE()
#define TIMx_IRQn					TIM7_IRQn
#define TIMx_IRQHandler				TIM7_IRQHandler
/* DEBUG */
#define DEBUG_GPIO_CLK_ENABLE()		__HAL_RCC_GPIOG_CLK_ENABLE();
#define DEBUG_GPIO_PORT				GPIOG
#define DEBUG_GPIO_PIN				GPIO_PIN_2

static TIM_HandleTypeDef timer = {
	.Instance = TIMx,
	.Init = {
		.CounterMode = TIM_COUNTERMODE_UP,
		.Period = 1000,
		.ClockDivision = TIM_CLOCKDIVISION_DIV1,
		.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE
	}
};

static uint32_t tick = 0;

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
#ifdef DEBUG_GPIO_PIN
	GPIO_InitTypeDef gpio_cfg = {
		.Pin = DEBUG_GPIO_PIN,
		.Mode = GPIO_MODE_OUTPUT_PP,
		.Pull = GPIO_PULLUP,
		.Speed = GPIO_SPEED_LOW
	};
#endif

	/* Enable Clocks */
	TIMx_CLK_ENABLE();
	__HAL_RCC_TIM6_CLK_ENABLE();

#ifdef DEBUG_GPIO_PIN
	DEBUG_GPIO_CLK_ENABLE();
	HAL_GPIO_Init(DEBUG_GPIO_PORT, &gpio_cfg);
#endif

	HAL_NVIC_SetPriority(TIMx_IRQn, 0, 1);
	HAL_NVIC_EnableIRQ(TIMx_IRQn);
}

void TIMx_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&timer);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	tick++;
}

uint64_t qembd_get_us_time()
{
	uint16_t cnt = __HAL_TIM_GET_COUNTER(&timer);
	return tick * 1000 + cnt;
}

void qembd_udelay(uint32_t us)
{
	// TODO
	return;
}

void timer_setup()
{
	HAL_StatusTypeDef status;

	timer.Init.Prescaler = (uint32_t) (((SystemCoreClock / 2) / TARGET_CLOCK) - 1); 
	status = HAL_TIM_Base_Init(&timer);
	bail_if_error(status, HAL_OK, "HAL_TIM_Base_Init");
	status = HAL_TIM_Base_Start_IT(&timer);
	bail_if_error(status, HAL_OK, "HAL_TIM_Base_Start");

	return;

bail:
	error_loop();
}

#if 0
void delay_us(uint16_t us)
{
#ifdef DEBUG_GPIO_PIN
	HAL_GPIO_WritePin(DEBUG_GPIO_PORT, DEBUG_GPIO_PIN, GPIO_PIN_SET);
#endif
	__HAL_TIM_SET_COUNTER(&timer, 0);
	while (__HAL_TIM_GET_COUNTER(&timer) < us);
#ifdef DEBUG_GPIO_PIN
	HAL_GPIO_WritePin(DEBUG_GPIO_PORT, DEBUG_GPIO_PIN, GPIO_PIN_RESET);
#endif
}
#endif