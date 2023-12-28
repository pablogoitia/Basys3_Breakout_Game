#ifndef __TIMER_H__
#define __TIMER_H__

#include "xparameters.h"

// Timer control constants
#define TIMER_INT	1
#define INTC_IPR	(*((volatile unsigned long *) (XPAR_INTC_0_BASEADDR + 0x04)))
#define INTC_IER	(*((volatile unsigned long *) (XPAR_INTC_0_BASEADDR + 0x08)))
#define INTC_IAR	(*((volatile unsigned long *) (XPAR_INTC_0_BASEADDR + 0x0C)))
#define INTC_MER	(*((volatile unsigned long *) (XPAR_INTC_0_BASEADDR + 0x1C)))
#define TCSR0		(*((volatile unsigned long *) (XPAR_AXI_TIMER_0_BASEADDR + 0x00)))
#define TLR0		(*((volatile unsigned long *) (XPAR_AXI_TIMER_0_BASEADDR + 0x04)))
#define RST_VALUE	100000	// Equals to 1 ms

// Function definition
void print_button(char button);
void init_timer(void);
void myISR(void) __attribute__ ((interrupt_handler));
void timer_ISR(void);
void msleep(int milisegundos);

#endif
