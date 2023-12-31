#include <stdio.h>
#include "xparameters.h"
#include "buttons.h"
#include "timer.h"

volatile int *gpio0 = (int *)XPAR_AXI_GPIO_0_BASEADDR; // dir base buttons

void init_buttons()
{
	// Configurar 4 bits como entrada
	gpio0[1] = MASK_BUTTONS;
}

button_t check_button()
{
	int data;
	button_t btn = no_button; // Default: no button pressed

	// Checks if any button is pressed
	if ((data = gpio0[0] & MASK_BUTTONS) != 0)
	{
		if ((data & 0x2) != 0)
			btn = left_button;
		else if ((data & 0x4) != 0)
			btn = right_button;
		else if ((data & 0x1) != 0)
			btn = top_button;
	}

	return btn;
}

void wait_button()
{
	int data;
	while ((data = gpio0[0] & MASK_BUTTONS) != 0)
		msleep(10);
}
