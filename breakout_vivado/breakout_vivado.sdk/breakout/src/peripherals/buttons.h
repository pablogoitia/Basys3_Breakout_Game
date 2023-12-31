#ifndef __BUTTONS_H__
#define __BUTTONS_H__

#define MASK_BUTTONS	0xF

typedef enum buttons {no_button, top_button, left_button, middle_button, right_button, bottom_button} button_t;

/**
 * Initializes the buttons.
 */
void init_buttons();

/**
 * Waits for a button release.
 */
void wait_button();

/**
 * Checks which if the button is pressed.
 * @return 1 if left button is pressed, 2 if top button is pressed, 3 if right button is pressed, 0 otherwise.
 */
button_t check_button();

#endif
