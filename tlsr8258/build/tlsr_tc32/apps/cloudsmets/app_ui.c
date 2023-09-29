/********************************************************************************************************
 * @file    app_ui.c
 *
 * @brief   This is the source file for app_ui
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

#if (__PROJECT_CLOUDSMETS__)

/**********************************************************************
 * INCLUDES
 */
#include "tl_common.h"
#include "zb_api.h"
#include "zcl_include.h"
#include "cloudsmets.h"
#include "app_ui.h"

/**********************************************************************
 * LOCAL CONSTANTS
 */


/**********************************************************************
 * TYPEDEFS
 */


/**********************************************************************
 * LOCAL FUNCTIONS
 */
void led_on(u32 pin)
{
	drv_gpio_write(pin, LED_ON);
}

void led_off(u32 pin)
{
	drv_gpio_write(pin, LED_OFF);
}


s32 zclLEDTimerCb(void *arg)
{
	u32 interval = 0;

	if(g_switchAppCtx.sta == g_switchAppCtx.oriSta){
		g_switchAppCtx.times--;
		if(g_switchAppCtx.times <= 0){
			g_switchAppCtx.timerLedEvt = NULL;
			return -1;
		}
	}

	g_switchAppCtx.sta = !g_switchAppCtx.sta;
	if(g_switchAppCtx.sta){
		light_on();
		interval = g_switchAppCtx.ledOnTime;
	}else{
		light_off();
		interval = g_switchAppCtx.ledOffTime;
	}

	return interval;
}

void led_blink_start(u32 pin, u8 times, u16 ledOnTime, u16 ledOffTime)
{
	u32 interval = 0;
	g_switchAppCtx.times = times;

	if(!g_switchAppCtx.timerLedEvt){
		if(g_switchAppCtx.oriSta){
			light_off();
			g_switchAppCtx.sta = 0;
			interval = ledOffTime;
		}else{
			led_on(pin);
			g_switchAppCtx.sta = 1;
			interval = ledOnTime;
		}
		g_switchAppCtx.ledOnTime = ledOnTime;
		g_switchAppCtx.ledOffTime = ledOffTime;

		g_switchAppCtx.timerLedEvt = TL_ZB_TIMER_SCHEDULE(zclLightTimerCb, pin, interval);
	}
}

void led_blink_stop(u32 pin)
{
	if(g_switchAppCtx.timerLedEvt){
		TL_ZB_TIMER_CANCEL(&g_switchAppCtx.timerLedEvt);

		g_switchAppCtx.times = 0;
		if(g_switchAppCtx.oriSta){
			led_on(pin);
		}else{
			led_off(pin);
		}
	}
}


#endif  /* __PROJECT_CLOUDSMETS__ */
