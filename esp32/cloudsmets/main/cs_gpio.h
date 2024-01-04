/*
 * Copyright (c) 2023 Paul D.Smith (paul@pauldsmith.org.uk).
 * License: Free to copy providing the author is acknowledged.
 */
#pragma once

/**
 * There are three ESP32c3 GPIO lines used by the T-Zigbee
 * - BLUE_LED is an LED that can be turned on/off by ESP32c3 programming
 * - TLSR8258_POWER turns no/off the power to the TLSR8258; an additional blue
 *   LED gets turned on when tlsr8258 power is on
 * - CLEAR_KEY is an input line that is driven low when the key is pressed;
 *   CloudSMETS uses this to enable (factory) reset function.
*/
#define BLUE_LED            GPIO_NUM_3
#define TLSR8258_POWER      GPIO_NUM_0
#define CLEAR_KEY           GPIO_NUM_2

