#include <stdbool.h>

#ifndef __COLORS_H__
#define __COLORS_H__

// General colors
#define W {0xff, 0xff, 0xff}
#define G {0xc2, 0x99, 0x27}
#define B {0xb4, 0xe2, 0xf4}
#define N {0x00, 0x00, 0x00}

// Level selection number colors
#define GO {0x3d, 0x3d, 0x3d}
#define GI {0x78, 0x78, 0x78}
#define GC {0xb0, 0xac, 0xb0}

// Smoke colors
#define G_GC {0xb4, 0xb4, 0xb4}
#define G_GO {0x46, 0x46, 0x46}
#define G_BL {0x00, 0x00, 0x00}

// Heart colors
#define H_LR {0xfc, 0xb2, 0xb7}
#define H_MR {0xef, 0x1c, 0x26}
#define H_DR {0x7a, 0x0a, 0x11}
#define H_BL {0x00, 0x00, 0x00}

// Available colors exclusive for the map
#define M_RED       {0xee, 0x48, 0x4c}
#define M_ORANGE    {0xde, 0x6a, 0x35}
#define M_YELLOW    {0xf3, 0xc0, 0x01}
#define M_GREEN     {0x53, 0xbe, 0x42}
#define M_DBLUE     {0x3f, 0x41, 0xfd}
#define M_PURPLE    {0x87, 0x48, 0xf1}
#define M_CYAN      {0x02, 0xf3, 0xf3}
#define M_DGREY     {0x46, 0x46, 0x46}

typedef struct color {
	unsigned char r, g, b;
} color_t;

/**
 * @brief Compares two color_t objects for equality.
 *
 * This function compares two color_t objects, color1 and color2, and returns true if they are equal, and false otherwise.
 *
 * @param color_1 The first color_t object to compare.
 * @param color_2 The second color_t object to compare.
 * @return true if color1 is equal to color2, false otherwise.
 */
bool equals_color(color_t color_1, color_t color_2);

#endif