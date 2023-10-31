/********************************************************************************************************
 * @file    app_cfg.h
 *
 * @brief   This is the header file for app_cfg
 *
 * @author  Zigbee Group
 * @date    2021
 *
 * @par     Copyright (c) 2021, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *			All rights reserved.
 *
 *          Portions Copyright (c) 2023, Paul D.Smith (pau@pauldsmith.org.uk)
 *
 *          Licensed under the Apache License, Version 2.0 (the "License");
 *          you may not use this file except in compliance with the License.
 *          You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 *          Unless required by applicable law or agreed to in writing, software
 *          distributed under the License is distributed on an "AS IS" BASIS,
 *          WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *          See the License for the specific language governing permissions and
 *          limitations under the License.
 *
 *******************************************************************************************************/

#pragma once

/* Enable C linkage for C++ Compilers: */
#if defined(__cplusplus)
extern "C" {
#endif


/**********************************************************************
 * Version configuration
 */
#include "version_cfg.h"

/**********************************************************************
 * Product Information
 */

/**********************************************************************
 * Debug mode config
 * Currently allowing printf output to tlsr8258 PC4 pin.
 */
#define	UART_PRINTF_MODE				1
#define USB_PRINTF_MODE         		0

/*
 * It seems that the TLSR8258 is not reliable above 115200.  Put simply it will
 * not communicate over the zbhci interface.
 */
#define BAUDRATE						115200

/* PM */
#define PM_ENABLE						0

/* PA */
#define PA_ENABLE						0

/* BDB */
#define TOUCHLINK_SUPPORT				0
// !!PDS: We may want this but not the binding bit?  Perhaps "binding"
//        unless "binding" means connecting with the coordinator.
#define FIND_AND_BIND_SUPPORT			1

/* UART module */
/* CloudSMETS uses the UART to talk to the ESP32-C3 */
#define	MODULE_UART_ENABLE							1

/* The ESP32-C3 drives the tlsr8253 over the zbhci interface over the UART. */
#define ZBHCI_UART									1

#if (ZBHCI_USB_PRINT || ZBHCI_USB_CDC || ZBHCI_USB_HID || ZBHCI_UART)
	#define ZBHCI_EN								1
#endif

/* Board ID */
#define BOARD_826x_EVK					0
#define BOARD_826x_DONGLE				1
#define BOARD_826x_DONGLE_PA			2
#define BOARD_8258_EVK					3
#define BOARD_8258_EVK_V1P2				4//C1T139A30_V1.2
#define BOARD_8258_DONGLE				5
#define BOARD_8278_EVK					6
#define BOARD_8278_DONGLE				7
#define BOARD_B91_EVK					8
#define BOARD_B91_DONGLE				9

/* The LilyGo T-ZigBee board is basically am 8258 dongle without the switches. */
#ifndef MCU_CORE_8258
#error "LilyGo T-ZigBee iuses a Telink tlsr8258."
#endif
#if (CHIP_TYPE == TLSR_8258_1M)
#error "LilyGo T-ZigBee uses a 512kB Telink tlsr8258."
#endif

/* Board define */
#if defined(MCU_CORE_826x)
#if !PA_ENABLE
	#define BOARD						BOARD_826x_DONGLE
#else
	#define BOARD						BOARD_826x_DONGLE_PA
#endif
	#define CLOCK_SYS_CLOCK_HZ  		32000000
#elif defined(MCU_CORE_8258)
#if (CHIP_TYPE == TLSR_8258_1M)
	#define FLASH_CAP_SIZE_1M			1
#endif
	#define BOARD						BOARD_8258_DONGLE//BOARD_8258_EVK
	#define CLOCK_SYS_CLOCK_HZ  		48000000
#elif defined(MCU_CORE_8278)
	#define FLASH_CAP_SIZE_1M			1
	#define BOARD						BOARD_8278_DONGLE//BOARD_8278_EVK
	#define CLOCK_SYS_CLOCK_HZ  		48000000
#elif defined(MCU_CORE_B91)
	#define FLASH_CAP_SIZE_1M			1
	#define BOARD						BOARD_B91_DONGLE//BOARD_B91_EVK
	#define CLOCK_SYS_CLOCK_HZ  		48000000
#else
	#error "MCU is undefined!"
#endif

/* Board include */
#if (BOARD == BOARD_826x_EVK)
	#include "board_826x_evk.h"
#elif (BOARD == BOARD_826x_DONGLE)
	#include "board_826x_dongle.h"
#elif (BOARD == BOARD_826x_DONGLE_PA)
	#include "board_826x_dongle_pa.h"
#elif (BOARD == BOARD_8258_DONGLE)
	#include "board_8258_dongle.h"
#elif (BOARD == BOARD_8258_EVK)
	#include "board_8258_evk.h"
#elif (BOARD == BOARD_8258_EVK_V1P2)
	#include "board_8258_evk_v1p2.h"
#elif (BOARD == BOARD_8278_EVK)
	#include "board_8278_evk.h"
#elif (BOARD == BOARD_8278_DONGLE)
	#include "board_8278_dongle.h"
#elif (BOARD == BOARD_B91_EVK)
	#include "board_b91_evk.h"
#elif (BOARD == BOARD_B91_DONGLE)
	#include "board_b91_dongle.h"
#endif


/* Voltage detect module */
/* If VOLTAGE_DETECT_ENABLE is set,
 * 1) if MCU_CORE_826x is defined, the DRV_ADC_VBAT_MODE mode is used by default,
 * and there is no need to configure the detection IO port;
 * 2) if MCU_CORE_8258 or MCU_CORE_8278 is defined, the DRV_ADC_VBAT_MODE mode is used by default,
 * we need to configure the detection IO port, and the IO must be in a floating state.
 * 3) if MCU_CORE_B91 is defined, the DRV_ADC_BASE_MODE mode is used by default,
 * we need to configure the detection IO port, and the IO must be connected to the target under test,
 * such as VCC.
 */
#define VOLTAGE_DETECT_ENABLE						0

#if defined(MCU_CORE_826x)
	#define VOLTAGE_DETECT_ADC_PIN					0
#elif defined(MCU_CORE_8258) || defined(MCU_CORE_8278)
	#define VOLTAGE_DETECT_ADC_PIN					GPIO_PC5
#elif defined(MCU_CORE_B91)
	#define VOLTAGE_DETECT_ADC_PIN					ADC_GPIO_PB0
#endif

/* Watch dog module */
#define MODULE_WATCHDOG_ENABLE						0


/**********************************************************************
 * ZCL cluster support setting
 * CloudSMETS has to support the "Identify" cluster and the commands of
 * the Smart Energy (SE) "Price" cluster.  All other ZigBee operations are
 * simple attribute reads.
 *
 * Also support ability to upgrade over the HCI.
 */
#define ZCL_IDENTIFY_SUPPORT						1
#define ZCL_PRICE_SUPPORT							1
#define ZCL_HCI_OTA_SUPPORT							1


/**********************************************************************
 * Stack configuration
 */
#include "stack_cfg.h"

/**********************************************************************
 * EV configuration
 */
typedef enum{
	EV_POLL_ED_DETECT,
	EV_POLL_HCI,
    EV_POLL_IDLE,
	EV_POLL_MAX,
}ev_poll_e;

/* Disable C linkage for C++ Compilers: */
#if defined(__cplusplus)
}
#endif
