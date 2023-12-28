#include <stdio.h>
#include <sleep.h>
#include "graphics.h"

// #############################
// ###   GENERIC FUNCTIONS   ###
// #############################

void print_background(color_t color) {
	color_t negro = N;
    for(int i=0;i<160;i++)
    	for(int j=0;j<120;j++)
    		paint(i,j,negro);
}

void paint_object(position_t pos, color_t *object, int rows, int cols) {
    // Process the array elements
    for (int i = 0; i < rows; i++)
        for (int j = 0; j < cols; j++)
			paint(pos.x + j, pos.y + i, object[i * cols + j]);
}

void paint_animation(position_t pos, color_t *animation, int frames, int period, int rows, int cols) {
    // Process the frames
    for (int f = 0; f < frames; f++) {
        paint_object(pos, &animation[f * rows * cols], rows, cols);
        usleep(period);
    }
}

void paint(int x, int y, color_t rgb) {
	int *ptr = (int *)VGA_CTRL_BASE;
	int val = (rgb.r>>4) | (rgb.b&0xf0) | ((rgb.g&0xf0)<<4);
	ptr[(y<<8)| x]=val;
}

void rect (position_t pos, color_t col, int w, int h) {
	int i,j;
	for(i=0;i<w;i++)
		for(j=0;j<h;j++)
			paint(pos.x+i,pos.y+j,col);
}

color_t get_color(int x, int y) {
	color_t col;
	int *ptr = (int*) VGA_CTRL_BASE;
	int val = ptr[(y<<8) | x];
	col.r = (val&0xf)<<4;
	col.b = (val&0xf0);
	col.g = (val&0xf00)>>4;
	return col;
}

// ###################################
// ###   CUSTOM GRAPHIC ELEMENTS   ###
// ###################################

void print_frame() {
	position_t frames[4];
	frames[0].x = INT_X_BORDER; frames[0].y = INT_Y_BORDER;
	frames[1].x = END_X_BORDER; frames[1].y = INT_Y_BORDER;
	frames[2].x = INT_X_BORDER; frames[2].y = END_Y_BORDER;
	frames[3].x = INT_X_BORDER; frames[3].y = INT_Y_BORDER;
	
	rect(frames[0], blanco, END_X_BORDER, BORDER_THICKNESS);
	rect(frames[1], blanco, BORDER_THICKNESS, END_Y_BORDER);
	rect(frames[2], blanco, END_X_BORDER, BORDER_THICKNESS);
	rect(frames[3], blanco, BORDER_THICKNESS, END_Y_BORDER);
}

void print_lives(position_t pos, int lifes) {
	position_t lives_pos = {LIVES_POS_X, LIVES_POS_Y};

	for(int i=0; i < lifes; i++) {
		pos.x += i * (HEART_WIDTH + HEART_DISTANCE);
		paint_object(lives_pos, *heart, HEART_HEIGHT, HEART_WIDTH);
	}
}
