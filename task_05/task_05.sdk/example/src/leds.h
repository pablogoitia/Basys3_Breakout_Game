#ifndef __LEDS_H__
#define __LEDS_H__

#include "xparameters.h"

#define NUM_LEDS 		16
#define MAX_LED 		0x8000
#define LEDS_ADDR 		XPAR_AXI_GPIO_1_BASEADDR
#define MASK_BUTTONS	0xF

// Function definition
void init_leds();
void led_loop(int times);

#endif
