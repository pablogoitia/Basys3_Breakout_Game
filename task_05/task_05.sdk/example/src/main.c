#include <stdio.h>
#include <sleep.h>
#include <stdbool.h>
#include "xil_printf.h"
#include "main.h"

#define MAX_LED 		0x8000
#define BASE_GPIO0 		0x40000000
#define BASE_GPIO1 		0x40010000
#define MASK_BUTTONS	0xF



color_t negro = N;
color_t blanco = W;
color_t azul = A;
color_t azul_claro = B;
color_t verde = C;
color_t verde_oscuro = D;
color_t amarillo = E;
color_t rojo = F;
color_t gold = G;
color_t gris_claro = GC;
color_t gris_intermedio = GI;
color_t gris_oscuro = GO;

color_t amar = {255, 255, 0};
position_t pos_barra={((RESOLUTION_X / 2) - BAR_LENGTH / 2), 100}; //75
ball_t bola;
block_t map[N_BLOCKS_X][N_BLOCKS_Y];
position_t frames[4];
int bola_activa=0;

volatile int *gpio0 = (int*)BASE_GPIO0; // dir base buttons

int main(){
	int count1, speed_ball;
	init_buttons();
	int dir;
	int lifes;
	int remaining_blocks;
	game_status_t status;
	levels_t level;

	while (1)
	{
		// Init variables
		count1 = 0;
		speed_ball = 0;
		lifes = NUM_LIFES;
		status = continues;
		remaining_blocks = N_BLOCKS_X * N_BLOCKS_Y;

		// Here it goes loading_screen()
		level = level_selection();

		// Prints background
		print_background(negro);

		// Print game borders
		frames[0].x = INT_X_BORDER; frames[0].y = INT_Y_BORDER;
		frames[1].x = END_X_BORDER; frames[1].y = INT_Y_BORDER;
		frames[2].x = INT_X_BORDER; frames[2].y = END_Y_BORDER;
		frames[3].x = INT_X_BORDER; frames[3].y = INT_Y_BORDER;
		rect(frames[0], blanco, END_X_BORDER, BORDER_THICKNESS);
		rect(frames[1], blanco, BORDER_THICKNESS, END_Y_BORDER);
		rect(frames[2], blanco, END_X_BORDER, BORDER_THICKNESS);
		rect(frames[3], blanco, BORDER_THICKNESS, END_Y_BORDER);

		// Print map
		init_map(map, level);
		print_map(map);

		// Pos inicial de la nave
		init_ball();
		move_bar(0);
		bola.mov = 3;

		while (status == continues) {
			// una de cada 5 veces, mueve la nave en el eje X
			if(count1==3){
				count1=0;
				if (((dir = wait_button()) != 0) && (dir == 1 || dir == 3))
					move_bar(dir);
			} else count1++;
			// Mueve la bala cada vuelta del ciclo. La bala va 5 veces m�s r�pido que la nave.
			if(speed_ball == 5){
				speed_ball=0;
				status = move_ball();
				if (status == lost_life)
				{
					count1 = 0;
					speed_ball = 0;
					lifes--;
					if (lifes == 0)
						status = game_over;
					else {
						status = continues;
						life_lost();
					}
				}
				else if (status == block_broken)
				{
					remaining_blocks--;
					xil_printf("Remaining blocks: %d\r", remaining_blocks);
					if (remaining_blocks == 0)
						status = win;
					else
						status = continues;
				}
			} else speed_ball++;
			usleep(10000);
		}

		if (status == game_over)
			xil_printf("GAME OVER\n");	// Aqui va game_over_screen()
		else if (status == win)
			xil_printf("YOU WON\n");	// Aqui va win_screen()
	}

	return 0;
}

void life_lost()
{
	position_t pos = {bola.x - SMOKE_WIDTH / 2, bola.y - SMOKE_HEIGHT - 1};
	paint_animation(pos, smoke, SMOKE_FRAMES, SMOKE_TIME, SMOKE_WIDTH, SMOKE_HEIGHT);
	init_ball();
}

void init_buttons()
{
	// Configurar 4 bits como entrada
	gpio0[1] = MASK_BUTTONS;
}

int wait_button()
{
	int data;
	char btn = 0;

	// Espera a que se pulse un boton.
	if ((data = gpio0[0] & MASK_BUTTONS) != 0) { 	
		if ((data & 0x2) != 0)		// Boton de mover a la izquierda
			btn = 1;
		else if ((data & 0x4) != 0)			// Boton de mover a la derecha
			btn = 3;
		else if ((data & 0x1) != 0)
			btn = 2;


	}
	return btn;
}

int equals_color(color_t color1, color_t color2)
{
	return ((color1.r == color2.r) && (color1.g == color2.g) && (color1.b == color2.b));
}

game_status_t move_ball()
{
	game_status_t status = continues;
	if(bola_activa)
	{
		position_t next_pos;
		side_t side;
		block_t block;

		movement_t next_mov;
		bool is_block = false;
		color_t next_color;
		switch(bola.mov)
		{
			case 0:
				next_pos.x = bola.x+1;
				next_pos.y = bola.y-1;
				break;
			case 1:
				next_pos.x = bola.x+1;
				next_pos.y = bola.y+1;
				break;
			case 2:
				next_pos.x = bola.x-1;
				next_pos.y = bola.y+1;
				break;
			case 3:
				next_pos.x = bola.x-1;
				next_pos.y = bola.y-1;
				break;
		}
		next_color = pixel(next_pos.x, next_pos.y);
		if (!equals_color(negro, next_color))
		{
			if ((side = calculate_border(next_pos)) == not_border)
			{
				if ((is_block = calculate_block(next_pos, &block)))
				{
					if (!block.indestructible)
					{
						block.collisions--;
						if (block.collisions == 0)
						{
							print_block(block.location, negro);
							status = block_broken;
						}
						else
							reprint_block(block);
					}
				}
			}
			else if (side == bottom)
			{
				paint(bola.x, bola.y, negro);
				return lost_life;
			}
			else
				is_block = false;

			next_mov = calculate_rebound(bola, side, is_block, &block, &next_pos);	// lo ha hecho Pablo el cutres
		} else
		{
			next_mov = bola.mov;
		}
		paint(bola.x, bola.y, negro);
		bola.x = next_pos.x;
		bola.y = next_pos.y;
		bola.mov = next_mov;
		paint(bola.x, bola.y, azul_claro);
	}
	return status;
}

side_t calculate_border(position_t next_pos)
{
	bool is_left = false, is_right = false, is_top = false, is_bottom = false;
	side_t res;

	if (next_pos.x == INT_X_BORDER)
		is_left = true;
	if (next_pos.x == END_X_BORDER)
		is_right = true;
	if (next_pos.y == INT_Y_BORDER)
		is_top = true;
	if (next_pos.y == END_Y_BORDER)
		is_bottom = true;

	if (is_left)
		if (is_top)
			res = top_left;
		else if (is_bottom)
			res = bottom_left;
		else
			res = left;
	else if (is_right)
		if (is_top)
			res = top_right;
		else if (is_bottom)
			res = bottom_right;
		else
			res = right;
	else if (is_top)
		res = top;
	else if (is_bottom)
		res = bottom;
	else
		res = not_border;


	return res;
}

side_t which_side_block(position_t next_pos, block_t block)
{
	bool is_left = false, is_right = false, is_top = false, is_bottom = false;
	side_t res;

	if (next_pos.x >= block.location.x && next_pos.x < block.location.x + BLOCK_LENGTH &&
			next_pos.y == block.location.y)
		is_top = true;
	if (next_pos.x >= block.location.x && next_pos.x < block.location.x + BLOCK_LENGTH &&
			next_pos.y == block.location.y + BLOCK_HEIGHT - 1)
		is_bottom = true;
	if (next_pos.x == block.location.x && next_pos.y >= block.location.y &&
			next_pos.y < block.location.y + BLOCK_HEIGHT)
		is_left = true;
	if (next_pos.x == block.location.x + BLOCK_LENGTH - 1 && next_pos.y >= block.location.y &&
			next_pos.y < block.location.y + BLOCK_HEIGHT)
		is_right = true;

	if (is_left)
		if (is_top)
			res = top_left;
		else if (is_bottom)
			res = bottom_left;
		else
			res = left;
	else if (is_right)
		if (is_top)
			res = top_right;
		else if (is_bottom)
			res = bottom_right;
		else
			res = right;
	else if (is_top)
		res = top;
	else if (is_bottom)
		res = bottom;

	return res;
}

side_t which_side_bar(position_t next_pos)
{
	bool is_left = false, is_right = false, is_top = false;
	side_t res;

	if (next_pos.x >= pos_barra.x && next_pos.x < pos_barra.x + BAR_LENGTH &&
			next_pos.y == pos_barra.y)
		is_top = true;
	if (next_pos.x == pos_barra.x && next_pos.y >= pos_barra.y &&
			next_pos.y < pos_barra.y + BAR_HEIGHT)
		is_left = true;
	if (next_pos.x == pos_barra.x + BAR_LENGTH - 1 && next_pos.y >= pos_barra.y &&
			next_pos.y < pos_barra.y + BAR_HEIGHT)
		is_right = true;

	if (is_left)
		if (is_top)
			res = top_left;
		else
			res = left;
	else if (is_right)
		if (is_top)
			res = top_right;
		else
			res = right;
	else if (is_top)
		res = top;

	return res;
}


bool calculate_block(position_t next_pos, block_t *block)
{
	block_t res;
	for (int i = N_BLOCKS_Y - 1; i >= 0; i--)
	{
		for (int j = N_BLOCKS_X - 1; j >= 0; j--)
		{
			res = map[j][i];
			if (next_pos.x >= res.location.x && next_pos.x < res.location.x + BLOCK_LENGTH &&
				next_pos.y >= res.location.y && next_pos.y < res.location.y + BLOCK_HEIGHT)
			{
				*block = res;
				return true;
			}

		}
	}
	return false;
}

movement_t calculate_rebound(ball_t bola, side_t side, bool is_block, block_t *block, position_t *next_pos)
{
	movement_t mov = bola.mov;
	if (side != not_border) 	// border
	{
		switch (side) {
			case top_left:
				mov = mov_bottom_right;
				next_pos->x = bola.x + 1;
				next_pos->y = bola.y + 1;
				break;
			case top_right:
				mov = mov_bottom_left;
				next_pos->x = bola.x - 1;
				next_pos->y = bola.y + 1;
				break;
			case top:
				if (bola.mov == mov_top_right) {
					mov = mov_bottom_right;
					next_pos->x = bola.x + 1;
					next_pos->y = bola.y + 1;
				}
				else {
					mov = mov_bottom_left;
					next_pos->x = bola.x - 1;
					next_pos->y = bola.y + 1;
				}
				break;
			case left:
				if (bola.mov == mov_top_left) {
					mov = mov_top_right;
					next_pos->x = bola.x + 1;
					next_pos->y = bola.y - 1;
				}
				else {
					mov = mov_bottom_right;
					next_pos->x = bola.x + 1;
					next_pos->y = bola.y + 1;
				}
				break;
			case right:
				if (bola.mov == mov_top_right) {
					mov = mov_top_left;
					next_pos->x = bola.x - 1;
					next_pos->y = bola.y - 1;
				}
				else {
					mov = mov_bottom_left;
					next_pos->x = bola.x - 1;
					next_pos->y = bola.y + 1;
				}
				break;
			default:
				break;
		}
	} else if (is_block) 	// block
	{
		switch (which_side_block(*next_pos, *block)) {
			case top_left:
				mov = mov_top_left;
				next_pos->x = bola.x - 1;
				next_pos->y = bola.y - 1;
				break;
			case top_right:
				mov = mov_top_right;
				next_pos->x = bola.x + 1;
				next_pos->y = bola.y - 1;
				break;
			case bottom_left:
				mov = mov_bottom_left;
				next_pos->x = bola.x - 1;
				next_pos->y = bola.y + 1;
				break;
			case bottom_right:
				mov = mov_bottom_right;
				next_pos->x = bola.x + 1;
				next_pos->y = bola.y + 1;
				break;
			case top:
				if (bola.mov == mov_bottom_right) {
					mov = mov_top_right;
					next_pos->x = bola.x + 1;
					next_pos->y = bola.y - 1;
				}
				else {
					mov = mov_top_left;
					next_pos->x = bola.x - 1;
					next_pos->y = bola.y - 1;
				}
				break;
			case bottom:
				if (bola.mov == mov_top_right) {
					mov = mov_bottom_right;
					next_pos->x = bola.x + 1;
					next_pos->y = bola.y + 1;
				}
				else {
					mov = mov_bottom_left;
					next_pos->x = bola.x - 1;
					next_pos->y = bola.y + 1;
				}
				break;
			case left:
				if (bola.mov == mov_bottom_right) {
					mov = mov_bottom_left;
					next_pos->x = bola.x - 1;
					next_pos->y = bola.y + 1;
				}
				else {
					mov = mov_top_left;
					next_pos->x = bola.x - 1;
					next_pos->y = bola.y - 1;
				}
				break;
			case right:
				if (bola.mov == mov_bottom_left) {
					mov = mov_top_right;
					next_pos->x = bola.x + 1;
					next_pos->y = bola.y - 1;
				}
				else {
					mov = mov_bottom_right;
					next_pos->x = bola.x + 1;
					next_pos->y = bola.y + 1;
				}
				break;
			default:
				break;
		}
	} else if (next_pos->y < END_Y_BORDER)
	{
		switch (which_side_bar(*next_pos)) {
			case top_left:
				mov = mov_top_left;
				next_pos->x = bola.x - 1;
				next_pos->y = bola.y - 1;
				break;
			case top_right:
				mov = mov_top_right;
				next_pos->x = bola.x + 1;
				next_pos->y = bola.y - 1;
				break;
			case top:
				if (bola.mov == mov_bottom_right) {
					mov = mov_top_right;
					next_pos->x = bola.x + 1;
					next_pos->y = bola.y - 1;
				}
				else {
					mov = mov_top_left;
					next_pos->x = bola.x - 1;
					next_pos->y = bola.y - 1;
				}
				break;
			case left:
				if (bola.mov == mov_bottom_right) {
					mov = mov_bottom_left;
					next_pos->x = bola.x - 1;
					next_pos->y = bola.y + 1;
				}
				else {
					mov = mov_top_left;
					next_pos->x = bola.x - 1;
					next_pos->y = bola.y - 1;
				}
				break;
			case right:
				if (bola.mov == mov_bottom_left) {
					mov = mov_top_right;
					next_pos->x = bola.x + 1;
					next_pos->y = bola.y - 1;
				}
				else {
					mov = mov_bottom_right;
					next_pos->x = bola.x + 1;
					next_pos->y = bola.y + 1;
				}
				break;
			default:
				break;
		}
	}
	return mov;
}


void move_bar(int dir){
	static int var = 0;
	if (dir == 1)
	{
		var = -1;
	}
	else if (dir == 3)
	{
		var = 1;
	}

	if (pos_barra.x + var > INT_X_BORDER && pos_barra.x + var <= END_X_BORDER - BAR_LENGTH) {
		rect(pos_barra, negro, BAR_LENGTH, BAR_HEIGHT);
		pos_barra.x = pos_barra.x + var;
		rect(pos_barra, gold, BAR_LENGTH, BAR_HEIGHT);
	}
}

void init_ball(){
	bola.x=pos_barra.x + BAR_LENGTH / 2;
	bola.y=pos_barra.y-1;
	bola_activa=1;
	paint(bola.x,bola.y,azul_claro);
}

levels_t level_selection() {
	int btn = 0;
	levels_t level;

	print_background(negro);

	position_t aux = {57, 25};
	paint_object(aux, choose_level, 5, 47);
	aux.x = 50;
	aux.y = 50;
	paint_object(aux, number_1, 12, 9);
	aux.y = 65;
	paint_object(aux, button, 10, 9);
	aux.x = 75;
	aux.y = 40;
	paint_object(aux, number_2, 12, 10);
	aux.y = 55;
	paint_object(aux, button, 10, 9);
	aux.x = 100;
	aux.y = 50;
	paint_object(aux, number_3, 12,9);
	aux.y = 65;
	paint_object(aux, button, 10, 9);
	aux.x = 5;
	aux.y = 100;
	paint_object(aux, authors, 14, 25);
	aux.x = 61;
	aux.y = 10;
	paint_object(aux, breakout, 7, 38);

	while (btn == 0)
		btn = wait_button();

	switch (btn) {
		case 1:
			level = first_lvl;
			break;
		case 2:
			level = second_lvl;
			break;
		case 3:
			level = third_lvl;
			break;
		default:
			break;
	}
	return level;
}

