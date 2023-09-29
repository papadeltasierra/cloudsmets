/********************************************************************************************************
 * @file    app_ui.h
 *
 * @brief   This is the header file for app_ui
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

#ifndef _APP_UI_H_
#define _APP_UI_H_

/**********************************************************************
 * CONSTANT
 */
#define LED_GREEN					LED1
#define LED_RED						LED3
#define LED_ON						1
#define LED_OFF						0


/**********************************************************************
 * TYPEDEFS
 */
enum{
	APP_STATE_NORMAL,
	APP_FACTORY_NEW_SET_CHECK,
	APP_FACTORY_NEW_DOING
};


/**********************************************************************
 * FUNCTIONS
 */
void led_blink_start(u32 pin, u8 times, u16 ledOnTime, u16 ledOffTime);
void led_blink_stop(void);

void led_on(u32 pin);
void led_off(u32 pin);

#endif	/* _APP_UI_H_ */
