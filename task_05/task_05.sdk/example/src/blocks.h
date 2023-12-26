#ifndef __BLOCKS_H__
#define __BLOCKS_H__

#include <stdbool.h>

#define RESOLUTION_X	160
#define RESOLUTION_Y	120

// Map properties
#define N_BLOCKS_X      11
#define N_BLOCKS_Y      7

// Block properties
#define BLOCK_LENGTH        12    // by default, 12
#define BLOCK_HEIGHT        3
#define SEPARATION          2
#define BLOCK_COLOR_DIFF    0x28  // decimal: 40

// Technical parameters
#define MAX_COLLISIONS  3

// Available colors exclusive for the map
#define M_RED       {0xee, 0x48, 0x4c}
#define M_ORANGE    {0xde, 0x6a, 0x35}
#define M_YELLOW    {0xf3, 0xc0, 0x01}
#define M_GREEN     {0x53, 0xbe, 0x42}
#define M_DBLUE     {0x3f, 0x41, 0xfd}
#define M_PURPLE    {0x87, 0x48, 0xf1}
#define M_CYAN      {0x02, 0xf3, 0xf3}
#define M_DGREY     {0x46, 0x46, 0x46}
/*
const color_t red = M_RED;
const color_t orange = M_ORANGE;
const color_t yellow = M_YELLOW;
const color_t green = M_GREEN;
const color_t dark_blue = M_DBLUE;
const color_t purple = M_PURPLE;
const color_t cyan = M_CYAN;
*/
typedef struct color {
	unsigned char r, g, b;
} color_t;

typedef struct position {
	int x, y;
} position_t;

// The block structure defines its current properties
typedef struct block {
	color_t color;
    int collisions;
    int i, j;
    position_t location;
    bool indestructible;
} block_t;

// Block/Map printing functions
void print_block(position_t pos, color_t col);
void init_map(block_t map[N_BLOCKS_X][N_BLOCKS_Y]);
void print_map(block_t map[N_BLOCKS_X][N_BLOCKS_Y]);
void reprint_block(block_t block);

#endif
