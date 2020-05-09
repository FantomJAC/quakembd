
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

#include "common.h"

#include <quakembd.h>
#include <ff.h>
#include "sd_diskio.h"
#include "timer.h"

#define MAX_KEY_EVENTS 10

static FATFS fatfs;
static char sd_path[4];

static key_event_t key_events[MAX_KEY_EVENTS];
static size_t key_head = 0;
static size_t key_tail = 0;

#define key_enqueue(c, d) do { \
	key_events[key_head].code = (c); \
	key_events[key_head].down = (d); \
	key_head = (key_head + 1) % MAX_KEY_EVENTS; \
} while (0)

static void CPU_CACHE_Enable(void)
{
	/* Enable I-Cache */
	SCB_EnableICache();

	/* Enable D-Cache */
	SCB_EnableDCache();
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 400000000 (CM7 CPU Clock)
  *            HCLK(Hz)                       = 200000000 (CM4 CPU, AXI and AHBs Clock)
  *            AHB Prescaler                  = 2
  *            D1 APB3 Prescaler              = 2 (APB3 Clock  100MHz)
  *            D2 APB1 Prescaler              = 2 (APB1 Clock  100MHz)
  *            D2 APB2 Prescaler              = 2 (APB2 Clock  100MHz)
  *            D3 APB4 Prescaler              = 2 (APB4 Clock  100MHz)
  *            HSE Frequency(Hz)              = 25000000
  *            PLL_M                          = 5
  *            PLL_N                          = 160
  *            PLL_P                          = 2
  *            PLL_Q                          = 4
  *            PLL_R                          = 2
  *            VDD(V)                         = 3.3
  *            Flash Latency(WS)              = 4
  * @param  None
  * @retval None
  */
void SystemClock_Config(void)
{
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;
	HAL_StatusTypeDef ret = HAL_OK;

	/*!< Supply configuration update enable */
	HAL_PWREx_ConfigSupply(PWR_DIRECT_SMPS_SUPPLY);

	/* The voltage scaling allows optimizing the power consumption when the device is 
		clocked below the maximum system frequency, to update the voltage scaling value 
		regarding system frequency refer to product datasheet.  */
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

	/* Enable HSE Oscillator and activate PLL with HSE as source */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
	RCC_OscInitStruct.CSIState = RCC_CSI_OFF;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;

	RCC_OscInitStruct.PLL.PLLM = 5;
	RCC_OscInitStruct.PLL.PLLN = 160;
	RCC_OscInitStruct.PLL.PLLFRACN = 0;
	RCC_OscInitStruct.PLL.PLLP = 2;
	RCC_OscInitStruct.PLL.PLLR = 2;
	RCC_OscInitStruct.PLL.PLLQ = 4;

	RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
	RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
	ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
	if (ret != HAL_OK) {
		error_loop();
	}

	/* Select PLL as system clock source and configure  bus clocks dividers */
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_D1PCLK1 | RCC_CLOCKTYPE_PCLK1 | \
									RCC_CLOCKTYPE_PCLK2  | RCC_CLOCKTYPE_D3PCLK1);

	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;  
	RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2; 
	RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2; 
	RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2; 
	ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4);
	if (ret != HAL_OK) {
		error_loop();
	}

	/*
	Note : The activation of the I/O Compensation Cell is recommended with communication  interfaces
			(GPIO, SPI, FMC, QSPI ...)  when  operating at  high frequencies(please refer to product datasheet)       
			The I/O Compensation Cell activation  procedure requires :
		- The activation of the CSI clock
		- The activation of the SYSCFG clock
		- Enabling the I/O Compensation Cell : setting bit[0] of register SYSCFG_CCCSR
	*/

	/*activate CSI clock mondatory for I/O Compensation Cell*/  
	__HAL_RCC_CSI_ENABLE() ;
	
	/* Enable SYSCFG clock mondatory for I/O Compensation Cell */
	__HAL_RCC_SYSCFG_CLK_ENABLE() ;

	/* Enables the I/O Compensation Cell */
	HAL_EnableCompensationCell();
}

static void MPU_Config(void)
{
	MPU_Region_InitTypeDef MPU_InitStruct;

	/* Disable the MPU */
	HAL_MPU_Disable();

	/* Configure the MPU attributes as WT for SDRAM */
	MPU_InitStruct.Enable = MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress = SDRAM_DEVICE_ADDR;
	MPU_InitStruct.Size = MPU_REGION_SIZE_32MB;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
	MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
	MPU_InitStruct.Number = MPU_REGION_NUMBER0;
	MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	/* Enable the MPU */
	HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

void BSP_PB_Callback(Button_TypeDef button)
{
	key_enqueue(200, HAL_GPIO_ReadPin(BUTTON_WAKEUP_GPIO_PORT, BUTTON_WAKEUP_PIN) == GPIO_PIN_SET);
}

void BSP_JOY_Callback(JOY_TypeDef joy, uint32_t joy_pin)
{
	uint8_t bind = 0;

	switch (joy_pin) {
	case JOY_SEL:
		bind = 13;
		break;
	case JOY_DOWN:
		bind = 129;
		break;
	case JOY_LEFT:
		bind = 0x2c;
		break;
	case JOY_RIGHT:
		bind = 0x2e;
		break;
	case JOY_UP:
		bind = 128;
		break;
	}

	if (bind != 0)
		key_enqueue(bind, HAL_GPIO_ReadPin(JOY1_SEL_GPIO_PORT, joy_pin << 2) != GPIO_PIN_SET);
}

static void filesystem_init()
{
	FRESULT r;

	FATFS_LinkDriver(&SD_Driver, sd_path);

	BSP_SD_Init(0);
	BSP_SD_DetectITConfig(0);

	if (!BSP_SD_IsDetected(0))
		bail("SD card is not detected");

	r = f_mount(&fatfs, (const TCHAR *) sd_path, 0);
	bail_if_error(r, FR_OK, "Cannot mount");

	return;

bail:
	error_loop();
}

void *qembd_allocmain(size_t size)
{
	return (void *) (0xD0400000U);
}

int qembd_dequeue_key_event(key_event_t *e)
{
	if (key_head == key_tail)
		return -1;

	e->code = key_events[key_tail].code;
	e->down = key_events[key_tail].down;
	key_tail = (key_tail + 1) % MAX_KEY_EVENTS;

	return 0;
}

int qembd_get_current_position(mouse_position_t *position)
{
	TS_State_t state = {0};
	int32_t r;

	r = BSP_TS_GetState(0, &state);
	bail_if_error(r, BSP_ERROR_NONE, "BSP_TS_GetState");

	if (!state.TouchDetected)
		goto bail;

	position->x = (int) state.TouchX;
//	position->y = (int) state.TouchY;

	return 0;

bail:
	return -1;
}

int main()
{
	COM_InitTypeDef com_cfg = {
		.BaudRate = 115200,
		.WordLength = COM_WORDLENGTH_8B,
		.StopBits = COM_STOPBITS_1,
		.Parity = COM_PARITY_NONE,
		.HwFlowCtl = COM_HWCONTROL_NONE
	};
	GPIO_InitTypeDef pb_gpio_cfg = {
		.Pin = BUTTON_WAKEUP_PIN,
		.Mode = GPIO_MODE_IT_RISING_FALLING,
		.Pull = GPIO_NOPULL,
		.Speed = GPIO_SPEED_FREQ_HIGH
	};
	TS_Init_t ts_cfg = {
		.Width = DISPLAY_WIDTH,
		.Height = DISPLAY_HEIGHT,
		.Orientation = TS_SWAP_XY | TS_SWAP_Y,
		.Accuracy = 0
	};
	HAL_StatusTypeDef status;

	/* Configure the MPU attributes as Write Through for SDRAM */
	MPU_Config();
	/* Enable the CPU Cache */
	CPU_CACHE_Enable();
	/* HAL initialization */
	HAL_Init();
	/* Configure the system clock to 400 MHz */
	SystemClock_Config();

	/* Timer initialization */
	timer_setup();

	/* BSP initializations */
	BSP_LED_Init(LED1);
	BSP_LED_Init(LED2);
	BSP_LED_Init(LED3);
	BSP_LED_Init(LED4);
	BSP_PB_Init(BUTTON_WAKEUP, BUTTON_MODE_EXTI);
	/* Tweak default PB behavior */
	HAL_GPIO_Init(BUTTON_WAKEUP_GPIO_PORT, &pb_gpio_cfg);
	BSP_JOY_Init(JOY1, JOY_MODE_EXTI, JOY_ALL);
	BSP_TS_Init(0, &ts_cfg);
	BSP_COM_Init(COM1, &com_cfg);
	BSP_SDRAM_Init(0);

	/* Filesystem initialization */
	filesystem_init();

	/* Run Q1 Engine */
	qembd_main(0, NULL);

	/* Should not be reached here */
	while (1) {
		HAL_Delay(500);
		BSP_LED_Toggle(LED1);
	}
}

void error_loop()
{
	BSP_LED_On(LED4);
	while (1)
		;
}