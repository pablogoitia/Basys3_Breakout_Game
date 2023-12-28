#include <stdio.h>
#include "timer.h"

#define NUM_LEDS 		16
#define MAX_LED 		0x8000
#define BASE_GPIO1 		0x40010000
#define MASK_BUTTONS	0xF

volatile int *gpio1 = (int*)BASE_GPIO1;

void init_leds() {
	gpio1[1] = 0;			// Configurarlo como salida
}

void led_loop(int times) {
	int led = 0b1;			// Posicion del LED en binario

	for (int t = 0; t < times; t++) {
		for (int l = 0; l < NUM_LEDS; l++) {
			gpio1[0] = led;
			led <<= 1;
			msleep(10);
		}
		for (int l = 0; l < NUM_LEDS; l++) {
			led >>= 1;
			gpio1[0] = led;
			msleep(10);
		}
		gpio1[0] = 0;
	}
}
