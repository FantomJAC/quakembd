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
#include <otm8009a.h>

#define DISPLAY_FRAME_BUFFER LCD_LAYER_0_ADDRESS

#define DISPLAY_VSYNC 1
#define DISPLAY_VBP 1
#define DISPLAY_VFP 1
#define DISPLAY_VACT DISPLAY_HEIGHT
#define DISPLAY_HSYNC 1
#define DISPLAY_HBP 1
#define DISPLAY_HFP 1
#define DISPLAY_HACT DISPLAY_WIDTH

#define LCD_RESET_PIN                    GPIO_PIN_3
#define LCD_RESET_PULL                   GPIO_NOPULL
#define LCD_RESET_GPIO_PORT              GPIOG
#define LCD_RESET_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOG_CLK_ENABLE()
#define LCD_RESET_GPIO_CLK_DISABLE()     __HAL_RCC_GPIOG_CLK_DISABLE()

static LTDC_HandleTypeDef ltdc = {
	.Instance = LTDC,
	.Init = {
		.HSPolarity = LTDC_HSPOLARITY_AL,
		.VSPolarity = LTDC_VSPOLARITY_AL,
		.DEPolarity = LTDC_DEPOLARITY_AL,
		.PCPolarity = LTDC_PCPOLARITY_IPC,
		.HorizontalSync = DISPLAY_HSYNC,
		.VerticalSync = DISPLAY_VSYNC,
		.AccumulatedHBP = DISPLAY_HSYNC + DISPLAY_HBP,
		.AccumulatedVBP = DISPLAY_VSYNC + DISPLAY_VBP,
		.AccumulatedActiveW = DISPLAY_HSYNC + DISPLAY_HBP + DISPLAY_HACT,
		.AccumulatedActiveH = DISPLAY_VSYNC + DISPLAY_VBP + DISPLAY_VACT,
		.TotalWidth = DISPLAY_HSYNC + DISPLAY_HBP + DISPLAY_HACT + DISPLAY_HFP,
		.TotalHeigh = DISPLAY_VSYNC + DISPLAY_VBP + DISPLAY_VACT + DISPLAY_VFP,
		.Backcolor = {
			.Blue = 0,
			.Green = 0,
			.Red = 0
		}
	}
};

static DSI_HandleTypeDef dsi = {
	.Instance = DSI,
	.Init = {
		.TXEscapeCkdiv = 0x4,
		.NumberOfLanes = DSI_TWO_DATA_LANES
	}
};

static OTM8009A_Object_t otm8009;

/* LCD clock configuration */
/* PLL3_VCO Input = HSE_VALUE/PLL3M = 5 Mhz */
/* PLL3_VCO Output = PLL3_VCO Input * PLL3N = 800 Mhz */
/* PLLLCDCLK = PLL3_VCO Output/PLL3R = 800/19 = 42 Mhz */
/* LTDC clock frequency = PLLLCDCLK = 42 Mhz */
static RCC_PeriphCLKInitTypeDef lcd_clock_config = {
	.PeriphClockSelection = RCC_PERIPHCLK_LTDC,
	.PLL3 = {
		.PLL3M = 5,
		.PLL3N = 160,
		.PLL3FRACN = 0,
		.PLL3P = 2,
		.PLL3Q = 2,
		.PLL3R = 19
	}
};
static DSI_PLLInitTypeDef dsi_pll_config = {
	.PLLNDIV  = 100,
	.PLLIDF   = DSI_PLL_IN_DIV5,
	.PLLODF  = DSI_PLL_OUT_DIV1
};
static DSI_CmdCfgTypeDef dsi_command_config = {
	.VirtualChannelID = 0,
	.ColorCoding = DSI_RGB888,
	.CommandSize = DISPLAY_HACT,
	.TearingEffectSource = DSI_TE_DSILINK,
	.TearingEffectPolarity = DSI_TE_RISING_EDGE,
	.HSPolarity = DSI_HSYNC_ACTIVE_HIGH,
	.VSPolarity = DSI_VSYNC_ACTIVE_HIGH,
	.DEPolarity = DSI_DATA_ENABLE_ACTIVE_HIGH,
	.VSyncPol = DSI_VSYNC_FALLING,
	.AutomaticRefresh = DSI_AR_DISABLE,
	.TEAcknowledgeRequest = DSI_TE_ACKNOWLEDGE_ENABLE
};
static DSI_PHY_TimerTypeDef dsi_phy_config = {
	.ClockLaneHS2LPTime = 35,
	.ClockLaneLP2HSTime = 35,
	.DataLaneHS2LPTime = 35,
	.DataLaneLP2HSTime = 35,
	.DataLaneMaxReadTime = 0,
	.StopWaitTime = 10
};

static LTDC_LayerCfgTypeDef layer_config = {
	.WindowX0 = 0,
	.WindowX1 = DISPLAY_WIDTH,
	.WindowY0 = 0,
	.WindowY1 = DISPLAY_HEIGHT,
	.PixelFormat = LTDC_PIXEL_FORMAT_ARGB8888,
	.Alpha = 255,
	.Alpha0 = 0,
	.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA,
	.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA,
	.FBStartAdress = DISPLAY_FRAME_BUFFER,
	.ImageWidth = DISPLAY_WIDTH,
	.ImageHeight = DISPLAY_HEIGHT,
	.Backcolor = {
		.Blue = 0,
		.Green = 0,
		.Red = 0
	}
};

static DMA2D_HandleTypeDef dma2d = {
	.Instance = DMA2D,
	.Init = {
		.Mode = DMA2D_M2M_PFC,
		.ColorMode = DMA2D_OUTPUT_ARGB8888,
		.OutputOffset = 0,
		.AlphaInverted = DMA2D_REGULAR_ALPHA,  /* No Output Alpha Inversion*/  
		.RedBlueSwap = DMA2D_RB_REGULAR     /* No Output Red & Blue swap */    
	},
	.LayerCfg = {
		{0},
		{
			.InputOffset = 0,
			.InputColorMode = DMA2D_INPUT_L8,
			.AlphaMode = DMA2D_REPLACE_ALPHA,
			.InputAlpha = 0xFF,
			.AlphaInverted = DMA2D_REGULAR_ALPHA, /* No ForeGround Alpha inversion */
			.RedBlueSwap = DMA2D_RB_REGULAR /* No ForeGround Red/Blue swap */
		}
	}
};

static volatile int refresh_pending = 0;

void HAL_DSI_EndOfRefreshCallback(DSI_HandleTypeDef *hdsi)
{
	refresh_pending = 0;
}

void LCD_MspInit(void)
{
	/** @brief Enable the LTDC clock */
	__HAL_RCC_LTDC_CLK_ENABLE();

	/** @brief Toggle Sw reset of LTDC IP */
	__HAL_RCC_LTDC_FORCE_RESET();
	__HAL_RCC_LTDC_RELEASE_RESET();

	/** @brief Enable the DMA2D clock */
	__HAL_RCC_DMA2D_CLK_ENABLE();

	/** @brief Toggle Sw reset of DMA2D IP */
	__HAL_RCC_DMA2D_FORCE_RESET();
	__HAL_RCC_DMA2D_RELEASE_RESET();

	/** @brief Enable DSI Host and wrapper clocks */
	__HAL_RCC_DSI_CLK_ENABLE();

	/** @brief Soft Reset the DSI Host and wrapper */
	__HAL_RCC_DSI_FORCE_RESET();
	__HAL_RCC_DSI_RELEASE_RESET();

	/** @brief NVIC configuration for LTDC interrupt that is now enabled */
	HAL_NVIC_SetPriority(LTDC_IRQn, 9, 0xf);
	HAL_NVIC_EnableIRQ(LTDC_IRQn);

	/** @brief NVIC configuration for DMA2D interrupt that is now enabled */
	HAL_NVIC_SetPriority(DMA2D_IRQn, 9, 0xf);
	HAL_NVIC_EnableIRQ(DMA2D_IRQn);

	/** @brief NVIC configuration for DSI interrupt that is now enabled */
	HAL_NVIC_SetPriority(DSI_IRQn, 9, 0xf);
	HAL_NVIC_EnableIRQ(DSI_IRQn);
}

void DSI_IRQHandler(void)
{
	HAL_DSI_IRQHandler(&dsi);
}

void DMA2D_IRQHandler(void)
{
	HAL_DMA2D_IRQHandler(&dma2d);  
}

static int32_t DSI_IO_Write(uint16_t ChannelNbr, uint16_t Reg, uint8_t *pData, uint16_t Size)
{
	int32_t ret = BSP_ERROR_BUS_FAILURE;
	HAL_StatusTypeDef status;

	if (Size <= 1U) {
		status = HAL_DSI_ShortWrite(&dsi, ChannelNbr, DSI_DCS_SHORT_PKT_WRITE_P1, Reg, (uint32_t)pData[Size]);
		bail_if_error(status, HAL_OK, "HAL_DSI_ShortWrite");
	} else {
		status = HAL_DSI_LongWrite(&dsi, ChannelNbr, DSI_DCS_LONG_PKT_WRITE, Size, (uint32_t)Reg, pData);
		bail_if_error(status, HAL_OK, "HAL_DSI_LongWrite");
	}

	ret = BSP_ERROR_NONE;

bail:
	return ret;
}

static int32_t DSI_IO_Read(uint16_t ChannelNbr, uint16_t Reg, uint8_t *pData, uint16_t Size)
{
	int32_t ret = BSP_ERROR_BUS_FAILURE;
	HAL_StatusTypeDef status;

	status = HAL_DSI_Read(&dsi, ChannelNbr, pData, Size, DSI_DCS_SHORT_PKT_READ, Reg, pData);
	bail_if_error(status, HAL_OK, "HAL_DSI_Read");

	ret = BSP_ERROR_NONE;

bail:
	return ret;
}

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
	GPIO_InitTypeDef gpio_config = {
		.Pin = LCD_RESET_PIN,
		.Mode = GPIO_MODE_OUTPUT_PP,
		.Pull = GPIO_PULLUP,
		.Speed = GPIO_SPEED_FREQ_VERY_HIGH
	};
	DSI_LPCmdTypeDef dsi_command;
	OTM8009A_IO_t otm8009_io_config = {
		.Address = 0,
		.WriteReg = DSI_IO_Write,
		.ReadReg = DSI_IO_Read,
		.GetTick = HAL_GetTick
	};

	LCD_RESET_GPIO_CLK_ENABLE();
	/* Configure the GPIO Reset pin */
	HAL_GPIO_Init(LCD_RESET_GPIO_PORT , &gpio_config);

	/* Toggle Hardware Reset of the DSI LCD using its XRES signal (active low) */
	/* Activate XRES active low */
	HAL_GPIO_WritePin(LCD_RESET_GPIO_PORT , LCD_RESET_PIN, GPIO_PIN_RESET);
	HAL_Delay(20);/* wait 20 ms */
	HAL_GPIO_WritePin(LCD_RESET_GPIO_PORT , LCD_RESET_PIN, GPIO_PIN_SET);/* Deactivate XRES */
	HAL_Delay(10);/* Wait for 10ms after releasing XRES before sending commands */

	/* Call first MSP Initialize only in case of first initialization
	* This will set IP blocks LTDC, DSI and DMA2D
	* - out of reset
	* - clocked
	* - NVIC IRQ related to IP blocks enabled
	*/
	LCD_MspInit();

	/* LCD clock configuration */
	HAL_RCCEx_PeriphCLKConfig(&lcd_clock_config);

	HAL_DSI_DeInit(&dsi);
	HAL_DSI_Init(&dsi, &dsi_pll_config);

	/* Configure the DSI for Command mode */
	HAL_DSI_ConfigAdaptedCommandMode(&dsi, &dsi_command_config);

	dsi_command.LPGenShortWriteNoP    = DSI_LP_GSW0P_ENABLE;
	dsi_command.LPGenShortWriteOneP   = DSI_LP_GSW1P_ENABLE;
	dsi_command.LPGenShortWriteTwoP   = DSI_LP_GSW2P_ENABLE;
	dsi_command.LPGenShortReadNoP     = DSI_LP_GSR0P_ENABLE;
	dsi_command.LPGenShortReadOneP    = DSI_LP_GSR1P_ENABLE;
	dsi_command.LPGenShortReadTwoP    = DSI_LP_GSR2P_ENABLE;
	dsi_command.LPGenLongWrite        = DSI_LP_GLW_ENABLE;
	dsi_command.LPDcsShortWriteNoP    = DSI_LP_DSW0P_ENABLE;
	dsi_command.LPDcsShortWriteOneP   = DSI_LP_DSW1P_ENABLE;
	dsi_command.LPDcsShortReadNoP     = DSI_LP_DSR0P_ENABLE;
	dsi_command.LPDcsLongWrite        = DSI_LP_DLW_ENABLE;
	HAL_DSI_ConfigCommand(&dsi, &dsi_command);

	/* Initialize LTDC */
	HAL_LTDC_DeInit(&ltdc);
	HAL_LTDC_Init(&ltdc);

	/* Start DSI */
	HAL_DSI_Start(&dsi);

	/* Configure DSI PHY HS2LP and LP2HS timings */
	HAL_DSI_ConfigPhyTimer(&dsi, &dsi_phy_config);

	/* Initialize the OTM8009A LCD Display IC Driver (KoD LCD IC Driver) */
	OTM8009A_RegisterBusIO(&otm8009, &otm8009_io_config);
	OTM8009A_Init(&otm8009, OTM8009A_COLMOD_RGB888, OTM8009A_ORIENTATION_LANDSCAPE);

	dsi_command.LPGenShortWriteNoP    = DSI_LP_GSW0P_DISABLE;
	dsi_command.LPGenShortWriteOneP   = DSI_LP_GSW1P_DISABLE;
	dsi_command.LPGenShortWriteTwoP   = DSI_LP_GSW2P_DISABLE;
	dsi_command.LPGenShortReadNoP     = DSI_LP_GSR0P_DISABLE;
	dsi_command.LPGenShortReadOneP    = DSI_LP_GSR1P_DISABLE;
	dsi_command.LPGenShortReadTwoP    = DSI_LP_GSR2P_DISABLE;
	dsi_command.LPGenLongWrite        = DSI_LP_GLW_DISABLE;
	dsi_command.LPDcsShortWriteNoP    = DSI_LP_DSW0P_DISABLE;
	dsi_command.LPDcsShortWriteOneP   = DSI_LP_DSW1P_DISABLE;
	dsi_command.LPDcsShortReadNoP     = DSI_LP_DSR0P_DISABLE;
	dsi_command.LPDcsLongWrite        = DSI_LP_DLW_DISABLE;
	HAL_DSI_ConfigCommand(&dsi, &dsi_command);

	HAL_DSI_ConfigFlowControl(&dsi, DSI_FLOW_CONTROL_BTA);
	HAL_DSI_ForceRXLowPower(&dsi, ENABLE);

	__HAL_DSI_WRAPPER_DISABLE(&dsi);

	/* Layer Init */
	HAL_LTDC_ConfigLayer(&ltdc, &layer_config, 0);

	__HAL_DSI_WRAPPER_ENABLE(&dsi);
}

void qembd_fillrect(uint8_t *src, uint32_t *clut, uint16_t x, uint16_t y, uint16_t xsize, uint16_t ysize)
{
	DMA2D_CLUTCfgTypeDef clut_cfg = {
		.pCLUT = clut,
		.CLUTColorMode = DMA2D_CCM_ARGB8888,
		.Size = (256 - 1)	/* The number of CLUT entries is equal to CS[7:0] + 1 */
	};
	uint32_t pd = (uint32_t) DISPLAY_FRAME_BUFFER + (y * DISPLAY_WIDTH + x) * 4;
	uint32_t ps = (uint32_t) src + (y * DISPLAY_WIDTH + x) * 4;
	HAL_StatusTypeDef status;

	dma2d.Init.OutputOffset = DISPLAY_WIDTH - xsize;

	status = HAL_DMA2D_Init(&dma2d);
	bail_if_error(status, HAL_OK, "HAL_DMA2D_Init");

	status = HAL_DMA2D_ConfigLayer(&dma2d, 1);
	bail_if_error(status, HAL_OK, "HAL_DMA2D_ConfigLayer");

	status = HAL_DMA2D_CLUTLoad(&dma2d, clut_cfg, 1);
	bail_if_error(status, HAL_OK, "HAL_DMA2D_CLUTLoad");
	HAL_DMA2D_PollForTransfer(&dma2d, 100);

	status = HAL_DMA2D_Start(&dma2d, ps, pd, xsize, ysize);
	bail_if_error(status, HAL_OK, "HAL_DMA2D_Start");
	HAL_DMA2D_PollForTransfer(&dma2d, 100);

bail:
	return;
}

void qembd_refresh()
{
	refresh_pending = 1;
	HAL_DSI_Refresh(&dsi);
	while (refresh_pending)
		;
}
